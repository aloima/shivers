#include <stdlib.h>
#include <string.h>

#include <shivers.h>
#include <utils.h>
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

		char *user_id = json_get_val(*message, "author.id").value.string;

		if (strcmp(splitted.data[0], "about") == 0) {
			run_with_cooldown(user_id, about, client, message, args);
		} else if (strcmp(splitted.data[0], "avatar") == 0) {
			run_with_cooldown(user_id, avatar, client, message, args);
		} else if (strcmp(splitted.data[0], "github") == 0) {
			run_with_cooldown(user_id, github, client, message, args);
		}

		split_free(&splitted);
	}
}
