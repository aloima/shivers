#include <discord.h>
#include <json.h>

#ifndef SHIVERS_H_
	#define SHIVERS_H_

	#define COLOR (double) 0xB8B8B8

	struct Cooldown {
		char *user_id;
		unsigned long timestamp;
	};

	struct CommandArgument {
		const char *name, *description;
		const unsigned char type, arg_size;
		const bool optional;
		const struct CommandArgument *args;
	};

	struct Command {
		void (*execute)(const struct Client client, const struct InteractionCommand command);
		const char *name, *description;
		const bool guild_only;
		const struct CommandArgument *args;
		const unsigned char arg_size;
	};

	void setup_commands(const struct Client client);
	void free_commands();
	const struct Command *get_commands();
	const unsigned short get_command_size();

	void free_cooldowns();
	void run_with_cooldown(
		const char *user_id,
		void (*execute)(const struct Client client, const struct InteractionCommand command),
		struct Client client,
		const struct InteractionCommand
	);

	void add_cooldown(const char *user_id);
	void remove_cooldown(const char *user_id);
	bool has_cooldown(const char *user_id);
	struct Cooldown get_cooldown(const char *user_id);

	#define INVALID_ARGUMENT "Invalid argument, please use `help` command."
	#define MISSING_ARGUMENT "Missing argument, please use `help` command."
	#define ADDITIONAL_ARGUMENT "Additional argument, please use `help` command."

	void on_force_close();
	void on_guild_create(struct Client client);
	void on_guild_delete(struct Client client);
	void on_handle_guilds(struct Client client);
	void on_interaction_command(struct Client client, struct InteractionCommand command);
	void on_message_create(struct Client client, jsonelement_t *message);
	void on_ready(struct Client client);

	extern const struct Command about;
	extern const struct Command avatar;
	extern const struct Command github;
	extern const struct Command help;
	extern const struct Command level;
	extern const struct Command level_settings;
	extern const struct Command wikipedia;
#endif
