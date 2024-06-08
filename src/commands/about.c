#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#if defined(_WIN32)
	#include <winsock2.h>
	#include <windows.h>
	#include <psapi.h>
#elif defined(__linux__)
	#include <unistd.h>
#endif

#include <shivers.h>
#include <discord.h>
#include <utils.h>
#include <json.h>

#define YEAR (60 * 60 * 24 * 30 * 12)
#define MONTH (60 * 60 * 24 * 30)
#define DAY (60 * 60 * 24)
#define HOUR (60 * 60)
#define MINUTE (60)

static void set_uptime_text(struct Client client, char uptime_text[]) {
	unsigned long long seconds = ((get_timestamp() - client.ready_at) / 1000);
	const int years = (seconds / YEAR);
	seconds -= (years * YEAR);
	const int months = (seconds / MONTH);
	seconds -= (months * MONTH);
	const int days = (seconds / DAY);
	seconds -= (days * DAY);
	const int hours = (seconds / HOUR);
	seconds -= (hours * HOUR);
	const int minutes = (seconds / MINUTE);
	seconds -= (minutes * MINUTE);

	struct Join uptime[6];
	int value = -1;

	char sseconds[8], sminutes[8], shours[7], sdays[8], smonths[8], syears[7];

	if (years != 0) {
		++value;

		uptime[value].data = syears;
		uptime[value].length = sprintf(uptime[value].data, "%hi yrs", years);
	}

	if (months != 0) {
		++value;

		uptime[value].data = smonths;
		uptime[value].length = sprintf(uptime[value].data, "%hi mths", months);
	}

	if (days != 0) {
		++value;

		uptime[value].data = sdays;
		uptime[value].length = sprintf(uptime[value].data, "%hi days", days);
	}

	if (hours != 0) {
		++value;

		uptime[value].data = shours;
		uptime[value].length = sprintf(uptime[value].data, "%hi hrs", hours);
	}

	if (minutes != 0) {
		++value;

		uptime[value].data = sminutes;
		uptime[value].length = sprintf(uptime[value].data, "%hi mins", minutes);
	}

	if (seconds != 0) {
		++value;

		uptime[value].data = sseconds;
		uptime[value].length = sprintf(uptime[value].data, "%llu secs", seconds);
	}

  ++value;
	join(uptime, uptime_text, value, " ");
}

static void execute(struct Shivers *shivers, const struct InteractionCommand command) {
	struct Embed embed = {
    .color = COLOR
  };

	struct Message message = {
		.target_type = TARGET_INTERACTION_COMMAND,
		.target = {
			.interaction_command = command
		}
	};

	char memory_usage[11];

	#if defined(_WIN32)
		PROCESS_MEMORY_COUNTERS memory = {0};
		GetProcessMemoryInfo(GetCurrentProcess(), &memory, sizeof(memory));

		sprintf(memory_usage, "%.2f MB", memory.WorkingSetSize / 1024.0 / 1024.0);
	#elif defined(__linux__)
		FILE *statm = fopen("/proc/self/statm", "r");
		unsigned long rss, vram;

		fscanf(statm, "%lu %lu", &vram, &rss);
		sprintf(memory_usage, "%.2f MB", (rss * getpagesize()) / 1024.0 / 1024.0);
	#endif

	char uptime_text[41];
	uptime_text[0] = 0;
	set_uptime_text(shivers->client, uptime_text);

	char guilds[4];
	sprintf(guilds, "%u", shivers->client.guilds->length);

	char latency[7];
	sprintf(latency, "%ums", get_latency());

	char add[110];
	sprintf(add, "[Add me!](https://discord.com/api/v10/oauth2/authorize?client_id=%s&scope=bot&permissions=8)", json_get_val(shivers->client.user, "id").value.string);

	embed.description = add;

	add_field_to_embed(&embed, "Maintainer", "<@840217542400409630>", true);
	add_field_to_embed(&embed, "Memory", memory_usage, true);
	add_field_to_embed(&embed, "Servers", guilds, true);
	add_field_to_embed(&embed, "Latency", latency, true);
	add_field_to_embed(&embed, "Uptime", uptime_text, true);
	add_field_to_embed(&embed, "Github", "[aloima/shivers](https://github.com/aloima/shivers)", true);

	#if defined(__clang__)
		char footer[33];
		sprintf(footer, "Compiled using clang %d.%d.%d", __clang_major__, __clang_minor__, __clang_patchlevel__);

		set_embed_footer(&embed, footer, NULL);
	#elif defined(__GNUC__)
		char footer[31];
		sprintf(footer, "Compiled using gcc %d.%d.%d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);

		set_embed_footer(&embed, footer, NULL);
	#endif

	add_embed_to_message_payload(embed, &(message.payload));
	send_message(shivers->client, message);
	free(embed.fields);
	free_message_payload(message.payload);
}

struct Command about = {
	.execute = execute,
	.description = "Gives information about the bot",
	.guild_only = false
};
