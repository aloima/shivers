#include <discord.h>
#include <json.h>

#ifndef SHIVERS_H_
	#define SHIVERS_H_

	#define PREFIX "s-"
	#define COLOR 0xB8B8B8

	void on_ready(Client client);
	void on_message_create(Client client, JSONElement **message);

	void about(Client client, JSONElement **message);
	void avatar(Client client, JSONElement **message, Split args);
	void github(Client client, JSONElement **message, Split args);
#endif
