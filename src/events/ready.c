#include <stdio.h>

#include <database.h>
#include <shivers.h>
#include <discord.h>
#include <json.h>
#include <hash.h>
#include <png.h>

void on_ready(struct Shivers *shivers) {
  setup_commands(shivers);
  puts("Set up all commands.");

  shivers->cooldowns = create_hashmap(16);
  puts("Cooldowns are initialized.");

  database_initialize("database.json");
  puts("Database is initialized.");

  initialize_fonts();
  puts("Fonts are initialized.");

  printf("%s is connected to gateway.\n", json_get_val(shivers->client.user, "username").value.string);
}
