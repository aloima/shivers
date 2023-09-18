#include <discord.h>
#include <json.h>

#ifndef SHIVERS_H_
	#define SHIVERS_H_

	#define PREFIX "s-"
	#define COLOR 0xB8B8B8

	struct Cooldown {
		char *user_id;
		unsigned long timestamp;
	};

	struct CooldownMemory {
		struct Cooldown *cooldowns;
		size_t size;
	};

	struct Command {
		void (*execute)(struct Client client, jsonelement_t **message, Split args);
		char *name;
		char *description;
		char **args;
	};

	void setup_commands();
	void free_commands();
	const struct Command *get_commands();
	const size_t get_command_size();

	void setup_cooldown_memory();
	void free_cooldown_memory();
	void run_with_cooldown(const char *user_id, void (*command)(struct Client client, jsonelement_t **message, Split args), struct Client client, jsonelement_t **message, Split args);

	void add_cooldown(const char *user_id);
	void remove_cooldown(const char *user_id);
	bool has_cooldown(const char *user_id);
	struct Cooldown get_cooldown(const char *user_id);

	void on_ready(struct Client client);
	void on_force_close();
	void on_message_create(struct Client client, jsonelement_t **message);

	extern const struct Command about;
	extern const struct Command avatar;
	extern const struct Command github;
	extern const struct Command help;
	extern const struct Command wikipedia;
#endif
