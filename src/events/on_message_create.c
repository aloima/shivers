#include <stdlib.h>
#include <string.h>

#include <shivers.h>
#include <json.h>

void on_message_create(Client client, JSONElement **message) {
	char *content = json_get_val(*message, "content").value.string;
	size_t prefix_length = strlen(PREFIX);

	if (content != NULL && strncmp(content, PREFIX, prefix_length) == 0) {
		Split splitted = split(content + prefix_length, " ");
		Split args = {
			.data = splitted.data + 1,
			.size = splitted.size - 1
		};

		if (strcmp(splitted.data[0], "about") == 0) {
			about(client, message);
		} else if (strcmp(splitted.data[0], "avatar") == 0) {
			avatar(client, message, args);
		}

		split_free(&splitted);
	}
}
