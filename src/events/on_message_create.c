#include <stdlib.h>
#include <string.h>

#include <shivers.h>
#include <json.h>

void on_message_create(Client client, JSONElement **message) {
	char *content = json_get_val(*message, "content").string;
	size_t prefix_length = strlen(PREFIX);

	if (content != NULL && strncmp(content, PREFIX, prefix_length) == 0) {
		Split args = split(content + prefix_length, " ");

		if (strcmp(args.data[0], "about") == 0) {
			about(client, message);
		}

		split_free(&args);
	}
}
