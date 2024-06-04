#include <stdbool.h>

#include <hash.h>
#include <json.h>
#include <utils.h>
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

	enum Status {
		OFFLINE,
		ONLINE,
		IDLE,
		DND
	};

	struct Client {
		jsonelement_t *user;
		unsigned long long ready_at;
		char *token;
		struct HashMap *guilds;
	};

	struct Member {
		enum Status status;
		bool at_voice, bot;
	};

	struct Guild {
		struct HashMap *members;
		unsigned int total_member_count, member_at_voice_count, non_offline_count, bot_count, channel_count;
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
		unsigned int field_size;
	};

	struct File {
		char *name, *data, *type;
		unsigned long size;
	};

	enum InteractionCommandTargetTypes {
		TARGET_INTERACTION_COMMAND = 1,
		TARGET_CHANNEL
	};

	enum ArgumentTypes {
		SUBCOMMAND_ARGUMENT = 1,
		SUBCOMMAND_GROUP_ARGUMENT,
		STRING_ARGUMENT,
		INTEGER_ARGUMENT,
		BOOLEAN_ARGUMENT,
		USER_ARGUMENT,
		CHANNEL_ARGUMENT,
		ROLE_ARGUMENT
	};

	struct MessagePayload {
		char *content;
		unsigned int embed_size, file_size;
		struct Embed *embeds;
		struct File *files;
		bool ephemeral;
	};

	struct InteractionSubcommand {
		struct InteractionArgument *arguments;
		unsigned int argument_size;
	};

	struct InteractionArgument {
		char *name;
		enum ArgumentTypes type;

		union {
			struct String string;
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
		unsigned int argument_size;
	};

	struct Message {
		union {
			struct InteractionCommand interaction_command;
			char *channel_id;
		} target;

		struct MessagePayload payload;
		enum InteractionCommandTargetTypes target_type;
	};

	void connect_gateway(const char *bot_token, const char *url, const unsigned int bot_intents);
	unsigned int get_latency();
	void set_presence(const char *name, const char *state, const char *details, const char type, const char *status);

	struct Response api_request(const char *token, const char *path, const char *method, const char *body, const struct FormData *formdata);
	struct Guild *get_guild(const struct Client client, const char *id);
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
