#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include <unistd.h>

#include <shivers.h>
#include <discord.h>
#include <json.h>
#include <network.h>

static Websocket websocket = {0};

static unsigned int heartbeat_interval;
static pthread_t heartbeat_thread;
static int last_sequence = -1;
static bool heartbeat_waiting = false;

static char token[256] = {0};
static size_t ready_guild_size = 0;
static bool handled_ready_guilds = false;

static struct Client client = {0};

static unsigned long previous_heartbeat_sent_at = 0;
static unsigned long heartbeat_sent_at = 0;
static unsigned long heartbeat_received_at = 0;

static void handle_exit(int sig) {
	close_websocket(&websocket, -1, NULL);

	if (client.user != NULL) {
		json_free(client.user);
	}

	pthread_cancel(heartbeat_thread);
	sleep(1);
	exit(EXIT_SUCCESS);
}

static void send_heartbeat() {
	char heartbeat_message[32];

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
	create_caches();
	puts("Websocket is started.");
}

static void onmessage(const WebsocketFrame frame) {
	jsonelement_t *data = json_parse(frame.payload);
	const char *event_name = json_get_val(data, "t").value.string;
	const unsigned short op = json_get_val(data, "op").value.number;

	switch (op) {
		case 0: {
			last_sequence = json_get_val(data, "s").value.number;

			if (strcmp(event_name, "READY") == 0) {
				struct Response response = api_request(token, "/users/@me", "GET", NULL);
				client.user = json_parse(response.data);
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
						puts("Handled all guilds.");
						handled_ready_guilds = true;
					}
				}
			} else if (strcmp(event_name, "MESSAGE_CREATE") == 0) {
				jsonelement_t *message = json_get_val(data, "d").value.object;
				on_message_create(client, message);
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

	json_free(data);
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

	websocket = create_websocket("wss://gateway.discord.gg/?v=10&encoding=json", (WebsocketMethods) {
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
