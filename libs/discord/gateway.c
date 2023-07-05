#include <stdio.h>

#include <unistd.h>

#include <discord.h>
#include <json.h>
#include <network.h>

static Websocket websocket;

static unsigned int heartbeat_interval;
static int last_sequence = -1;

static char token[256] = {0};

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

	send_websocket_message(&websocket, heartbeat_message);
}

static void *start_heartbeat_thread() {
	do {
		usleep(heartbeat_interval * 1000);
		send_heartbeat();
	} while (websocket.connected);

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
	"}", token, (1 << 0 | 1 << 1 | 1 << 9));

	send_websocket_message(&websocket, identify_message);
}

static void onstart() {
	create_caches();
	puts("Started.");
}

static void onmessage(const WebsocketFrame frame) {
	JSONElement *data = json_parse((char *) frame.payload);
	char *event_name = json_get_val(data, "t").string;
	unsigned short op = (unsigned short) json_get_val(data, "op").number;

	switch (op) {
		case 0: {
			last_sequence = (int) json_get_val(data, "s").number;

			if (strcmp(event_name, "GUILD_CREATE") == 0) {
				add_to_cache(get_guilds_cache(), json_get_val(data, "d.id").string);
			}

			break;
		}

		case 10: {
			heartbeat_interval = (unsigned short) json_get_val(data, "d.heartbeat_interval").number;
			send_identify();
			pthread_t heartbeat_thread;
			pthread_create(&heartbeat_thread, NULL, start_heartbeat_thread, NULL);
			pthread_detach(heartbeat_thread);
			break;
		}
	}

	json_free(data);
}

static void onclose(const short code, const char *reason) {
	if (reason) {
		printf("Closed: %d\n%s\n", code, reason);
	} else {
		printf("Closed: %d\n", code);
	}
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
