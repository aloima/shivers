#include <stdbool.h>

#include <json.h>
#include <network.h>

#ifndef DISCORD_H_
	#define DISCORD_H_

	#define AVATAR_URL "https://cdn.discordapp.com/avatars/%s/%s.%s"
	#define DEFAULT_AVATAR_URL "https://cdn.discordapp.com/embed/avatars/%d.png"

	#define KickMembers (1 << 1)
	#define BanMembers (1 << 2)
	#define Administrator (1 << 3)
	#define ManageChannels (1 << 4)
	#define ManageGuilds (1 << 5)
	#define ManageMessages (1 << 13)
	#define ManageRoles (1 << 28)
	#define ManageThreads (1 << 34)

	struct Client {
		jsonelement_t *user;
		unsigned long long ready_at;
		char *token;
	};

	struct Guild {
		char *id;
		unsigned long long member_count;
		unsigned long long online_count;
		char **online_members;
		unsigned long long bot_count;
		unsigned long long ban_count;
		unsigned short channel_count;
		unsigned long long member_at_voice_count;
	};

	struct EmbedField {
		char *name, *value;
		bool is_inline;
	};

	struct EmbedAuthor {
		char *name, *url, *icon_url;
	};

	struct EmbedFooter {
		char *text, *icon_url;
	};

	struct Embed {
		char *title, *description, *image_url, *thumbnail_url;
		double color;
		struct EmbedAuthor author;
		struct EmbedFooter footer;
		struct EmbedField *fields;
		unsigned char field_size;
	};

	struct File {
		char *name, *data, *type;
		unsigned long size;
	};

	#define TARGET_INTERACTION_COMMAND 1
	#define TARGET_CHANNEL 2

	#define SUBCOMMAND_ARGUMENT 1
	#define SUBCOMMAND_GROUP_ARGUMENT 2
	#define STRING_ARGUMENT 3
	#define INTEGER_ARGUMENT 4
	#define BOOLEAN_ARGUMENT 5
	#define USER_ARGUMENT 6
	#define CHANNEL_ARGUMENT 7
	#define ROLE_ARGUMENT 8

	struct MessagePayload {
		char *content;
		unsigned char embed_size, file_size;
		struct Embed *embeds;
		struct File *files;
		bool ephemeral;
	};

	struct InteractionSubcommand {
		struct InteractionArgument *arguments;
		unsigned char argument_size;
	};

	struct InteractionArgument {
		char *name;
		unsigned char type;

		union {
			char *string;
			long number;
			bool boolean;

			struct {
				jsonelement_t *user_data, *member_data;
			} user;

			jsonelement_t *role, *channel;
			struct InteractionSubcommand subcommand;
		} value;
	};

	struct InteractionCommand {
		char *id, *token, *guild_id, *channel_id, *name;
		jsonelement_t *user;
		struct InteractionArgument *arguments;
		unsigned char argument_size;
	};

	struct Message {
		union {
			struct InteractionCommand interaction_command;
			char *channel_id;
		} target;

		struct MessagePayload payload;
		unsigned char target_type;
	};

	void connect_gateway(const char *bot_token, const char *url, const unsigned int bot_intents);
	unsigned int get_latency();
	void set_presence(const char *name, const char *state, const char *details, const char type, const char *status);

	void clear_guilds();
	struct Guild *get_guilds();
	unsigned int get_guild_count();
	void add_guild_to_cache(struct Guild guild);
	struct Guild *get_guild_from_cache(const char *id);
	void remove_guild_from_cache(const char *id);

	struct Response api_request(const char *token, const char *path, const char *method, const char *body, const struct FormData *formdata);
	void get_avatar_url(char *url, const char *user_id, const char *discriminator, const char *hash, const bool force_png, const short size);

	unsigned short send_message(const struct Client client, const struct Message message);
	void free_message_payload(struct MessagePayload message_payload);

	void add_field_to_embed(struct Embed *embed, const char *name, const char *value, const bool is_inline);
	void set_embed_author(struct Embed *embed, const char *name, const char *url, const char *icon_url);
	void set_embed_footer(struct Embed *embed, const char *text, const char *icon_url);
	void add_embed_to_message_payload(const struct Embed embed, struct MessagePayload *message_payload);
	void add_file_to_message_payload(struct MessagePayload *message_payload, const char *name, const char *data, const unsigned long size, const char *type);

	bool check_snowflake(const char *snowflake);
#endif
