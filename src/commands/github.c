#define _XOPEN_SOURCE

#include <time.h>
#include <string.h>
#include <stdlib.h>

#include <shivers.h>
#include <discord.h>
#include <network.h>
#include <utils.h>
#include <json.h>

static void execute(struct Client client, jsonelement_t *message, const struct Split args) {
	struct Message reply = {0};
	const char *channel_id = json_get_val(message, "channel_id").value.string;

	if (args.size == 0) {
		reply.content = MISSING_ARGUMENT;
		send_message(client, channel_id, reply);
		return;
	} else if (args.size != 1) {
		reply.content = ADDITIONAL_ARGUMENT;
		send_message(client, channel_id, reply);
		return;
	} else {
		struct Header headers[] = {
			(struct Header) {
				.name = "User-Agent",
				.value = "shivers"
			}
		};

		struct Embed embed = {0};
		struct RequestConfig config = {
			.method = "GET",
			.headers = headers,
			.header_size = 1
		};

		char url[256];
		struct Response response = {0};

		if (char_at(args.data[0].data, '/') == -1) {
			sprintf(url, "https://api.github.com/users/%s", args.data[0].data);

			config.url = url;
			response = request(config);

			if (response.status.code == 404) {
				reply.content = "Not found.";
				send_message(client, channel_id, reply);
			} else {
				jsonelement_t *user = json_parse((const char *) response.data);
				jsonresult_t login_data = json_get_val(user, "login");
				jsonresult_t name_data = json_get_val(user, "name");
				jsonresult_t bio = json_get_val(user, "bio");

				char following[8];
				sprintf(following, "%ld", (unsigned long) json_get_val(user, "following").value.number);

				char followers[8];
				sprintf(followers, "%ld", (unsigned long) json_get_val(user, "followers").value.number);

				char repositories[6];
				sprintf(repositories, "%ld", (unsigned long) json_get_val(user, "public_repos").value.number);

				char gists[6];
				sprintf(gists, "%ld", (unsigned long) json_get_val(user, "public_gists").value.number);

				char joined_at[18];
				struct tm tm;

				strptime(json_get_val(user, "created_at").value.string, "%Y-%m-%dT%H:%M:%SZ", &tm);
				strftime(joined_at, sizeof(joined_at), "%d %B %Y", &tm);

				char name[128];
				sprintf(name, "@%s", login_data.value.string);

				if (name_data.type != JSON_NULL && (strcmp(login_data.value.string, name_data.value.string) != 0)) {
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

				add_embed_to_message(embed, &reply);
				send_message(client, channel_id, reply);
				free_message(reply);
				free(embed.fields);
				json_free(user, false);
			}
		} else {
			sprintf(url, "https://api.github.com/repos/%s", args.data[0].data);

			config.url = url;
			response = request(config);

			if (response.status.code == 404) {
				reply.content = "Not found.";
				send_message(client, channel_id, reply);
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

				add_embed_to_message(embed, &reply);
				send_message(client, channel_id, reply);
				free(embed.fields);
				free_message(reply);
				json_free(repository, false);
			}
		}

		response_free(&response);
	}
}

static const struct CommandArgument args[] = {
	(struct CommandArgument) {
		.name = "query",
		.description = "The repository or the user that you want to get information",
		.examples = (const char *[]) {"aloima", "torvalds", "aloima/shivers", "torvalds/linux"},
		.example_size = 4,
		.optional = true
	}
};

const struct Command github = {
	.execute = execute,
	.name = "github",
	.description = "Fetches data from GitHub and sends them",
	.args = args,
	.arg_size = sizeof(args) / sizeof(struct CommandArgument)
};
