#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <shivers.h>
#include <utils.h>
#include <hash.h>

void run_with_cooldown(const char *user_id, void (*execute)(struct Shivers *shivers, const struct InteractionCommand command), struct Shivers *shivers, const struct InteractionCommand command) {
	const struct Node *cooldown = get_node(shivers->cooldowns, user_id);
	unsigned long long target = (cooldown ? (*((unsigned long long *) cooldown->value) + 3000) : 0);
	unsigned long long current = get_timestamp();

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
			memcpy(get_node(shivers->cooldowns, user_id)->value, &current, sizeof(unsigned long long));
		} else {
			insert_node(shivers->cooldowns, user_id, &current, sizeof(unsigned long long));
		}

		execute(shivers, command);
	}
}
