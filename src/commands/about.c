#include <stdio.h>
#include <stdlib.h>

#include <sys/resource.h>

#include <shivers.h>
#include <discord.h>
#include <utils.h>
#include <json.h>

#define SECONDS_IN_YEAR (60 * 60 * 24 * 30 * 12)
#define SECONDS_IN_MONTH (60 * 60 * 24 * 30)
#define SECONDS_IN_DAY (60 * 60 * 24)
#define SECONDS_IN_HOUR (60 * 60)
#define SECONDS_IN_MINUTE (60)

static void execute(Client client, JSONElement **message, Split args) {
	Embed embed = {0};

	struct rusage r_usage;
	char memory_usage[32] = {0};

	getrusage(RUSAGE_SELF, &r_usage);
	sprintf(memory_usage, "%.2f MB", (double) r_usage.ru_maxrss / 1024);

	char guilds[8] = {0};
	sprintf(guilds, "%ld", get_cache_size(get_guilds_cache()));

	char latency[8] = {0};
	sprintf(latency, "%dms", get_latency());

	char **uptime = NULL;
	size_t uptime_element_count = 0;
	long seconds = ((get_timestamp(NULL) - client.ready_at) / 1000);
	short years = ((seconds - (seconds % SECONDS_IN_YEAR)) / SECONDS_IN_YEAR);
	seconds -= (years * SECONDS_IN_YEAR);
	short months = ((seconds - (seconds % SECONDS_IN_MONTH)) / SECONDS_IN_MONTH);
	seconds -= (months * SECONDS_IN_MONTH);
	short days = ((seconds - (seconds % SECONDS_IN_DAY)) / SECONDS_IN_DAY);
	seconds -= (days * SECONDS_IN_DAY);
	short hours = ((seconds - (seconds % SECONDS_IN_HOUR)) / SECONDS_IN_HOUR);
	seconds -= (hours * SECONDS_IN_HOUR);
	short minutes = ((seconds - (seconds % SECONDS_IN_MINUTE)) / SECONDS_IN_MINUTE);
	seconds -= (minutes * SECONDS_IN_MINUTE);

	if (years != 0) {
		++uptime_element_count;
		uptime = allocate(uptime, uptime_element_count - 1, uptime_element_count, sizeof(char *));
		uptime[uptime_element_count - 1] = allocate(NULL, 0, 8, sizeof(char));
		sprintf(uptime[uptime_element_count - 1], "%hi yrs", years);
	}

	if (months != 0) {
		++uptime_element_count;
		uptime = allocate(uptime, uptime_element_count - 1, uptime_element_count, sizeof(char *));
		uptime[uptime_element_count - 1] = allocate(NULL, 0, 8, sizeof(char));
		sprintf(uptime[uptime_element_count - 1], "%hi mths", months);
	}

	if (days != 0) {
		++uptime_element_count;
		uptime = allocate(uptime, uptime_element_count - 1, uptime_element_count, sizeof(char *));
		uptime[uptime_element_count - 1] = allocate(NULL, 0, 8, sizeof(char));
		sprintf(uptime[uptime_element_count - 1], "%hi days", days);
	}

	if (hours != 0) {
		++uptime_element_count;
		uptime = allocate(uptime, uptime_element_count - 1, uptime_element_count, sizeof(char *));
		uptime[uptime_element_count - 1] = allocate(NULL, 0, 8, sizeof(char));
		sprintf(uptime[uptime_element_count - 1], "%hi hrs", hours);
	}

	if (minutes != 0) {
		++uptime_element_count;
		uptime = allocate(uptime, uptime_element_count - 1, uptime_element_count, sizeof(char *));
		uptime[uptime_element_count - 1] = allocate(NULL, 0, 8, sizeof(char));
		sprintf(uptime[uptime_element_count - 1], "%hi mins", minutes);
	}
	if (seconds != 0) {
		++uptime_element_count;
		uptime = allocate(uptime, uptime_element_count - 1, uptime_element_count, sizeof(char *));
		uptime[uptime_element_count - 1] = allocate(NULL, 0, 8, sizeof(char));
		sprintf(uptime[uptime_element_count - 1], "%ld secs", seconds);
	}

	char uptime_text[128] = {0};
	join(uptime, uptime_text, uptime_element_count, " ");

	for (short i = 0; i < uptime_element_count; ++i) {
		free(uptime[i]);
	}

	free(uptime);

	char add[128] = {0};
	sprintf(add, "[Add me!](https://discord.com/api/v10/oauth2/authorize?client_id=%s&scope=bot&permissions=8)", json_get_val(client.user, "id").value.string);

	embed.color = COLOR;
	embed.description = add;

	add_field_to_embed(&embed, "Developer", "<@840217542400409630>", true);
	add_field_to_embed(&embed, "Memory usage", memory_usage, true);
	add_field_to_embed(&embed, "Guilds", guilds, true);
	add_field_to_embed(&embed, "Latency", latency, true);
	add_field_to_embed(&embed, "Uptime", uptime_text, true);
	add_field_to_embed(&embed, "Github", "[aloima/shivers](https://github.com/aloima/shivers)", true);

	send_embed(client, json_get_val(*message, "channel_id").value.string, embed);
	free(embed.fields);
}

struct Command about = {
	.execute = execute,
	.name = "about",
	.description = "Sends bot information",
	.args = NULL
};
