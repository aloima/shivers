#include <stdio.h>

#include <shivers.h>
#include <discord.h>

void on_guild_create(struct Shivers *shivers) {
  char custom_status[12];
  sprintf(custom_status, "%u servers", shivers->client.guilds->length);
  set_presence("custom", custom_status, NULL, 4, "online");
}
