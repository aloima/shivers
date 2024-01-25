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
	const unsigned long target = (get_cooldown(user_id).timestamp + 3000);
	const unsigned long current = get_timestamp(NULL);

	if (target > current) {
		char warning[51];
		sprintf(warning, "You need to wait `%.2Lf seconds` to use a command.", ((long double) target - (long double) current) / 1000.0);

		struct Message message = {
			.target_type = TARGET_INTERACTION_COMMAND,
			.target = {
				.interaction_command = command
			},
			.payload = {
				.content = warning
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

	const struct Cooldown cooldown = {
		.timestamp = get_timestamp(NULL),
		.user_id = allocate(NULL, -1, strlen(user_id) + 1, sizeof(char))
	};

	strcpy(cooldown.user_id, user_id);
	memcpy(cooldowns + cooldown_size - 1, &cooldown, sizeof(struct Cooldown));
}

void remove_cooldown(const char *user_id) {
	unsigned short at = 0;

	for (unsigned short i = 0; i < cooldown_size; ++i) {
		if (strcmp(cooldowns[i].user_id, user_id) == 0) {
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
		if (strcmp(cooldowns[i].user_id, user_id) == 0) {
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

		if (strcmp(data.user_id, user_id) == 0) {
			cooldown = data;
			break;
		}
	}

	return cooldown;
}
