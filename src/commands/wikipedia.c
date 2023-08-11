#include <stdio.h>

#include <json.h>
#include <shivers.h>
#include <discord.h>
#include <network.h>
#include <utils.h>

void wikipedia(Client client, JSONElement **message, Split args) {
	if (args.size != 1) {
		send_content(client, json_get_val(*message, "channel_id").value.string, "Missing argument, please use `help` command.");
		return;
	} else {
		RequestConfig config;
		memset(&config, 0, sizeof(RequestConfig));

		char url[512] = {0};
		sprintf(url, "https://en.wikipedia.org/w/api.php?action=query&prop=pageprops&titles=%s&format=json", args.data[0]);
		config.url = url;
		config.method = "GET";

		Response info_response = request(config);
		JSONElement *info_data = json_parse(info_response.data);
		JSONElement *page_info = ((JSONElement **) json_get_val(info_data, "query.pages").value.object->value)[0];
		Embed embed;
		memset(&embed, 0, sizeof(Embed));

		char *title = json_get_val(page_info, "title").value.string;
		char page_url[512] = {0};
		sprintf(page_url, "https://en.wikipedia.org/wiki/%s", title);

		embed.color = COLOR;
		embed.description = json_get_val(page_info, "pageprops.wikibase-shortdesc").value.string;
		set_embed_author(&embed, title, page_url, NULL);

		JSONResult image_name = json_get_val(page_info, "pageprops.page_image_free");

		if (image_name.exist) {
			char image_url[512] = {0};
			sprintf(image_url, "https://en.wikipedia.org/w/api.php?action=query&titles=File:%s&prop=imageinfo&iiprop=url&format=json", image_name.value.string);

			config.url = image_url;
			Response image_response = request(config);
			JSONElement *image_data = json_parse(image_response.data);
			JSONElement *image_info = ((JSONElement **) json_get_val(image_data, "query.pages").value.object->value)[0];
			embed.image_url = json_get_val(image_info, "imageinfo.0.url").value.string;

			send_embed(client, json_get_val(*message, "channel_id").value.string, embed);
			response_free(&image_response);
			json_free(image_data);
		} else {
			send_embed(client, json_get_val(*message, "channel_id").value.string, embed);
		}

		response_free(&info_response);
		json_free(info_data);
	}
}
