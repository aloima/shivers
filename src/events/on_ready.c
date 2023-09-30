#include <stdio.h>

#include <shivers.h>
#include <discord.h>
#include <json.h>

void on_ready(struct Client client) {
	setup_commands();

	printf("%s is connected to gateway.\n", json_get_val(client.user, "username").value.string);
}
