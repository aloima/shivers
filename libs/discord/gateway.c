#include <stdio.h>

#include <json.h>
#include <network.h>

void onmessage(char *message) {
	printf("%s\n", message);

	JSONElement *data = json_parse(message);
}

void connect_gateway(const char *token) {
	Websocket ws = {
		.url = "wss://gateway.discord.gg/?v=10&encoding=json",
		.port = 443,
		.onmessage = onmessage
	};

	connect_websocket(&ws);
}
