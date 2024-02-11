#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include <pthread.h>
#include <unistd.h>

#include <shivers.h>
#include <discord.h>
#include <json.h>
#include <network.h>

static struct Websocket websocket = {0};

static unsigned int heartbeat_interval;
static pthread_t heartbeat_thread;
static int last_sequence = -1;
static bool heartbeat_waiting = false;

static char token[96];
static unsigned long ready_guild_size = 0;
static bool handled_ready_guilds = false;

static struct Client client = {0};

static unsigned long previous_heartbeat_sent_at = 0;
static unsigned long heartbeat_sent_at = 0;
static unsigned long heartbeat_received_at = 0;

static void handle_exit(int sig) {
	close_websocket(&websocket, -1, NULL);

	if (client.user != NULL) {
		json_free(client.user, false);
	}

	pthread_cancel(heartbeat_thread);
	usleep(250000);
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

	heartbeat_sent_at = get_timestamp(NULL);
	send_websocket_message(&websocket, heartbeat_message);
}

static void *start_heartbeat_thread() {
	do {
		heartbeat_waiting = true;
		usleep(heartbeat_interval * 1000);
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
			"\"intents\":%d,"
			"\"properties\":{"
				"\"os\":\"linux\","
				"\"browser\":\"shivers\","
				"\"device\":\"shivers\""
			"}"
		"}"
	"}", token, get_all_intents());

	send_websocket_message(&websocket, identify_message);
}

static void onstart() {
	puts("Websocket is started.");
}

static void parse_interaction_base_arguments(struct InteractionArgument *argument, jsonelement_t *data, unsigned char type, char *name, jsonvalue_t value) {
	switch (type) {
		case STRING_ARGUMENT:
			*argument = (struct InteractionArgument) {
				.name = name,
				.value = {
					.string = value.string
				}
			};

			break;

		case INTEGER_ARGUMENT:
			*argument = (struct InteractionArgument) {
				.name = name,
				.value = {
					.number = value.number
				}
			};

			break;

		case BOOLEAN_ARGUMENT:
			*argument = (struct InteractionArgument) {
				.name = name,
				.value = {
					.boolean = value.boolean
				}
			};

			break;

		case USER_ARGUMENT: {
			char search[35];
			sprintf(search, "resolved.users.%s", value.string);

			*argument = (struct InteractionArgument) {
				.name = name,
				.value = {
					.user = json_get_val(data, search).value.object
				}
			};

			break;
		}

		case CHANNEL_ARGUMENT: {
			char search[38];
			sprintf(search, "resolved.channels.%s", value.string);

			*argument = (struct InteractionArgument) {
				.name = name,
				.value = {
					.channel = json_get_val(data, search).value.object
				}
			};

			break;
		}

		case ROLE_ARGUMENT: {
			char search[35];
			sprintf(search, "resolved.roles.%s", value.string);

			*argument = (struct InteractionArgument) {
				.name = name,
				.value = {
					.channel = json_get_val(data, search).value.object
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

			if (strcmp(event_name, "READY") == 0) {
				struct Response response = api_request(token, "/users/@me", "GET", NULL, NULL);
				client.user = json_parse((const char *) response.data);
				client.token = token;
				client.ready_at = get_timestamp(NULL);

				ready_guild_size = json_get_val(data, "d.guilds").value.array->size;

				response_free(&response);
				on_ready(client);
			} else if (strcmp(event_name, "GUILD_CREATE") == 0) {
				add_to_cache(get_guilds_cache(), json_get_val(data, "d.id").value.string);

				if (!handled_ready_guilds) {
					--ready_guild_size;

					if (ready_guild_size == 0) {
						handled_ready_guilds = true;
						on_handle_guilds(client);
					}
				} else {
					on_guild_create(client);
				}
			} else if (strcmp(event_name, "GUILD_DELETE") == 0) {
				struct Cache *guilds = get_guilds_cache();
				const char *guild_id = json_get_val(data, "d.id").value.string;
				unsigned int i;

				for (i = 0; i < guilds->size; ++i) {
					if (strcmp(guilds->data[i], guild_id) == 0) {
						break;
					}
				}

				remove_from_cache_index(get_guilds_cache(), i);
				on_guild_delete(client);
			} else if (strcmp(event_name, "MESSAGE_CREATE") == 0) {
				jsonelement_t *message = json_get_val(data, "d").value.object;
				on_message_create(client, message);
			} else if (strcmp(event_name, "INTERACTION_CREATE") == 0) {
				jsonelement_t *interaction = json_get_val(data, "d").value.object;

				if (json_get_val(interaction, "type").value.number == 2.0) {
					jsonelement_t *interaction_data = json_get_val(data, "d.data").value.object;

					struct InteractionCommand command = {
						.id = json_get_val(interaction, "id").value.string,
						.token = json_get_val(interaction, "token").value.string,
						.name = json_get_val(interaction_data, "name").value.string,
						.channel_id = json_get_val(interaction, "channel_id").value.string
					};

					jsonresult_t guild_result = json_get_val(interaction, "guild_id");

					if (guild_result.exist && guild_result.type != JSON_NULL) {
						command.guild_id = json_get_val(interaction, "guild_id").value.string;
						command.user = json_get_val(interaction, "member.user").value.object;
					} else {
						command.user = json_get_val(interaction_data, "user.id").value.object;
					}

					jsonresult_t options_result = json_get_val(interaction_data, "options");
					const unsigned char options_size = options_result.value.array->size;

					if (options_result.type == JSON_ARRAY && options_size != 0) {
						command.arguments = allocate(NULL, -1, options_size, sizeof(struct InteractionArgument));
						command.argument_size = options_size;

						for (unsigned char i = 0; i < options_size; ++i) {
							jsonelement_t *option_element = ((jsonelement_t **) options_result.element->value)[i];
							const unsigned int option_type = json_get_val(option_element, "type").value.number;
							char *option_name = json_get_val(option_element, "name").value.string;

							if (option_type == SUBCOMMAND_ARGUMENT) {
								command.arguments[i].name = option_name;

								jsonresult_t sc_options_result = json_get_val(option_element, "options");
								const unsigned char sc_options_size = sc_options_result.value.array->size;

								if (sc_options_result.type == JSON_ARRAY && sc_options_size != 0) {
									command.arguments[i].value.subcommand.arguments = allocate(NULL, -1, sc_options_size, sizeof(struct InteractionArgument));
									command.arguments[i].value.subcommand.argument_size = sc_options_size;

									for (unsigned char s = 0; s < sc_options_size; ++s) {
										jsonelement_t *sc_option_element = ((jsonelement_t **) sc_options_result.element->value)[s];
										const unsigned int sc_option_type = json_get_val(sc_option_element, "type").value.number;
										char *sc_option_name = json_get_val(sc_option_element, "name").value.string;

										struct InteractionArgument *sc_argument = &(command.arguments[i].value.subcommand.arguments[s]);
										jsonvalue_t sc_option_value = json_get_val(sc_option_element, "value").value;
										parse_interaction_base_arguments(sc_argument, interaction_data, sc_option_type, sc_option_name, sc_option_value);
									}
								}
							} else {
								struct InteractionArgument *argument = &(command.arguments[i]);
								jsonvalue_t option_value = json_get_val(option_element, "value").value;
								parse_interaction_base_arguments(argument, interaction_data, option_type, option_name, option_value);
							}
						}
					}

					on_interaction_command(client, command);

					for (unsigned char i = 0; i < options_size; ++i) {
						struct InteractionArgument argument = command.arguments[i];

						if (argument.type == SUBCOMMAND_ARGUMENT && argument.value.subcommand.argument_size != 0) {
							free(argument.value.subcommand.arguments);
						}
					}

					free(command.arguments);
				}
			}

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
			heartbeat_received_at = get_timestamp(NULL);
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

	on_force_close();
	clear_cache(get_guilds_cache());
}

void connect_gateway(const char *bot_token) {
	strcpy(token, bot_token);

	websocket = create_websocket("wss://gateway.discord.gg/?v=10&encoding=json", (struct WebsocketMethods) {
		.onstart = onstart,
		.onmessage = onmessage,
		.onclose = onclose
	});

	connect_websocket(&websocket);
}

int get_latency() {
	if (heartbeat_sent_at > heartbeat_received_at) {
		return (heartbeat_received_at - previous_heartbeat_sent_at);
	} else {
		return (heartbeat_received_at - heartbeat_sent_at);
	}
}

void set_presence(const char *name, const char type, const char *status) {
	char presence[256];
	sprintf(presence, "{"
		"\"op\":3,"
		"\"d\":{"
			"\"since\":null,"
			"\"activities\":["
				"{"
					"\"name\":\"%s\","
					"\"type\":%d"
				"}"
			"],"
			"\"status\":\"%s\","
			"\"afk\":false"
		"}"
	"}", name, type, status);

	send_websocket_message(&websocket, presence);
}
