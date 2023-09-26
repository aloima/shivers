#include <stdio.h>
#include <stdbool.h>

#include <shivers.h>
#include <discord.h>
#include <network.h>
#include <utils.h>
#include <json.h>

// TODO: add url escaping for ' ' => "%20"

static void execute(struct Client client, jsonelement_t **message, Split args) {
	struct Message reply = {0};
	const char *channel_id = json_get_val(*message, "channel_id").value.string;

	if (args.size == 0) {
		reply.content = "Missing argument, please use `help` command.";
		send_message(client, channel_id, reply);
		return;
	} else {
		struct RequestConfig config = {0};
		config.method = "GET";

		char search_query[512] = {0};
		join(args.data, search_query, args.size, " ");

		char search_url[512] = {0};
		sprintf(search_url, "https://en.wikipedia.org/w/api.php?action=opensearch&search=%s", search_query);
		config.url = search_url;

		struct Response search_response = request(config);
		jsonelement_t *search_result = json_parse(search_response.data);
		jsonresult_t page_name = json_get_val(search_result, "1.0");

		if (page_name.exist) {
			char url[512] = {0};
			sprintf(url, "https://en.wikipedia.org/w/api.php?action=query&prop=pageprops&titles=%s&format=json", page_name.value.string);
			config.url = url;

			struct Response info_response = request(config);
			jsonelement_t *info_data = json_parse(info_response.data);
			jsonelement_t *page_info = ((jsonelement_t **) json_get_val(info_data, "query.pages").value.object->value)[0];
			struct Embed embed = {0};

			char *title = json_get_val(page_info, "title").value.string;
			char page_url[512] = {0};
			sprintf(page_url, "https://en.wikipedia.org/wiki/%s", title);

			embed.color = COLOR;
			embed.description = json_get_val(page_info, "pageprops.wikibase-shortdesc").value.string;
			set_embed_author(&embed, title, page_url, NULL);

			jsonresult_t image_name = json_get_val(page_info, "pageprops.page_image_free");

			if (image_name.exist) {
				char image_url[512] = {0};
				sprintf(image_url, "https://en.wikipedia.org/w/api.php?action=query&titles=File:%s&prop=imageinfo&iiprop=url&format=json", image_name.value.string);
				config.url = image_url;

				struct Response image_response = request(config);
				jsonelement_t *image_data = json_parse(image_response.data);
				jsonelement_t *image_info = ((jsonelement_t **) json_get_val(image_data, "query.pages").value.object->value)[0];
				embed.image_url = json_get_val(image_info, "imageinfo.0.url").value.string;

				add_embed_to_message(embed, &reply);
				send_message(client, channel_id, reply);
				response_free(&image_response);
				json_free(image_data);
			} else {
				add_embed_to_message(embed, &reply);
				send_message(client, channel_id, reply);
			}

			response_free(&info_response);
			free_message(reply);
			json_free(info_data);
		} else {
			reply.content = "Not found.";
			send_message(client, channel_id, reply);
		}

		response_free(&search_response);
		json_free(search_result);
	}
}

const struct Command wikipedia = {
	.execute = execute,
	.name = "wikipedia",
	.description = "Sends short info from Wikipedia",
	.args = (struct CommandArgument[]) {
		(struct CommandArgument) {
			.name = "query",
			.description = "The search query of the page which you want to get information",
			.examples = (const char *[]) {"federal republic of germany", "ottoman"},
			.example_size = 2,
			.optional = true
		}
	},
	.arg_size = 1
};
