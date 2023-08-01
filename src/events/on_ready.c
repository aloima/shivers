#include <stdio.h>

#include <shivers.h>
#include <discord.h>
#include <json.h>

void on_ready(Client client) {
	setup_cooldown_memory();

	printf("%s is connected to gateway.\n", json_get_val(client.user, "username").value.string);
}
