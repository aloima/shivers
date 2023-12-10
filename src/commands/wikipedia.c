#include <stdio.h>
#include <stdbool.h>

#include <shivers.h>
#include <discord.h>
#include <network.h>
#include <utils.h>
#include <json.h>

static void execute(struct Client client, jsonelement_t *message, const struct Split args) {
	struct Message reply = {0};
	const char *channel_id = json_get_val(message, "channel_id").value.string;

	if (args.size == 0) {
		reply.content = "Missing argument, please use `help` command.";
		send_message(client, channel_id, reply);
		return;
	} else {
		struct RequestConfig config = {
			.method = "GET"
		};

		char search_query[512];
		struct Join arg_joins[args.size];

		for (int i = 0; i < args.size; ++i) {
			arg_joins[i].data = args.data[i].data;
			arg_joins[i].length = args.data[i].length;
		}

		join(arg_joins, search_query, args.size, " ");

		char search_url[512];
		sprintf(search_url, "https://en.wikipedia.org/w/api.php?action=opensearch&search=%s", search_query);
		config.url = search_url;

		struct Response search_response = request(config);
		jsonelement_t *search_result = json_parse((const char *) search_response.data);
		jsonresult_t page_name = json_get_val(search_result, "1.0");

		if (page_name.exist) {
			char url[512];
			sprintf(url, "https://en.wikipedia.org/w/api.php?action=query&prop=pageprops&titles=%s&format=json", page_name.value.string);
			config.url = url;

			struct Response info_response = request(config);
			jsonelement_t *info_data = json_parse((const char *) info_response.data);
			jsonelement_t *page_info = ((jsonelement_t **) json_get_val(info_data, "query.pages").value.object->value)[0];
			jsonresult_t page_description = json_get_val(page_info, "pageprops.wikibase-shortdesc");

			if (!page_description.exist) {
				response_free(&info_response);

				const char *page_id = page_info->key;
				sprintf(url, "https://en.wikipedia.org/w/api.php?action=query&format=json&pageids=%s&redirects", page_id);
				config.url = url;
				json_free(info_data, false);

				struct Response redirect_response = request(config);
				jsonelement_t *redirect_result = json_parse((const char *) redirect_response.data);
				response_free(&redirect_response);

				page_name = json_get_val(redirect_result, "query.redirects.[0].to");
				sprintf(url, "https://en.wikipedia.org/w/api.php?action=query&prop=pageprops&titles=%s&format=json", page_name.value.string);
				json_free(redirect_result, false);

				config.url = url;
				info_response = request(config);

				info_data = json_parse((const char *) info_response.data);
				page_info = ((jsonelement_t **) json_get_val(info_data, "query.pages").value.object->value)[0];
				page_description = json_get_val(page_info, "pageprops.wikibase-shortdesc");
			}

			struct Embed embed = {0};

			const char *title = json_get_val(page_info, "title").value.string;
			char page_url[512], *encoded_page_url = NULL;
			sprintf(page_url, "https://en.wikipedia.org/wiki/%s", title);
			encoded_page_url = percent_encode(page_url);

			embed.color = COLOR;
			embed.description = page_description.value.string;
			set_embed_author(&embed, title, encoded_page_url, NULL);

			jsonresult_t image_name = json_get_val(page_info, "pageprops.page_image_free");

			if (image_name.exist) {
				char image_url[512];
				sprintf(image_url, "https://en.wikipedia.org/w/api.php?action=query&titles=File:%s&prop=imageinfo&iiprop=url&format=json", image_name.value.string);
				config.url = image_url;

				struct Response image_response = request(config);
				jsonelement_t *image_data = json_parse((const char *) image_response.data);
				jsonelement_t *image_info = ((jsonelement_t **) json_get_val(image_data, "query.pages").value.object->value)[0];
				const char *final_image_url = json_get_val(image_info, "imageinfo.0.url").value.string;
				const size_t final_image_url_length = strlen(final_image_url);

				if (strncmp(final_image_url + final_image_url_length - 3, "svg", 3) == 0) {
					char *svg_url = json_get_val(image_info, "imageinfo.0.url").value.string;
					char png_url[512];
					struct Split svg_splitter = split(svg_url, strlen(svg_url), "/");
					char image_code[32];
					image_code[0] = 0;

					struct Join svg_joins[2];

					for (unsigned char i = 0; i < 2; ++i) {
						svg_joins[i].data = svg_splitter.data[i + 5].data;
						svg_joins[i].length = svg_splitter.data[i + 5].length;
					}

					join(svg_joins, image_code, 2, "/");
					split_free(svg_splitter);

					sprintf(png_url, "https://upload.wikimedia.org/wikipedia/commons/thumb/%s/%s/1024px-%s.png", image_code, image_name.value.string, image_name.value.string);
					embed.image_url = png_url;
				} else {
					embed.image_url = (char *) final_image_url;
				}


				add_embed_to_message(embed, &reply);
				send_message(client, channel_id, reply);
				response_free(&image_response);
				json_free(image_data, false);
			} else {
				add_embed_to_message(embed, &reply);
				send_message(client, channel_id, reply);
			}

			response_free(&info_response);
			free_message(reply);
			json_free(info_data, false);
			free(encoded_page_url);
		} else {
			reply.content = "Not found.";
			send_message(client, channel_id, reply);
		}

		response_free(&search_response);
		json_free(search_result, false);
	}
}

const static struct CommandArgument args[] = {
	(struct CommandArgument) {
		.name = "query",
		.description = "The search query of the page which you want to get information",
		.examples = (const char *[]) {"federal republic of germany", "ottoman"},
		.example_size = 2,
		.optional = true
	}
};

const struct Command wikipedia = {
	.execute = execute,
	.name = "wikipedia",
	.description = "Sends short info from Wikipedia",
	.args = args,
	.arg_size = sizeof(args) / sizeof(struct CommandArgument)
};
