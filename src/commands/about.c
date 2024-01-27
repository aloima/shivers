#include <stdio.h>
#include <stdlib.h>

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

#define SECONDS_IN_YEAR (60 * 60 * 24 * 30 * 12)
#define SECONDS_IN_MONTH (60 * 60 * 24 * 30)
#define SECONDS_IN_DAY (60 * 60 * 24)
#define SECONDS_IN_HOUR (60 * 60)
#define SECONDS_IN_MINUTE (60)

static void set_uptime_text(const struct Client client, char uptime_text[]) {
	long seconds = ((get_timestamp(NULL) - client.ready_at) / 1000);
	const char years = ((seconds - (seconds % SECONDS_IN_YEAR)) / SECONDS_IN_YEAR);
	seconds -= (years * SECONDS_IN_YEAR);
	const char months = ((seconds - (seconds % SECONDS_IN_MONTH)) / SECONDS_IN_MONTH);
	seconds -= (months * SECONDS_IN_MONTH);
	const char days = ((seconds - (seconds % SECONDS_IN_DAY)) / SECONDS_IN_DAY);
	seconds -= (days * SECONDS_IN_DAY);
	const char hours = ((seconds - (seconds % SECONDS_IN_HOUR)) / SECONDS_IN_HOUR);
	seconds -= (hours * SECONDS_IN_HOUR);
	const char minutes = ((seconds - (seconds % SECONDS_IN_MINUTE)) / SECONDS_IN_MINUTE);
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

	struct Join uptime_joins[uptime_element_count];
	create_join_elements_nz(uptime_joins, (const char **) uptime, uptime_element_count);
	join(uptime_joins, uptime_text, uptime_element_count, " ");

	for (unsigned char i = 0; i < uptime_element_count; ++i) {
		free(uptime[i]);
	}
}

static void execute(const struct Client client, const struct InteractionCommand command) {
	struct Embed embed = {0};
	struct Message message = {
		.target_type = TARGET_INTERACTION_COMMAND,
		.target = {
			.interaction_command = command
		}
	};

	char memory_usage[11];

	#if defined(_WIN32)
		HINSTANCE hProcHandle = GetModuleHandle(NULL);
		PPROCESS_MEMORY_COUNTERS memory = {0};
		GetProcessMemoryInfo(hProcHandle, memory, sizeof(memory));

		sprintf(memory_usage, "%.2f MB", memory->WorkingSetSize / 1024.0 / 1024.0);
	#elif defined(__linux__)
		FILE *statm = fopen("/proc/self/statm", "r");
		unsigned long rss, vram;

		fscanf(statm, "%lu %lu", &vram, &rss);
		sprintf(memory_usage, "%.2f MB", (rss * getpagesize()) / 1024.0 / 1024.0);
	#endif

	char uptime_text[41];
	uptime_text[0] = 0;
	set_uptime_text(client, uptime_text);

	char guilds[4];
	sprintf(guilds, "%ld", get_guilds_cache()->size);

	char latency[7];
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
		char footer[32];
		sprintf(footer, "Compiled with clang %d.%d.%d", __clang_major__, __clang_minor__, __clang_patchlevel__);

		set_embed_footer(&embed, footer, NULL);
	#elif defined(__GNUC__)
		char footer[30];
		sprintf(footer, "Compiled with gcc %d.%d.%d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);

		set_embed_footer(&embed, footer, NULL);
	#endif

	add_embed_to_message_payload(embed, &(message.payload));
	send_message(client, message);
	free(embed.fields);
	free_message_payload(message.payload);
}

const struct Command about = {
	.execute = execute,
	.name = "about",
	.description = "Sends bot information"
};
