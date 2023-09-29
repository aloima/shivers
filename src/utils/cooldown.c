#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <shivers.h>
#include <utils.h>

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

void run_with_cooldown(const char *user_id, void (*command)(struct Client client, jsonelement_t *message, Split args), struct Client client, jsonelement_t *message, Split args) {
	const unsigned long target = (get_cooldown(user_id).timestamp + 3000);
	const unsigned long current = get_timestamp(NULL);

	if (target > current) {
		struct Message reply = {0};
		char warning[64] = {0};
		sprintf(warning, "You need to wait `%.2Lf seconds` to use a command.", ((long double) target - (long double) current) / 1000.0);
		reply.content = warning;

		send_message(client, json_get_val(message, "channel_id").value.string, reply);
	} else {
		if (has_cooldown(user_id)) {
			remove_cooldown(user_id);
		}

		add_cooldown(user_id);
		command(client, message, args);
	}
}

void add_cooldown(const char *user_id) {
	++cooldown_memory->size;
	cooldown_memory->cooldowns = allocate(cooldown_memory->cooldowns, cooldown_memory->size - 1, cooldown_memory->size, sizeof(struct Cooldown));

	struct Cooldown cooldown = {
		.timestamp = get_timestamp(NULL),
		.user_id = allocate(NULL, 0, strlen(user_id) + 1, sizeof(char))
	};

	strcpy(cooldown.user_id, user_id);

	memcpy(cooldown_memory->cooldowns + cooldown_memory->size - 1, &cooldown, sizeof(struct Cooldown));
}

void remove_cooldown(const char *user_id) {
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

bool has_cooldown(const char *user_id) {
	bool result = false;

	for (size_t i = 0; i < cooldown_memory->size; ++i) {
		if (strcmp(cooldown_memory->cooldowns[i].user_id, user_id) == 0) {
			result = true;
			break;
		}
	}

	return result;
}

struct Cooldown get_cooldown(const char *user_id) {
	struct Cooldown cooldown = {0};

	for (size_t i = 0; i < cooldown_memory->size; ++i) {
		struct Cooldown data = cooldown_memory->cooldowns[i];

		if (strcmp(data.user_id, user_id) == 0) {
			cooldown = data;
			break;
		}
	}

	return cooldown;
}
