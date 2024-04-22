#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <shivers.h>
#include <utils.h>

static struct Cooldown *cooldowns = NULL;
static unsigned short cooldown_size = 0;

void free_cooldowns() {
	for (unsigned short i = 0; i < cooldown_size; ++i) {
		free(cooldowns[i].user_id);
	}

	free(cooldowns);
}

void run_with_cooldown(const char *user_id, void (*execute)(const struct Client client, const struct InteractionCommand command), const struct Client client, const struct InteractionCommand command) {
	const unsigned long long target = (get_cooldown(user_id).timestamp + 3000);
	const unsigned long long current = get_timestamp();

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

		send_message(client, message);
	} else {
		if (has_cooldown(user_id)) {
			remove_cooldown(user_id);
		}

		add_cooldown(user_id);
		execute(client, command);
	}
}

void add_cooldown(const char *user_id) {
	++cooldown_size;
	cooldowns = allocate(cooldowns, -1, cooldown_size, sizeof(struct Cooldown));

	const unsigned char size_of_user_id = (strlen(user_id) + 1);

	const struct Cooldown cooldown = {
		.timestamp = get_timestamp(),
		.user_id = allocate(NULL, -1, size_of_user_id, sizeof(char))
	};

	memcpy(cooldown.user_id, user_id, size_of_user_id);
	memcpy(cooldowns + cooldown_size - 1, &cooldown, sizeof(struct Cooldown));
}

void remove_cooldown(const char *user_id) {
	unsigned short at = 0;

	for (unsigned short i = 0; i < cooldown_size; ++i) {
		if (strsame(cooldowns[i].user_id, user_id)) {
			at = i;
			break;
		}
	}

	for (unsigned short i = at + 1; i < cooldown_size; ++i) {
		cooldowns[i - 1].user_id = allocate(cooldowns[i - 1].user_id, -1, strlen(cooldowns[i].user_id) + 1, sizeof(char));
		memcpy(cooldowns + i - 1, cooldowns + i, sizeof(struct Cooldown));
	}

	free(cooldowns[cooldown_size - 1].user_id);

	--cooldown_size;

	if (cooldown_size != 0) {
		cooldowns = allocate(cooldowns, -1, cooldown_size, sizeof(struct Cooldown));
	}
}

bool has_cooldown(const char *user_id) {
	bool result = false;

	for (unsigned short i = 0; i < cooldown_size; ++i) {
		if (strsame(cooldowns[i].user_id, user_id)) {
			result = true;
			break;
		}
	}

	return result;
}

struct Cooldown get_cooldown(const char *user_id) {
	struct Cooldown cooldown = {0};

	for (unsigned short i = 0; i < cooldown_size; ++i) {
		const struct Cooldown data = cooldowns[i];

		if (strsame(data.user_id, user_id)) {
			cooldown = data;
			break;
		}
	}

	return cooldown;
}
