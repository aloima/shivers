#include <stdio.h>

#include <shivers.h>
#include <discord.h>
#include <json.h>

void on_ready(struct Client client) {
	const char *username = json_get_val(client.user, "username").value.string;

	setup_commands();
	puts("Set up all commands.");

	printf("%s is connected to gateway.\n", username);
}
