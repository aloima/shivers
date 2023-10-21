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

static void set_uptime_text(const struct Client client, char uptime_text[]) {
	long seconds = ((get_timestamp(NULL) - client.ready_at) / 1000);
	char years = ((seconds - (seconds % SECONDS_IN_YEAR)) / SECONDS_IN_YEAR);
	seconds -= (years * SECONDS_IN_YEAR);
	char months = ((seconds - (seconds % SECONDS_IN_MONTH)) / SECONDS_IN_MONTH);
	seconds -= (months * SECONDS_IN_MONTH);
	char days = ((seconds - (seconds % SECONDS_IN_DAY)) / SECONDS_IN_DAY);
	seconds -= (days * SECONDS_IN_DAY);
	char hours = ((seconds - (seconds % SECONDS_IN_HOUR)) / SECONDS_IN_HOUR);
	seconds -= (hours * SECONDS_IN_HOUR);
	char minutes = ((seconds - (seconds % SECONDS_IN_MINUTE)) / SECONDS_IN_MINUTE);
	seconds -= (minutes * SECONDS_IN_MINUTE);

	char *uptime[6] = {0};
	unsigned char uptime_element_count = 0;

	if (years != 0) {
		++uptime_element_count;
		uptime[uptime_element_count - 1] = allocate(NULL, -1, 7, sizeof(char));
		sprintf(uptime[uptime_element_count - 1], "%hi yrs", years);
	}

	if (months != 0) {
		++uptime_element_count;
		uptime[uptime_element_count - 1] = allocate(NULL, -1, 8, sizeof(char));
		sprintf(uptime[uptime_element_count - 1], "%hi mths", months);
	}

	if (days != 0) {
		++uptime_element_count;
		uptime[uptime_element_count - 1] = allocate(NULL, -1, 8, sizeof(char));
		sprintf(uptime[uptime_element_count - 1], "%hi days", days);
	}

	if (hours != 0) {
		++uptime_element_count;
		uptime[uptime_element_count - 1] = allocate(NULL, -1, 7, sizeof(char));
		sprintf(uptime[uptime_element_count - 1], "%hi hrs", hours);
	}

	if (minutes != 0) {
		++uptime_element_count;
		uptime[uptime_element_count - 1] = allocate(NULL, -1, 8, sizeof(char));
		sprintf(uptime[uptime_element_count - 1], "%hi mins", minutes);
	}

	if (seconds != 0) {
		++uptime_element_count;
		uptime[uptime_element_count - 1] = allocate(NULL, 0, 8, sizeof(char));
		sprintf(uptime[uptime_element_count - 1], "%ld secs", seconds);
	}

	join(uptime, uptime_text, uptime_element_count, " ");

	for (unsigned char i = 0; i < uptime_element_count; ++i) {
		free(uptime[i]);
	}
}

static void execute(struct Client client, jsonelement_t *message, Split args) {
	struct Message reply = {0};
	struct Embed embed = {0};

	struct rusage r_usage;
	char memory_usage[16];

	getrusage(RUSAGE_SELF, &r_usage);
	sprintf(memory_usage, "%.2f MB", r_usage.ru_maxrss / 1024.0);

	char uptime_text[48];
	uptime_text[0] = 0;
	set_uptime_text(client, uptime_text);

	char guilds[4];
	sprintf(guilds, "%ld", get_guilds_cache()->size);

	char latency[8];
	sprintf(latency, "%dms", get_latency());

	char add[112];
	sprintf(add, "[Add me!](https://discord.com/api/v10/oauth2/authorize?client_id=%s&scope=bot&permissions=8)", json_get_val(client.user, "id").value.string);

	embed.color = COLOR;
	embed.description = add;

	add_field_to_embed(&embed, "Developer", "<@840217542400409630>", true);
	add_field_to_embed(&embed, "Memory", memory_usage, true);
	add_field_to_embed(&embed, "Servers", guilds, true);
	add_field_to_embed(&embed, "Latency", latency, true);
	add_field_to_embed(&embed, "Uptime", uptime_text, true);
	add_field_to_embed(&embed, "Github", "[aloima/shivers](https://github.com/aloima/shivers)", true);

	#if defined(__clang__)
		char footer[64];
		sprintf(footer, "Compiled with clang %d.%d.%d", __clang_major__, __clang_minor__, __clang_patchlevel__);

		set_embed_footer(&embed, footer, NULL);
	#elif defined(__GNUC__)
		char footer[64];
		sprintf(footer, "Compiled with gcc %d.%d.%d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);

		set_embed_footer(&embed, footer, NULL);
	#endif

	add_embed_to_message(embed, &reply);
	send_message(client, json_get_val(message, "channel_id").value.string, reply);
	free(embed.fields);
	free_message(reply);
}

const struct Command about = {
	.execute = execute,
	.name = "about",
	.description = "Sends bot information",
	.args = NULL,
	.arg_size = 0
};
