#include <string.h>
#include <stdbool.h>

#include <shivers.h>
#include <utils.h>
#include <json.h>

static struct Command *commands = NULL;
static unsigned short command_size = 0;

void setup_commands(const struct Client client) {
	const struct Command command_array[] = {about, avatar, github, level, level_settings, vstats, wikipedia};
	command_size = sizeof(command_array) / sizeof(struct Command);

	commands = allocate(NULL, -1, command_size, sizeof(struct Command));
	memcpy(commands, command_array, sizeof(command_array));

	jsonelement_t *commands_body = create_empty_json_element(true);
	double command_type = 1.0;
	char key[32], argument_key[32], sc_argument_key[32];

	for (unsigned char i = 0; i < command_size; ++i) {
		const struct Command command = command_array[i];
		jsonelement_t *command_body = create_empty_json_element(false);
		sprintf(key, "[%d]", i);

		json_set_val(command_body, "name", (char *) command.name, JSON_STRING);
		json_set_val(command_body, "type", &command_type, JSON_NUMBER);
		json_set_val(command_body, "description", (char *) command.description, JSON_STRING);

		if (command.guild_only) {
			jsonelement_t *contexts = create_empty_json_element(true);
			double guild_only = 0.0;
			json_set_val(contexts, "[0]", &guild_only, JSON_NUMBER);

			json_set_val(command_body, "contexts", contexts, JSON_ARRAY);
			json_free(contexts, false);
		}

		if (command.permissions != 0) {
			char permissions[21];
			sprintf(permissions, "%ld", command.permissions);
			json_set_val(command_body, "default_member_permissions", permissions, JSON_STRING);
		}

		if (command.arg_size != 0) {
			jsonelement_t *arguments_body = create_empty_json_element(true);

			for (unsigned char a = 0; a < command.arg_size; ++a) {
				const struct CommandArgument argument = command.args[a];
				bool argument_required = !argument.optional;
				double argument_type = argument.type;
				jsonelement_t *argument_body = create_empty_json_element(false);
				sprintf(argument_key, "[%d]", a);

				json_set_val(argument_body, "name", (char *) argument.name, JSON_STRING);
				json_set_val(argument_body, "description", (char *) argument.description, JSON_STRING);
				json_set_val(argument_body, "type", &argument_type, JSON_NUMBER);

				if (argument_type == SUBCOMMAND_ARGUMENT) {
					jsonelement_t *sc_arguments_body = create_empty_json_element(true);

					for (unsigned char b = 0; b < argument.arg_size; ++b) {
						const struct CommandArgument sc_argument = argument.args[b];
						bool sc_argument_required = !sc_argument.optional;
						double sc_argument_type = sc_argument.type;
						jsonelement_t *sc_argument_body = create_empty_json_element(false);
						sprintf(sc_argument_key, "[%d]", b);

						json_set_val(sc_argument_body, "name", (char *) sc_argument.name, JSON_STRING);
						json_set_val(sc_argument_body, "description", (char *) sc_argument.description, JSON_STRING);
						json_set_val(sc_argument_body, "type", &sc_argument_type, JSON_NUMBER);
						json_set_val(sc_argument_body, "required", &sc_argument_required, JSON_BOOLEAN);

						json_set_val(sc_arguments_body, sc_argument_key, sc_argument_body, JSON_OBJECT);
						json_free(sc_argument_body, false);
					}

					json_set_val(argument_body, "options", sc_arguments_body, JSON_ARRAY);
					json_free(sc_arguments_body, false);
				} else {
					json_set_val(argument_body, "required", &argument_required, JSON_BOOLEAN);
				}

				json_set_val(arguments_body, argument_key, argument_body, JSON_OBJECT);
				json_free(argument_body, false);
			}

			json_set_val(command_body, "options", arguments_body, JSON_ARRAY);
			json_free(arguments_body, false);
		}

		json_set_val(commands_body, key, command_body, JSON_OBJECT);
		json_free(command_body, false);
	}

	char *body = json_stringify(commands_body, 2);
	json_free(commands_body, false);

	char path[42];
	sprintf(path, "/applications/%s/commands", json_get_val(client.user, "id").value.string);
	struct Response response = api_request(client.token, path, "PUT", body, NULL);

	response_free(response);
	free(body);
}

void free_commands() {
	free(commands);
}

const struct Command *get_commands() {
	return commands;
}

const unsigned short get_command_size() {
	return command_size;
}
