#include <stdlib.h>

#include <shivers.h>
#include <database.h>
#include <discord.h>
#include <utils.h>
#include <json.h>

static void execute(struct Client client, jsonelement_t *message, Split args) {
	struct Message reply = {0};
	struct Embed embed = {0};

	const char *user_id = json_get_val(message, "author.id").value.string;
	char xp_key[22], level_key[25];
	sprintf(xp_key, "%s.xp", user_id);
	sprintf(level_key, "%s.level", user_id);

	char xp[12], level[12];
	sprintf(xp, "%.0f", database_has(xp_key) ? database_get(xp_key).number : 0.0);
	sprintf(level, "%.0f", database_has(level_key) ? database_get(level_key).number : 0.0);

	embed.color = COLOR;

	add_field_to_embed(&embed, "Level", level, true);
	add_field_to_embed(&embed, "Experience", xp, true);

	add_embed_to_message(embed, &reply);
	send_message(client, json_get_val(message, "channel_id").value.string, reply);
	free(embed.fields);
	free_message(reply);
}

const struct Command level = {
	.execute = execute,
	.name = "level",
	.description = "Display your level"
};
