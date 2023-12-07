#include <discord.h>
#include <json.h>

#ifndef SHIVERS_H_
	#define SHIVERS_H_

	#define PREFIX "s-"
	#define COLOR (double) 0xB8B8B8

	struct Cooldown {
		char *user_id;
		unsigned long timestamp;
	};

	struct CommandArgument {
		const char *name;
		const char *description;
		const char **examples;
		const size_t example_size;
		const bool optional;
	};

	struct Command {
		void (*execute)(struct Client client, jsonelement_t *message, const struct Split args);
		const char *name;
		const char *description;
		const struct CommandArgument *args;
		const size_t arg_size;
	};

	void setup_commands();
	void free_commands();
	const struct Command *get_commands();
	const unsigned short get_command_size();

	void free_cooldowns();
	void run_with_cooldown(const char *user_id, void (*command)(struct Client client, jsonelement_t *message, const struct Split args), struct Client client, jsonelement_t *message, const struct Split args);

	void add_cooldown(const char *user_id);
	void remove_cooldown(const char *user_id);
	bool has_cooldown(const char *user_id);
	struct Cooldown get_cooldown(const char *user_id);

	#define INVALID_ARGUMENT "Invalid argument, please use `help` command."
	#define MISSING_ARGUMENT "Missing argument, please use `help` command."
	#define ADDITIONAL_ARGUMENT "Additional argument, please use `help` command."

	void on_ready(struct Client client);
	void on_handle_guilds(struct Client client);
	void on_force_close();
	void on_message_create(struct Client client, jsonelement_t *message);

	extern const struct Command about;
	extern const struct Command avatar;
	extern const struct Command github;
	extern const struct Command help;
	extern const struct Command level;
	extern const struct Command wikipedia;
#endif
