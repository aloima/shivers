#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <utils.h>
#include <shivers.h>

struct CooldownMemory *cooldown_memory = NULL;

void setup_cooldown_memory() {
	cooldown_memory = allocate(NULL, 0, 1, sizeof(struct CooldownMemory));
}

void free_cooldown_memory() {
	for (size_t i = 0; i < cooldown_memory->size; ++i) {
		free(cooldown_memory->cooldowns[i].user_id);
	}

	free(cooldown_memory->cooldowns);
	free(cooldown_memory);
}

void run_with_cooldown(char *user_id, void (*command)(Client client, JSONElement **message, Split args), Client client, JSONElement **message, Split args) {
	unsigned long target = (get_cooldown(user_id).timestamp + 3000);
	unsigned long current = get_timestamp(NULL);

	if (target > current) {
		char warning[64] = {0};
		sprintf(warning, "You need to wait `%.2Lf seconds` to use a command.", ((long double) target - (long double) current) / 1000.0);
		send_content(client, json_get_val(*message, "channel_id").value.string, warning);
	} else {
		if (has_cooldown(user_id)) {
			remove_cooldown(user_id);
		}

		add_cooldown(user_id);
		command(client, message, args);
	}
}

void add_cooldown(char *user_id) {
	++cooldown_memory->size;
	cooldown_memory->cooldowns = allocate(cooldown_memory->cooldowns, cooldown_memory->size - 1, cooldown_memory->size, sizeof(struct Cooldown));

	struct Cooldown cooldown;
	cooldown.timestamp = get_timestamp(NULL);
	cooldown.user_id = allocate(NULL, 0, strlen(user_id) + 1, sizeof(char));
	strcpy(cooldown.user_id, user_id);

	memcpy(cooldown_memory->cooldowns + cooldown_memory->size - 1, &cooldown, sizeof(struct Cooldown));
}

void remove_cooldown(char *user_id) {
	size_t at = 0;

	for (size_t i = 0; i < cooldown_memory->size; ++i) {
		if (strcmp(cooldown_memory->cooldowns[i].user_id, user_id) == 0) {
			at = i;
			break;
		}
	}

	for (size_t i = at + 1; i < cooldown_memory->size; ++i) {
		cooldown_memory->cooldowns[i - 1].user_id = allocate(NULL, 0, strlen(cooldown_memory->cooldowns[i + 1].user_id) + 1, sizeof(char));
		memcpy(cooldown_memory->cooldowns + i - 1, cooldown_memory->cooldowns + i, sizeof(struct Cooldown));
	}

	free(cooldown_memory->cooldowns[cooldown_memory->size - 1].user_id);

	--cooldown_memory->size;
	cooldown_memory->cooldowns = allocate(cooldown_memory->cooldowns, cooldown_memory->size + 1, cooldown_memory->size, sizeof(struct Cooldown));
}

bool has_cooldown(char *user_id) {
	bool result = false;

	for (size_t i = 0; i < cooldown_memory->size; ++i) {
		if (strcmp(cooldown_memory->cooldowns[i].user_id, user_id) == 0) {
			result = true;
			break;
		}
	}

	return result;
}

struct Cooldown get_cooldown(char *user_id) {
	struct Cooldown cooldown;
	memset(&cooldown, 0, sizeof(struct Cooldown));

	for (size_t i = 0; i < cooldown_memory->size; ++i) {
		if (strcmp(cooldown_memory->cooldowns[i].user_id, user_id) == 0) {
			cooldown = cooldown_memory->cooldowns[i];
			break;
		}
	}

	return cooldown;
}
