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
		void (*execute)(Client client, JSONElement **message, Split args);
		char *name;
		char *description;
		char **args;
	};

	void setup_commands();
	const struct Command *get_commands();
	const size_t get_command_size();

	void setup_cooldown_memory();
	void free_cooldown_memory();
	void run_with_cooldown(char *user_id, void (*command)(Client client, JSONElement **message, Split args), Client client, JSONElement **message, Split args);

	void add_cooldown(char *user_id);
	void remove_cooldown(char *user_id);
	bool has_cooldown(char *user_id);
	struct Cooldown get_cooldown(char *user_id);

	void on_ready(Client client);
	void on_force_close();
	void on_message_create(Client client, JSONElement **message);

	extern struct Command about;
	extern struct Command avatar;
	extern struct Command github;
	extern struct Command help;
	extern struct Command wikipedia;
#endif
