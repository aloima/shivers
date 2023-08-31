#include <string.h>

#include <shivers.h>
#include <utils.h>
#include <json.h>

void on_message_create(Client client, jsonelement_t **message) {
	char *content = json_get_val(*message, "content").value.string;
	size_t prefix_length = strlen(PREFIX);

	if (content != NULL && strncmp(content, PREFIX, prefix_length) == 0) {
		Split splitted = split(content + prefix_length, " ");
		Split args = {
			.data = splitted.data + 1,
			.size = splitted.size - 1
		};

		const char *input = splitted.data[0];
		char *user_id = json_get_val(*message, "author.id").value.string;

		const size_t command_size = get_command_size();
		const struct Command *commands = get_commands();

		for (size_t i = 0; i < command_size; ++i) {
			struct Command command = commands[i];

			if (strcmp(input, command.name) == 0) {
				run_with_cooldown(user_id, command.execute, client, message, args);
				break;
			}
		}

		split_free(&splitted);
	}
}
