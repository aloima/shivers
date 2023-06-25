#include <stdio.h>

#include <unistd.h>

#include <json.h>
#include <network.h>

static Websocket ws;

static unsigned int heartbeat_interval;
static int last_sequence;

static char token[256] = {0};

static void send_identify() {
	char identify_message[1024];

	sprintf(identify_message, "{"
		"\"op\":2,"
		"\"d\":{"
		"\"token\":\"%s\","
		"\"properties\":{"
			"\"os\":\"linux\","
			"\"browser\":\"shivers\","
			"\"device\":\"shivers\""
		"},"
		"\"intents\":%d"
	"}", token, (1 << 0 | 1 << 1 | 1 << 9));

	send_websocket_message(&ws, identify_message);
}

void onmessage(const WebsocketFrame frame) {
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

void connect_gateway(const char *bot_token) {
	strcpy(token, bot_token);

	create_websocket("wss://gateway.discord.gg/?v=10&encoding=json", (WebsocketMethods) {
		.onmessage = onmessage
	});
}
