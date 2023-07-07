#include <network.h>

const unsigned int get_all_intents() {
	unsigned int result = 0;

	for (int i = 0; i <= 21; ++i) {
		result |= (1 << i);
	}

	return (const unsigned int) result;
}

Response api_request(char *token, char *path, char *method, char *content) {
	char url[256] = {0};
	sprintf(url, "https://discord.com/api/v10%s", path);

	char authorization[128] = {0};
	sprintf(authorization, "Bot %s", token);

	Header *headers = allocate(NULL, 2, sizeof(Header));

	headers[0] = (Header) {
		.name = "Authorization",
		.value = authorization
	};

	headers[1] = (Header) {
		.name = "Content-Type",
		.value = "application/json"
	};

	RequestConfig config;
	config.url = url;
	config.method = method;
	config.header_size = 2;
	config.headers = headers;

	return request(config);
}
