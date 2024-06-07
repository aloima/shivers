#include <discord.h>
#include <json.h>
#include <hash.h>

#ifndef SHIVERS_H_
	#define SHIVERS_H_

	#define COLOR (double) 0xB8B8B8

	struct Shivers {
		struct Client client;
		struct HashMap *commands;
	};

	struct Cooldown {
		char *user_id;
		unsigned long long timestamp;
	};

	struct VoiceStatsChannel {
		char *name, *id;
	};

	struct CommandArgument {
		char *name, *description;
		enum ArgumentTypes type;
		unsigned int arg_size;
		bool optional;
		struct CommandArgument *args;
	};

	struct Command {
		void (*execute)(const struct Client client, const struct InteractionCommand command);
		char *description;
		bool guild_only;
		unsigned long permissions;
		struct CommandArgument *args;
		unsigned int arg_size;
	};

	void setup_commands(struct Shivers *shivers);

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

	void update_voice_stats(const struct Client client, const char *guild_id);
	void prepare_voice_stats_channel_name(const struct Client client, char **channel_name, const char *guild_id);

	#define INVALID_ARGUMENT "Invalid argument, please use `help` command."
	#define MISSING_ARGUMENT "Missing argument, please use `help` command."
	#define ADDITIONAL_ARGUMENT "Additional argument, please use `help` command."

	#define MISSING_MANAGE_CHANNELS "Could not open a new channel, because I do not have required permissions."

	void on_force_close(struct Shivers *shivers);
	void on_guild_create(struct Shivers *shivers);
	void on_guild_delete(struct Shivers *shivers);
	void on_guild_member_add(struct Shivers *shivers, struct Node *guild_node);
	void on_guild_member_remove(struct Shivers *shivers, struct Node *guild_node);
	void on_handle_guilds(struct Shivers *shivers);
	void on_interaction_command(struct Shivers *shivers, struct InteractionCommand command);
	void on_message_create(struct Shivers *shivers, jsonelement_t *message);
	void on_presence_update(struct Shivers *shivers, struct Node *guild_node);
	void on_ready(struct Shivers *shivers);
	void on_voice_state_update(struct Shivers *shivers, struct Node *guild_node);

	extern struct Command about;
	extern struct Command avatar;
	extern struct Command github;
	extern struct Command help;
	extern struct Command level;
	extern struct Command level_settings;
	extern struct Command vstats;
	extern struct Command wikipedia;
#endif
