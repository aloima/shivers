#include <shivers.h>
#include <utils.h>
#include <json.h>
#include <hash.h>

void on_interaction_command(struct Shivers *shivers, const struct InteractionCommand interaction_command) {
  const char *user_id = json_get_val(interaction_command.user, "id").value.string;
  const struct Command *command = get_node(shivers->commands, interaction_command.name)->value;

  run_with_cooldown(user_id, command->execute, shivers, interaction_command);
}
