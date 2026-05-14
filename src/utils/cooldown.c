#include <shivers.h>

void run_with_cooldown(const char *user_id, void (*execute)(struct Shivers *shivers, const struct InteractionCommand command), struct Shivers *shivers, const struct InteractionCommand command) {
  const struct Node *cooldown = get_node(shivers->cooldowns, user_id);
  const uint64_t target = (cooldown ? (*((uint64_t *) cooldown->value) + 3000) : 0);
  int64_t current = get_timestamp();
  if (current == -1)
    throw(GET_TIMESTAMP_ERROR);

  if (target > current) {
    char warning[51];
    sprintf(warning, "You need to wait `%.2f seconds` to use a command.", (target - current) / 1000.0);

    struct Message message = {
      .target_type = TARGET_INTERACTION_COMMAND,
      .target = {
        .interaction_command = command
      },
      .payload = {
        .content = warning,
        .ephemeral = true
      }
    };

    send_message(shivers->client, message);
  } else {
    if (cooldown != NULL) {
      memcpy(get_node(shivers->cooldowns, user_id)->value, &current, sizeof(int64_t));
    } else {
      insert_node(shivers->cooldowns, user_id, &current, sizeof(int64_t));
    }

    execute(shivers, command);
  }
}
