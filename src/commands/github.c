#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <shivers.h>
#include <discord.h>
#include <network.h>
#include <utils.h>
#include <json.h>

static void execute(const struct Client client, const struct InteractionCommand command) {
	struct Embed embed = {0};
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

	char url[256];
	struct Response response;

	const char *query = command.arguments[0].value.string;

	if (char_at(query, '/') == -1) {
		sprintf(url, "https://api.github.com/users/%s", query);

		config.url = url;
		response = request(config);

		if (response.status.code == 404) {
			message.payload.content = "Not found.";
			message.payload.ephemeral = true;
			send_message(client, message);
		} else {
			jsonelement_t *user = json_parse((const char *) response.data);
			const jsonresult_t login_data = json_get_val(user, "login");
			const jsonresult_t name_data = json_get_val(user, "name");
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

			char name[128];
			sprintf(name, "@%s", login_data.value.string);

			if (name_data.type != JSON_NULL && !strsame(login_data.value.string, name_data.value.string)) {
				sprintf(name, "%s (@%s)", name_data.value.string, login_data.value.string);
			}

			add_field_to_embed(&embed, "Repositories", repositories, true);
			add_field_to_embed(&embed, "Following", following, true);
			add_field_to_embed(&embed, "Followers", followers, true);
			add_field_to_embed(&embed, "Joined at", joined_at, true);
			add_field_to_embed(&embed, "Gists", gists, true);

			embed.thumbnail_url = json_get_val(user, "avatar_url").value.string;
			embed.color = COLOR;
			set_embed_author(&embed, name, json_get_val(user, "html_url").value.string, NULL);

			if (bio.type != JSON_NULL) {
				embed.description = bio.value.string;
			}

			add_embed_to_message_payload(embed, &(message.payload));
			send_message(client, message);
			free_message_payload(message.payload);
			free(embed.fields);
			json_free(user, false);
		}
	} else {
		sprintf(url, "https://api.github.com/repos/%s", query);

		config.url = url;
		response = request(config);

		if (response.status.code == 404) {
			message.payload.content = "Not found.";
			message.payload.ephemeral = true;
			send_message(client, message);
		} else {
			jsonelement_t *repository = json_parse((const char *) response.data);
			jsonresult_t description = json_get_val(repository, "description");
			jsonresult_t language = json_get_val(repository, "language");
			jsonresult_t license = json_get_val(repository, "license");

			char stars[8];
			sprintf(stars, "%ld", (unsigned long) json_get_val(repository, "stargazers_count").value.number);

			char watchers[8];
			sprintf(watchers, "%ld", (unsigned long) json_get_val(repository, "subscribers_count").value.number);

			char forks[8];
			sprintf(forks, "%ld", (unsigned long) json_get_val(repository, "forks_count").value.number);

			add_field_to_embed(&embed, "Stars", stars, true);
			add_field_to_embed(&embed, "Watchers", watchers, true);
			add_field_to_embed(&embed, "Forks", forks, true);
			add_field_to_embed(&embed, "Is archived?", json_get_val(repository, "archived").value.boolean ? "Yes" : "No", true);
			add_field_to_embed(&embed, "Major language", language.type != JSON_NULL ? language.value.string : "None", true);
			add_field_to_embed(&embed, "License", license.type != JSON_NULL ? json_get_val(license.value.object, "spdx_id").value.string : "None", true);

			embed.color = COLOR;
			set_embed_author(&embed, json_get_val(repository, "full_name").value.string, json_get_val(repository, "html_url").value.string, NULL);

			if (description.type != JSON_NULL) {
				embed.description = description.value.string;
			}

			add_embed_to_message_payload(embed, &(message.payload));
			send_message(client, message);
			free_message_payload(message.payload);
			free(embed.fields);
			json_free(repository, false);
		}
	}

	response_free(response);
}

static const struct CommandArgument args[] = {
	(struct CommandArgument) {
		.name = "query",
		.description = "The repository or the user that you want to get information",
		.type = STRING_ARGUMENT,
		.optional = false
	}
};

const struct Command github = {
	.execute = execute,
	.name = "github",
	.description = "Fetches data from GitHub and sends them",
	.guild_only = false,
	.args = args,
	.arg_size = sizeof(args) / sizeof(struct CommandArgument)
};
