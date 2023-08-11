#include <string.h>
#include <stdio.h>

#include <json.h>
#include <shivers.h>
#include <utils.h>

static void execute(Client client, JSONElement **message, Split args) {
	Embed embed;
	memset(&embed, 0, sizeof(Embed));

	char text[4096] = {0};
	sprintf(text, (
		"```\\n"
		"Help page\\n"
		"---------\\n\\n"
		"- about     | Sends bot information\\n"
		"+ avatar    | Sends an avatar image\\n"
		"+ github    | Fetches data from GitHub and sends them\\n"
		"+ help      | Sends help page\\n"
		"+ wikipedia | Sends short info from Wikipedia\\n\\n"

		"If character before command is +, this command accepts/requires usage of argument(s), if it is -, the command does not.\\n"
		"To see usage of a command, use `help [command]`.\\n"
		"```"
	));

	embed.color = COLOR;
	embed.description = text;
	send_embed(client, json_get_val(*message, "channel_id").value.string, embed);
}

struct Command help = {
	.execute = execute,
	.name = "help",
	.description = "Sends help page",
	.args = NULL
};
