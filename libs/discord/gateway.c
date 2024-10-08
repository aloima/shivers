#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include <pthread.h>

#if defined(_WIN32)
  #include <winsock2.h>
  #include <windows.h>
#elif defined(__linux__)
  #include <unistd.h>
#endif

#include <shivers.h>
#include <discord.h>
#include <network.h>
#include <database.h>
#include <json.h>
#include <hash.h>

static struct Websocket websocket = {0};

static unsigned int heartbeat_interval;
static pthread_t heartbeat_thread;
static int last_sequence = -1;
static bool heartbeat_waiting = false;

static char session_id[48], resume_gateway_url[48];

static char token[96];
static unsigned long long ready_guild_size = 0, intents;
static bool handled_ready_guilds = false;

static struct Shivers shivers = {0};

static unsigned long long previous_heartbeat_sent_at = 0, heartbeat_sent_at = 0, heartbeat_received_at = 0;

static void handle_exit(int sig) {
  close_websocket(&websocket, -1, NULL);

  if (shivers.client.user != NULL) {
    json_free(shivers.client.user, false);
  }

  pthread_cancel(heartbeat_thread);

  #if defined(__linux__)
    usleep(250000);
  #elif defined(_WIN32)
    Sleep(250);
  #endif

  exit(EXIT_SUCCESS);
}

static void send_heartbeat() {
  char heartbeat_message[24];

  if (last_sequence == -1) {
    sprintf(heartbeat_message, "{"
      "\"op\":1,"
      "\"d\":null"
    "}");
  } else {
    sprintf(heartbeat_message, "{"
      "\"op\":1,"
      "\"d\":%d"
    "}", last_sequence);
  }

  heartbeat_sent_at = get_timestamp();
  send_websocket_message(&websocket, heartbeat_message);
}

static void *start_heartbeat_thread() {
  do {
    heartbeat_waiting = true;

    #if defined(__linux__)
      usleep(heartbeat_interval * 1000);
    #elif defined(_WIN32)
      Sleep(heartbeat_interval);
    #endif

    send_heartbeat();
    heartbeat_waiting = false;
  } while (websocket.connected && !websocket.closed);

  pthread_exit(NULL);
  return NULL;
}

static void send_identify() {
  char identify_message[1024];

  sprintf(identify_message, "{"
    "\"op\":2,"
    "\"d\":{"
      "\"token\":\"%s\","
      "\"intents\":%lld,"
      "\"properties\":{"
        "\"os\":\"linux\","
        "\"browser\":\"shivers\","
        "\"device\":\"shivers\""
      "}"
    "}"
  "}", token, intents);

  send_websocket_message(&websocket, identify_message);
}

static void send_resume() {
  char resume_message[512];

  sprintf(resume_message, "{"
    "\"op\":6,"
    "\"d\":{"
      "\"token\":\"%s\","
      "\"session_id\":%s,"
      "\"seq\":%d"
    "}"
  "}", token, session_id, last_sequence);

  puts(resume_message);
  send_websocket_message(&websocket, resume_message);
}

static void onstart() {
  puts("Websocket is started.");
}

static void parse_interaction_base_arguments(struct InteractionArgument *argument, jsonelement_t *data, unsigned char type, char *name, jsonresult_t input) {
  switch (type) {
    case STRING_ARGUMENT:
      *argument = (struct InteractionArgument) {
        .name = name,
        .type = type,
        .value = {
          .string = {
            .value = input.value.string,
            .length = input.element->size
          }
        }
      };

      break;

    case INTEGER_ARGUMENT:
      *argument = (struct InteractionArgument) {
        .name = name,
        .type = type,
        .value = {
          .number = input.value.number
        }
      };

      break;

    case BOOLEAN_ARGUMENT:
      *argument = (struct InteractionArgument) {
        .name = name,
        .type = type,
        .value = {
          .boolean = input.value.boolean
        }
      };

      break;

    case USER_ARGUMENT: {
      char user_search[16 + input.element->size], member_search[18 + input.element->size];
      sprintf(user_search, "resolved.users.%s", input.value.string);
      sprintf(member_search, "resolved.members.%s", input.value.string);

      jsonresult_t member_result = json_get_val(data, member_search);

      *argument = (struct InteractionArgument) {
        .name = name,
        .type = type,
        .value = {
          .user = {
            .user_data = json_get_val(data, user_search).element,
            .member_data = member_result.exist ? member_result.element : NULL
          }
        }
      };

      break;
    }

    case CHANNEL_ARGUMENT: {
      char search[19 + input.element->size];
      sprintf(search, "resolved.channels.%s", input.value.string);

      *argument = (struct InteractionArgument) {
        .name = name,
        .type = type,
        .value = {
          .channel = json_get_val(data, search).element
        }
      };

      break;
    }

    case ROLE_ARGUMENT: {
      char search[16 + input.element->size];
      sprintf(search, "resolved.roles.%s", input.value.string);

      *argument = (struct InteractionArgument) {
        .name = name,
        .type = type,
        .value = {
          .channel = json_get_val(data, search).element
        }
      };

      break;
    }
  }
}

static void onmessage(const struct WebsocketFrame frame) {
  jsonelement_t *data = json_parse(frame.payload);
  const char *event_name = json_get_val(data, "t").value.string;
  const unsigned short op = json_get_val(data, "op").value.number;

  switch (op) {
    case 0: {
      last_sequence = json_get_val(data, "s").value.number;

      if (streq(event_name, "READY")) {
        shivers.client.user = clone_json_element(json_get_val(data, "d.user").element);
        shivers.client.token = token;
        shivers.client.ready_at = get_timestamp();
        shivers.client.guilds = create_hashmap(16);

        const jsonresult_t json_resume_gateway_url = json_get_val(data, "d.resume_gateway_url");
        const jsonresult_t json_session_id= json_get_val(data, "d.session_id");

        memcpy(resume_gateway_url, json_resume_gateway_url.value.string, json_resume_gateway_url.element->size + 1);
        memcpy(session_id, json_session_id.value.string, json_session_id.element->size + 1);

        ready_guild_size = json_get_val(data, "d.guilds").element->size;

        on_ready(&shivers);
      } else if (streq(event_name, "GUILD_CREATE")) {
        const char *id = json_get_val(data, "d.id").value.string;
        const jsonresult_t members = json_get_val(data, "d.members");
        const jsonresult_t presences = json_get_val(data, "d.presences");
        const jsonresult_t voice_states = json_get_val(data, "d.voice_states");

        struct Guild guild = {
          .members = create_hashmap(32),
          .total_member_count = json_get_val(data, "d.member_count").value.number,
          .bot_count = 0,
          .non_offline_count = 0,
          .member_at_voice_count = voice_states.element->size
        };

        for (unsigned int i = 0; i < members.element->size; ++i) {
          jsonelement_t *json_user = json_get_val(((jsonelement_t **) members.element->value)[i], "user").element;

          struct Member member = {
            .bot = json_get_val(json_user, "bot").value.boolean
          };

          insert_node(guild.members, json_get_val(json_user, "id").value.string, &member, sizeof(struct Member));
          if (member.bot) ++guild.bot_count;
        }

        for (unsigned int i = 0; i < presences.element->size; ++i) {
          jsonelement_t *presence = ((jsonelement_t **) presences.element->value)[i];
          const char *status = json_get_val(presence, "status").value.string;
          const char *user_id = json_get_val(presence, "user.id").value.string;

          if (!streq(status, "offline")) {
            struct Member *member = get_node(guild.members, user_id)->value;

            if (streq(status, "offline")) member->status = OFFLINE;
            else {
              if (streq(status, "online")) member->status = ONLINE;
              else if (streq(status, "idle")) member->status = IDLE;
              else if (streq(status, "dnd")) member->status = DND;

              ++guild.non_offline_count;
            }
          }
        }

        for (unsigned int i = 0; i < guild.member_at_voice_count; ++i) {
          const char *user_id = json_get_val(((jsonelement_t **) voice_states.element->value)[i], "user_id").value.string;
          struct Member *member = get_node(guild.members, user_id)->value;
          member->at_voice = true;
        }

        insert_node(shivers.client.guilds, id, &guild, sizeof(struct Guild));

        if (!handled_ready_guilds) {
          --ready_guild_size;

          if (ready_guild_size == 0) {
            handled_ready_guilds = true;
            on_handle_guilds(&shivers);
          }
        } else {
          on_guild_create(&shivers);
        }
      } else if (streq(event_name, "GUILD_DELETE")) {
        const char *id = json_get_val(data, "d.id").value.string;
        delete_node(shivers.client.guilds, id);

        on_guild_delete(&shivers);
      } else if (streq(event_name, "GUILD_MEMBER_ADD")) {
        struct Node *guild_node = get_node(shivers.client.guilds, json_get_val(data, "d.guild_id").value.string);
        struct Guild *guild = guild_node->value;
        jsonelement_t *user = json_get_val(data, "d.user").element;
        const char *user_id = json_get_val(user, "id").value.string;
        struct Member member = {
          .bot = json_get_val(user, "bot").value.string
        };

        insert_node(guild->members, user_id, &member, sizeof(struct Member));
        on_guild_member_add(&shivers, guild_node);
      } else if (streq(event_name, "GUILD_MEMBER_REMOVE")) {
        struct Node *guild_node = get_node(shivers.client.guilds, json_get_val(data, "d.guild_id").value.string);
        struct Guild *guild = guild_node->value;
        const char *user_id = json_get_val(data, "d.user.id").value.string;

        delete_node(guild->members, user_id);
        on_guild_member_remove(&shivers, guild_node);
      } else if (streq(event_name, "MESSAGE_CREATE")) {
        jsonelement_t *message = json_get_val(data, "d").element;
        on_message_create(&shivers, message);
      } else if (streq(event_name, "INTERACTION_CREATE")) {
        jsonelement_t *interaction = json_get_val(data, "d").element;

        if (json_get_val(interaction, "type").value.number == 2.0) {
          jsonelement_t *interaction_data = json_get_val(interaction, "data").element;

          struct InteractionCommand command = {
            .id = json_get_val(interaction, "id").value.string,
            .token = json_get_val(interaction, "token").value.string,
            .name = json_get_val(interaction_data, "name").value.string,
            .channel_id = json_get_val(interaction, "channel_id").value.string
          };

          jsonresult_t guild_id = json_get_val(interaction, "guild_id");

          if (guild_id.exist && guild_id.element->type != JSON_NULL) {
            command.guild_id = guild_id.value.string;
            command.user = json_get_val(interaction, "member.user").element;
          } else {
            command.user = json_get_val(interaction, "user").element;
          }

          jsonresult_t options_result = json_get_val(interaction_data, "options");
          unsigned char options_size = (options_result.exist ? options_result.element->size : 0);

          if (options_result.exist && options_result.element->type == JSON_ARRAY && options_size != 0) {
            command.arguments = allocate(NULL, -1, options_size, sizeof(struct InteractionArgument));
            command.argument_size = options_result.element->size;

            for (unsigned int i = 0; i < options_size; ++i) {
              jsonelement_t *option_element = ((jsonelement_t **) options_result.element->value)[i];
              const unsigned int option_type = json_get_val(option_element, "type").value.number;
              char *option_name = json_get_val(option_element, "name").value.string;

              if (option_type == SUBCOMMAND_ARGUMENT) {
                command.arguments[i].name = option_name;
                command.arguments[i].type = option_type;

                struct InteractionSubcommand *subcommand = &(command.arguments[i].value.subcommand);
                jsonresult_t sc_options_result = json_get_val(option_element, "options");
                subcommand->argument_size = sc_options_result.element->size;

                if (sc_options_result.element->type == JSON_ARRAY && subcommand->argument_size != 0) {
                  struct InteractionSubcommand *subcommand = &(command.arguments[i].value.subcommand);
                  subcommand->arguments = allocate(NULL, -1, subcommand->argument_size, sizeof(struct InteractionArgument));

                  for (unsigned int s = 0; s < subcommand->argument_size; ++s) {
                    jsonelement_t *option_element = ((jsonelement_t **) sc_options_result.element->value)[s];
                    const unsigned int option_type = json_get_val(option_element, "type").value.number;
                    char *option_name = json_get_val(option_element, "name").value.string;

                    struct InteractionArgument *subcommand_argument = &(subcommand->arguments[s]);
                    jsonresult_t option_value = json_get_val(option_element, "value");
                    parse_interaction_base_arguments(subcommand_argument, interaction_data, option_type, option_name, option_value);
                  }
                }
              } else {
                struct InteractionArgument *argument = &(command.arguments[i]);
                jsonresult_t option_value = json_get_val(option_element, "value");
                parse_interaction_base_arguments(argument, interaction_data, option_type, option_name, option_value);
              }
            }
          }

          on_interaction_command(&shivers, command);

          for (unsigned int i = 0; i < options_size; ++i) {
            struct InteractionArgument argument = command.arguments[i];

            if (argument.type == SUBCOMMAND_ARGUMENT && argument.value.subcommand.argument_size != 0) {
              free(argument.value.subcommand.arguments);
            }
          }

          free(command.arguments);
        }
      } else if (streq(event_name, "PRESENCE_UPDATE")) {
        const char *user_id = json_get_val(data, "d.user.id").value.string;
        const char *status = json_get_val(data, "d.status").value.string;
        struct Node *guild_node = get_node(shivers.client.guilds, json_get_val(data, "d.guild_id").value.string);
        struct Guild *guild = guild_node->value;
        struct Member *member = get_node(guild->members, user_id)->value;

        if (streq(status, "offline")) {
          --guild->non_offline_count;
          member->status = OFFLINE;
        } else {
          if (member->status == OFFLINE) {
            ++guild->non_offline_count;
          }

          if (streq(status, "online")) member->status = ONLINE;
          else if (streq(status, "idle")) member->status = IDLE;
          else if (streq(status, "dnd")) member->status = DND;
        }

        on_presence_update(&shivers, guild_node);
      } else if (streq(event_name, "VOICE_STATE_UPDATE")) {
        const char *user_id = json_get_val(data, "d.user_id").value.string;
        const jsonresult_t channel_id = json_get_val(data, "d.channel_id");
        const jsonresult_t guild_id = json_get_val(data, "d.guild_id");

        if (guild_id.exist && guild_id.element->type == JSON_STRING) {
          struct Node *guild_node = get_node(shivers.client.guilds, guild_id.value.string);
          struct Guild *guild = guild_node->value;
          struct Member *member = get_node(guild->members, user_id)->value;

          if (!channel_id.exist || channel_id.element->type == JSON_NULL) {
            member->at_voice = false;
            --guild->member_at_voice_count;
          } else {
            if (!member->at_voice) ++guild->member_at_voice_count;
            member->at_voice = true;
          }

          on_voice_state_update(&shivers, guild_node);
        }
      }

      break;
    }

    case 7: {
      puts("resume");
      database_save();
      close_websocket(&websocket, -2, NULL);
      websocket.connected = true;
      websocket.closed = false;
      connect_gateway(token, resume_gateway_url, intents);
      send_resume();
      puts("Received and handled RESUME opcode.");
      break;
    }

    case 10: {
      heartbeat_interval = json_get_val(data, "d.heartbeat_interval").value.number;
      send_identify();
      send_heartbeat();
      pthread_create(&heartbeat_thread, NULL, start_heartbeat_thread, NULL);
      pthread_detach(heartbeat_thread);
      signal(SIGINT, handle_exit);
      break;
    }

    case 11: {
      previous_heartbeat_sent_at = heartbeat_sent_at;
      heartbeat_received_at = get_timestamp();
      break;
    }
  }

  json_free(data, false);
}

static void onclose(const short code, const char *reason) {
  if (code != -1) {
    if (reason) {
      printf("Closed: %d\n%s\n", code, reason);
    } else {
      printf("Closed: %d\n", code);
    }
  }

  if (code != -2) {
    for (unsigned int i = 0; i < shivers.client.guilds->size; ++i) {
      struct Node *node = shivers.client.guilds->nodes[i];

      if (node != NULL) {
        while (node) {
          free_hashmap(((struct Guild *) node->value)->members);
          node = node->next;
        }
      }
    }

    free_hashmap(shivers.client.guilds);
    on_force_close(&shivers);
  }
}

void connect_gateway(const char *bot_token, const char *url, const unsigned int bot_intents) {
  strcpy(token, bot_token);
  intents = bot_intents;

  char connection_url[72];
  sprintf(connection_url, "%s/?v=10&encoding=json", url);

  websocket = create_websocket(connection_url, (struct WebsocketMethods) {
    .onstart = onstart,
    .onmessage = onmessage,
    .onclose = onclose
  });

  connect_websocket(&websocket);
}

unsigned int get_latency() {
  if (heartbeat_sent_at > heartbeat_received_at) {
    return (heartbeat_received_at - previous_heartbeat_sent_at);
  } else {
    return (heartbeat_received_at - heartbeat_sent_at);
  }
}

void set_presence(const char *name, const char *state, const char *details, const char type, const char *status) {
  char activity[128];

  if (state == NULL && details == NULL) {
    sprintf(activity, "{"
      "\"name\":\"%s\","
      "\"type\":%d"
    "}", name, type);
  } else if (state && details == NULL) {
    sprintf(activity, "{"
      "\"name\":\"%s\","
      "\"state\":\"%s\","
      "\"type\":%d"
    "}", name, state, type);
  } else {
    sprintf(activity, "{"
      "\"name\":\"%s\","
      "\"state\":\"%s\","
      "\"details\":\"%s\","
      "\"type\":%d"
    "}", name, state, details, type);
  }

  char presence[256];
  sprintf(presence, "{"
    "\"op\":3,"
    "\"d\":{"
      "\"since\":null,"
      "\"activities\":["
        "%s"
      "],"
      "\"status\":\"%s\","
      "\"afk\":false"
    "}"
  "}", activity, status);

  send_websocket_message(&websocket, presence);
}
