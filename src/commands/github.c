#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <shivers.h>
#include <discord.h>
#include <network.h>
#include <utils.h>
#include <json.h>

static void execute(const struct Client client, const struct InteractionCommand command) {
	struct Embed embed = {
		.color = COLOR
	};

	struct Message message = {
		.target_type = TARGET_INTERACTION_COMMAND,
		.target = {
			.interaction_command = command
		}
	};

	struct Header headers[] = {
		(struct Header) {
			.name = "User-Agent",
			.value = "shivers"
		}
	};

	struct RequestConfig config = {
		.method = "GET",
		.headers = headers,
		.header_size = 1
	};

	const struct String query = command.arguments[0].value.string;
	char url[30 + query.length];
	struct Response response;

	if (char_at(query.value, '/') == -1) {
		sprintf(url, "https://api.github.com/users/%s", query.value);

		config.url = url;
		response = request(config);

		if (response.status.code == 404) {
			message.payload = (struct MessagePayload) {
				.content = "Not found.",
				.ephemeral = true
			};

			send_message(client, message);
		} else {
			jsonelement_t *user = json_parse((const char *) response.data);
			const jsonresult_t json_login = json_get_val(user, "login");
			const jsonresult_t json_name = json_get_val(user, "name");
			const jsonresult_t bio = json_get_val(user, "bio");

			char following[8];
			sprintf(following, "%u", (unsigned int) json_get_val(user, "following").value.number);

			char followers[8];
			sprintf(followers, "%u", (unsigned int) json_get_val(user, "followers").value.number);

			char repositories[6];
			sprintf(repositories, "%u", (unsigned int) json_get_val(user, "public_repos").value.number);

			char gists[6];
			sprintf(gists, "%u", (unsigned int) json_get_val(user, "public_gists").value.number);

			char joined_at[18];
			const char *joined_at_string = json_get_val(user, "created_at").value.string;
			const char *months[12] = {
				"January", "February", "March", "April", "May", "June",
				"July", "August", "September", "October", "November", "December"
			};
			char month_string[3];
			memcpy(month_string, joined_at_string + 5, 2);
			month_string[2] = 0;

			sprintf(joined_at, "%.2s %s %.4s", joined_at_string + 8, months[atoi_s(month_string, -1) - 1], joined_at_string);

			char name[json_login.element->size + 1];
			sprintf(name, "@%s", json_login.value.string);

			if (json_name.element->type != JSON_NULL && !strsame(json_login.value.string, json_name.value.string)) {
				sprintf(name, "%s (@%s)", json_name.value.string, json_login.value.string);
			}

			add_field_to_embed(&embed, "Repositories", repositories, true);
			add_field_to_embed(&embed, "Following", following, true);
			add_field_to_embed(&embed, "Followers", followers, true);
			add_field_to_embed(&embed, "Joined at", joined_at, true);
			add_field_to_embed(&embed, "Gists", gists, true);

			embed.thumbnail_url = json_get_val(user, "avatar_url").value.string;
			set_embed_author(&embed, name, json_get_val(user, "html_url").value.string, NULL);

			if (bio.element->type != JSON_NULL) {
				embed.description = bio.value.string;
			}

			add_embed_to_message_payload(embed, &(message.payload));
			send_message(client, message);

			free_message_payload(message.payload);
			json_free(user, false);
			free(embed.fields);
		}
	} else {
		sprintf(url, "https://api.github.com/repos/%s", query.value);

		config.url = url;
		response = request(config);

		if (response.status.code == 404) {
			message.payload = (struct MessagePayload) {
				.content = "Not found.",
				.ephemeral = true
			};

			send_message(client, message);
		} else {
			jsonelement_t *repository = json_parse((const char *) response.data);
			jsonresult_t description = json_get_val(repository, "description");
			jsonresult_t language = json_get_val(repository, "language");
			jsonresult_t license = json_get_val(repository, "license");

			char stars[8];
			sprintf(stars, "%lu", (unsigned long) json_get_val(repository, "stargazers_count").value.number);

			char watchers[8];
			sprintf(watchers, "%lu", (unsigned long) json_get_val(repository, "subscribers_count").value.number);

			char forks[8];
			sprintf(forks, "%lu", (unsigned long) json_get_val(repository, "forks_count").value.number);

			add_field_to_embed(&embed, "Stars", stars, true);
			add_field_to_embed(&embed, "Watchers", watchers, true);
			add_field_to_embed(&embed, "Forks", forks, true);
			add_field_to_embed(&embed, "Is archived?", json_get_val(repository, "archived").value.boolean ? "Yes" : "No", true);
			add_field_to_embed(&embed, "Major language", language.element->type != JSON_NULL ? language.value.string : "None", true);
			add_field_to_embed(&embed, "License", license.element->type != JSON_NULL ? json_get_val(license.element, "spdx_id").value.string : "None", true);

			embed.color = COLOR;
			set_embed_author(&embed, json_get_val(repository, "full_name").value.string, json_get_val(repository, "html_url").value.string, NULL);

			if (description.element->type != JSON_NULL) {
				embed.description = description.value.string;
			}

			add_embed_to_message_payload(embed, &(message.payload));
			send_message(client, message);

			free_message_payload(message.payload);
			json_free(repository, false);
			free(embed.fields);
		}
	}

	response_free(response);
}

static struct CommandArgument args[] = {
	(struct CommandArgument) {
		.name = "query",
		.description = "The repository or the user that you want to get information",
		.type = STRING_ARGUMENT,
		.optional = false
	}
};

struct Command github = {
	.execute = execute,
	.description = "Fetches data from GitHub and sends them",
	.guild_only = false,
	.args = args,
	.arg_size = sizeof(args) / sizeof(struct CommandArgument)
};
