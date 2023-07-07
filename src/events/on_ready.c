#include <stdio.h>

#include <json.h>
#include <discord.h>

void on_ready(Client client) {
	printf("%s is connected to gateway.\n", json_get_val(client.user, "username").string);
}
