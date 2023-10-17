#include <string.h>

#include <shivers.h>
#include <utils.h>
#include <json.h>

void on_message_create(struct Client client, jsonelement_t *message) {
	const bool is_webhook = json_get_val(message, "webhook_id").exist;
	const char *content = json_get_val(message, "content").value.string;
	const unsigned char prefix_length = strlen(PREFIX);

	if (content != NULL && !is_webhook && strncmp(content, PREFIX, prefix_length) == 0) {
		Split splitted = split(content + prefix_length, " ");
		Split args = {
			.data = splitted.data + 1,
			.size = splitted.size - 1
		};

		const char *input = splitted.data[0];
		const char *user_id = json_get_val(message, "author.id").value.string;

		const unsigned short command_size = get_command_size();
		const struct Command *commands = get_commands();

		for (unsigned short i = 0; i < command_size; ++i) {
			const struct Command command = commands[i];

			if (strcmp(input, command.name) == 0) {
				run_with_cooldown(user_id, command.execute, client, message, args);
				break;
			}
		}

		split_free(&splitted);
	}
}
