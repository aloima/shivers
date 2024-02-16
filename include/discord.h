#include <stdbool.h>

#include <json.h>
#include <network.h>

#ifndef DISCORD_H_
	#define DISCORD_H_

	#define AVATAR_URL "https://cdn.discordapp.com/avatars/%s/%s.%s"
	#define DEFAULT_AVATAR_URL "https://cdn.discordapp.com/embed/avatars/%d.png"

	struct Cache {
		char **data;
		unsigned long size;
	};

	struct Client {
		jsonelement_t *user;
		unsigned long ready_at;
		char *token;
	};

	struct EmbedField {
		char *name;
		char *value;
		bool is_inline;
	};

	struct EmbedAuthor {
		char *name;
		char *url;
		char *icon_url;
	};

	struct EmbedFooter {
		char *text;
		char *icon_url;
	};

	struct Embed {
		char *title;
		char *description;
		double color;
		char *image_url;
		char *thumbnail_url;
		struct EmbedAuthor author;
		struct EmbedFooter footer;
		struct EmbedField *fields;
		short field_size;
	};

	struct File {
		char *name;
		char *data;
		char *type;
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
		struct Embed *embeds;
		unsigned char embed_size;
		struct File *files;
		unsigned char file_size;
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
				jsonelement_t *user_data;
				jsonelement_t *member_data;
			} user;

			jsonelement_t *role;
			jsonelement_t *channel;
			struct InteractionSubcommand subcommand;
		} value;
	};

	struct InteractionCommand {
		char *id;
		char *token;

		char *guild_id;
		char *channel_id;
		jsonelement_t *user;
		char *name;
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

	void connect_gateway(const char *token);
	int get_latency();
	void set_presence(const char *name, const char type, const char *status);

	void clear_cache(struct Cache *cache);
	void add_to_cache(struct Cache *cache, const char *data);
	void remove_from_cache_index(struct Cache *cache, const unsigned long index);

	struct Cache *get_guilds_cache();

	unsigned int get_all_intents();
	struct Response api_request(const char *token, const char *path, const char *method, const char *body, const struct FormData *formdata);
	void get_avatar_url(char *url, const char *token, const char *user_id, const char *discriminator, const char *hash, const bool force_png, const short size);

	unsigned short send_message(const struct Client client, const struct Message message);
	void free_message_payload(struct MessagePayload message_payload);

	void add_field_to_embed(struct Embed *embed, const char *name, const char *value, const bool is_inline);
	void set_embed_author(struct Embed *embed, const char *name, const char *url, const char *icon_url);
	void set_embed_footer(struct Embed *embed, const char *text, const char *icon_url);
	void add_embed_to_message_payload(const struct Embed embed, struct MessagePayload *message_payload);
	void add_file_to_message_payload(struct MessagePayload *message_payload, const char *name, const char *data, const unsigned long size, const char *type);

	bool check_snowflake(const char *snowflake);
#endif
