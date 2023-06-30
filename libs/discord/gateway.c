#include <stdio.h>

#include <unistd.h>

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
			"\"op\": 1,"
			"\"d\": null"
		"}");
	} else {
		sprintf(heartbeat_message, "{"
			"\"op\":1,"
			"\"d\":%d"
		"}", last_sequence);
	}

	send_websocket_message(&websocket, heartbeat_message);
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

static void onmessage(const WebsocketFrame frame) {
	JSONElement *data = json_parse((char *) frame.payload);
	unsigned short op = (unsigned short) json_get_val(data, "op").number;

	switch (op) {
		case 0:
			last_sequence = (int) json_get_val(data, "s").number;
			break;

		case 10:
			heartbeat_interval = (unsigned short) json_get_val(data, "d.heartbeat_interval").number;
			send_identify();
			break;
	}

	json_free(data);
}

static void onclose(const short code) {
	printf("closed: %d\n", code);
}

void connect_gateway(const char *bot_token) {
	strcpy(token, bot_token);

	websocket = create_websocket("wss://gateway.discord.gg/?v=10&encoding=json", (WebsocketMethods) {
		.onmessage = onmessage,
		.onclose = onclose
	});

	connect_websocket(&websocket);
}
