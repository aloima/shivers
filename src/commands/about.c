#include <shivers.h>

typedef struct TimeUnitFormat {
  char src[8];
  int value;
  char *format;
} TimeUnitFormat;

#define YEAR (60 * 60 * 24 * 30 * 12)
#define MONTH (60 * 60 * 24 * 30)
#define DAY (60 * 60 * 24)
#define HOUR (60 * 60)
#define MINUTE (60)

static void set_uptime_text(struct Client client, char uptime_text[]) {
  const int64_t now = get_timestamp();
  if (now == -1)
    throw(GET_TIMESTAMP_ERROR);

  const uint64_t seconds = (now - client.ready_at) / 1000;

  const int years   = (seconds / YEAR);
  const int months  = (seconds % YEAR) / MONTH;
  const int days    = (seconds % MONTH) / DAY;
  const int hours   = (seconds % DAY) / HOUR;
  const int minutes = (seconds % HOUR) / MINUTE;
  const int secs    = seconds % MINUTE;

  struct Join uptime[6];
  uint8_t count = 0;

  const TimeUnitFormat pairs[6] = {
    {"", years, "%hi yrs"},
    {"", months, "%hi mths"},
    {"", days, "%hi days"},
    {"", hours, "%hi hrs"},
    {"", minutes, "%hi mins"},
    {"", secs, "%hi secs"},
  };

  for (uint8_t i = 0; i < 6; ++i) {
    const TimeUnitFormat *pair = &pairs[i];

    if (pair->value != 0) {
      char *src = (char *) pair->src;
      const int length = SPRINTF_S(src, pair->format, pair->value);

      uptime[count].data = src;
      uptime[count].length = length;
      count += 1;
    }
  }

  join(uptime, uptime_text, count, " ");
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
  // GetCurrentProcess() is guaranteed method.
  PROCESS_MEMORY_COUNTERS memory = {0};
  ASSERT(GetProcessMemoryInfo(GetCurrentProcess(), &memory, sizeof(memory)), ==, true);

  SPRINTF_S(memory_usage, "%.2f MB", memory.WorkingSetSize / 1024.0 / 1024.0);
#elif defined(__linux__)
  uint64_t rss, vram;
  FILE *statm = fopen("/proc/self/statm", "r");
  if (statm == NULL)
    throw("fopen(): cannot open statm file to get memory usage");

  ASSERT(fscanf(statm, "%lu %lu", &vram, &rss), >, 0);
  SPRINTF_S(memory_usage, "%.2f MB", (rss * getpagesize()) / 1024.0 / 1024.0);
#endif

  char uptime_text[41];
  uptime_text[0] = 0;
  set_uptime_text(shivers->client, uptime_text);

  char guilds[4];
  SPRINTF_S(guilds, "%u", shivers->client.guilds->length);

  char latency[7];
  SPRINTF_S(latency, "%ums", get_latency());

  #define ADD_MESSAGE ("[Add me!](https://discord.com/api/v10/oauth2/authorize?client_id=%s&scope=bot&permissions=8)")
  const jsonresult_t id_prop = json_get_val(shivers->client.user, "id");
  ASSERT(id_prop.exist, ==, true);

  char add[110];
  SPRINTF_S(add, ADD_MESSAGE, id_prop.value.string);

  embed.description = add;

  add_field_to_embed(&embed, "Maintainer", "<@840217542400409630>", true);
  add_field_to_embed(&embed, "Memory", memory_usage, true);
  add_field_to_embed(&embed, "Servers", guilds, true);
  add_field_to_embed(&embed, "Latency", latency, true);
  add_field_to_embed(&embed, "Uptime", uptime_text, true);
  add_field_to_embed(&embed, "Github", "[aloima/shivers](https://github.com/aloima/shivers)", true);

  #if defined(__clang__)
    char footer[33];
    SPRINTF_S(footer, "Compiled using clang %d.%d.%d", __clang_major__, __clang_minor__, __clang_patchlevel__);
  #elif defined(__GNUC__)
    char footer[31];
    SPRINTF_S(footer, "Compiled using gcc %d.%d.%d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
  #endif

  set_embed_footer(&embed, footer, NULL);

  add_embed_to_message_payload(embed, &(message.payload));
  ASSERT(send_message(shivers->client, message), ==, HTTP_NO_CONTENT);

  free_embed(embed);
  free_message_payload(message.payload);
}

struct Command about = {
  .execute = execute,
  .description = "Gives information about the bot",
  .guild_only = false
};
