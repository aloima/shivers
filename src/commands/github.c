#define _XOPEN_SOURCE

#include <time.h>
#include <string.h>
#include <stdlib.h>

#include <shivers.h>
#include <discord.h>
#include <network.h>
#include <utils.h>
#include <json.h>

static void execute(struct Client client, jsonelement_t **message, Split args) {
	struct Message reply = {0};
	const char *channel_id = json_get_val(*message, "channel_id").value.string;

	if (args.size != 1) {
		reply.content = MISSING_ARGUMENT;
		send_message(client, channel_id, reply);
		return;
	} else {
		struct Embed embed = {0};
		struct RequestConfig config = {0};

		config.header_size = 1;
		config.headers = allocate(NULL, 0, 1, sizeof(struct Header));
		config.headers[0] = (struct Header) {
			.name = "User-Agent",
			.value = "shivers"
		};

		char url[256] = {0};
		struct Response response = {0};

		if (char_at(args.data[0], '/') == -1) {
			sprintf(url, "https://api.github.com/users/%s", args.data[0]);

			config.url = url;
			config.method = "GET";

			response = request(config);

			if (response.status.code == 404) {
				reply.content = "Not found.";
				send_message(client, channel_id, reply);
			} else {
				jsonelement_t *user = json_parse(response.data);
				jsonresult_t login_data = json_get_val(user, "login");
				jsonresult_t name_data = json_get_val(user, "name");
				jsonresult_t bio = json_get_val(user, "bio");

				char following[24] = {0};
				sprintf(following, "%ld", (unsigned long) json_get_val(user, "following").value.number);

				char followers[24] = {0};
				sprintf(followers, "%ld", (unsigned long) json_get_val(user, "followers").value.number);

				char repositories[24] = {0};
				sprintf(repositories, "%ld", (unsigned long) json_get_val(user, "public_repos").value.number);

				char gists[24] = {0};
				sprintf(gists, "%ld", (unsigned long) json_get_val(user, "public_gists").value.number);

				char given_stars[24] = {0};
				sprintf(url, "https://api.github.com/users/%s/starred", args.data[0]);
				config.url = url;
				jsonelement_t *stars_json = json_parse(response.data);
				sprintf(given_stars, "%ld", stars_json->size);
				json_free(stars_json);

				char joined_at[32] = {0};
				struct tm tm = {0};

				strptime(json_get_val(user, "created_at").value.string, "%Y-%m-%dT%H:%M:%SZ", &tm);
				strftime(joined_at, sizeof(joined_at), "%d %B %Y", &tm);

				char name[128] = {0};
				sprintf(name, "@%s", login_data.value.string);

				if (name_data.type != JSON_NULL && (strcmp(login_data.value.string, name_data.value.string) != 0)) {
					sprintf(name, "%s (@%s)", name_data.value.string, login_data.value.string);
				}

				add_field_to_embed(&embed, "Repositories", repositories, true);
				add_field_to_embed(&embed, "Following", following, true);
				add_field_to_embed(&embed, "Followers", followers, true);
				add_field_to_embed(&embed, "Joined at", joined_at, true);
				add_field_to_embed(&embed, "Given Stars", given_stars, true);
				add_field_to_embed(&embed, "Gists", gists, true);

				embed.thumbnail_url = json_get_val(user, "avatar_url").value.string;
				embed.color = COLOR;
				set_embed_author(&embed, name, json_get_val(user, "html_url").value.string, NULL);

				if (bio.type != JSON_NULL) {
					embed.description = bio.value.string;
				}

				add_embed_to_message(embed, &reply);
				send_message(client, channel_id, reply);
				free_message(reply);
				free(embed.fields);
				json_free(user);
			}
		} else {
			sprintf(url, "https://api.github.com/repos/%s", args.data[0]);

			config.url = url;
			config.method = "GET";

			response = request(config);

			if (response.status.code == 404) {
				reply.content = "Not found.";
				send_message(client, channel_id, reply);
			} else {
				jsonelement_t *repository = json_parse(response.data);
				jsonresult_t description = json_get_val(repository, "description");
				jsonresult_t language = json_get_val(repository, "language");
				jsonresult_t license = json_get_val(repository, "license");

				char stars[24] = {0};
				char watchers[24] = {0};
				char forks[24] = {0};
				sprintf(stars, "%ld", (unsigned long) json_get_val(repository, "stargazers_count").value.number);
				sprintf(watchers, "%ld", (unsigned long) json_get_val(repository, "subscribers_count").value.number);
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

				add_embed_to_message(embed, &reply);
				send_message(client, channel_id, reply);
				free(embed.fields);
				free_message(reply);
				json_free(repository);
			}
		}

		response_free(&response);
		free(config.headers);
	}
}

const struct Command github = {
	.execute = execute,
	.name = "github",
	.description = "Fetches data from GitHub and sends them",
	.args = (struct CommandArgument[]) {
		(struct CommandArgument) {
			.name = "query",
			.description = "The repository or the user that you want to get information",
			.examples = (const char *[]) {"aloima", "torvalds", "aloima/shivers", "torvalds/linux"},
			.example_size = 4,
			.optional = true
		}
	},
	.arg_size = 1
};
