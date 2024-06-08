#include <string.h>
#include <stdbool.h>

#include <shivers.h>
#include <utils.h>
#include <json.h>
#include <hash.h>

void setup_commands(struct Shivers *shivers) {
	shivers->commands = create_hashmap(7);
	insert_node(shivers->commands, "about", &about, sizeof(struct Command));
	insert_node(shivers->commands, "avatar", &avatar, sizeof(struct Command));
	insert_node(shivers->commands, "github", &github, sizeof(struct Command));
	insert_node(shivers->commands, "level", &level, sizeof(struct Command));
	insert_node(shivers->commands, "level-settings", &level_settings, sizeof(struct Command));
	insert_node(shivers->commands, "vstats", &vstats, sizeof(struct Command));
	insert_node(shivers->commands, "wikipedia", &wikipedia, sizeof(struct Command));

	jsonelement_t *commands_body = create_empty_json_element(true);
	double command_type = 1.0;
	char key[6], argument_key[32], sc_argument_key[32];
	unsigned int stringified_commands = 0;

	for (unsigned int i = 0; i < 7; ++i) {
		struct Node *command_node = shivers->commands->nodes[i];

		while (command_node) {
			struct Command *command = command_node->value;
			jsonelement_t *command_body = create_empty_json_element(false);
			sprintf(key, "[%d]", stringified_commands);
			++stringified_commands;

			json_set_val(command_body, "name", command_node->key, JSON_STRING);
			json_set_val(command_body, "type", &command_type, JSON_NUMBER);
			json_set_val(command_body, "description", command->description, JSON_STRING);

			if (command->guild_only) {
				jsonelement_t *contexts = create_empty_json_element(true);
				double guild_only = 0.0;
				json_set_val(contexts, "[0]", &guild_only, JSON_NUMBER);

				json_set_val(command_body, "contexts", contexts, JSON_ARRAY);
				json_free(contexts, false);
			}

			if (command->permissions != 0) {
				char permissions[21];
				sprintf(permissions, "%ld", command->permissions);
				json_set_val(command_body, "default_member_permissions", permissions, JSON_STRING);
			}

			if (command->arg_size != 0) {
				jsonelement_t *arguments_body = create_empty_json_element(true);

				for (unsigned int a = 0; a < command->arg_size; ++a) {
					const struct CommandArgument argument = command->args[a];
					bool argument_required = !argument.optional;
					double argument_type = argument.type;
					jsonelement_t *argument_body = create_empty_json_element(false);
					sprintf(argument_key, "[%d]", a);

					json_set_val(argument_body, "name", argument.name, JSON_STRING);
					json_set_val(argument_body, "description", argument.description, JSON_STRING);
					json_set_val(argument_body, "type", &argument_type, JSON_NUMBER);

					if (argument_type == SUBCOMMAND_ARGUMENT) {
						jsonelement_t *sc_arguments_body = create_empty_json_element(true);

						for (unsigned int b = 0; b < argument.arg_size; ++b) {
							const struct CommandArgument subcommand_arg = argument.args[b];
							char *subcommand_arg_required = !subcommand_arg.optional ? "true" : "false";
							sprintf(sc_argument_key, "[%d]", b);

							char subcommand_arg_body[187];
							sprintf(subcommand_arg_body, (
								"{"
									"\"name\":\"%s\","
									"\"description\":\"%s\","
									"\"type\":%d,"
									"\"required\":\"%s\""
								"}"
							), subcommand_arg.name, subcommand_arg.description, subcommand_arg.type, subcommand_arg_required);

							jsonelement_t *json_subcommand_arg = json_parse(subcommand_arg_body);
							json_set_val(sc_arguments_body, sc_argument_key, json_subcommand_arg, JSON_OBJECT);
							json_free(json_subcommand_arg, false);
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

			command_node = command_node->next;
		}
	}

	char *body = json_stringify(commands_body, 2);
	json_free(commands_body, false);

	char path[42];
	sprintf(path, "/applications/%s/commands", json_get_val(shivers->client.user, "id").value.string);

	response_free(api_request(shivers->client.token, path, "PUT", body, NULL));
	free(body);
}
