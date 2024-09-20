#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#if defined(_WIN32)
  #include <winsock2.h>
#elif defined(__linux__)
  #include <sys/select.h>
  #include <sys/socket.h>
  #include <unistd.h>
  #include <arpa/inet.h>
  #include <netdb.h>
#endif

#include <openssl/ssl.h>
#include <openssl/sha.h>
#include <openssl/err.h>

#include <network.h>
#include <utils.h>

static void handle_events(struct Websocket *websocket);

static void switch_protocols(struct Websocket *websocket);
static void check_response(struct Websocket *websocket, const char *response, char *key);

static void initialize_websocket(struct Websocket *websocket, const char *url);

static void handle_events(struct Websocket *websocket) {
  if (FD_ISSET(websocket->sockfd, &(websocket->readfds))) {
    if (websocket->key) {
      char buffer[4096];
      unsigned int read_count = s_read(websocket->ssl, websocket->sockfd, buffer, 4095);
      buffer[read_count] = 0;

      if (strncmp(buffer + 9, "101", 3) == 0) {
        check_response(websocket, buffer, websocket->key);
        free(websocket->key);
        websocket->key = NULL;

        if (websocket->methods.onstart) {
          websocket->methods.onstart();
        }
      } else {
        throw_network("invalid http status code", !!websocket->ssl);
      }
    } else {
      struct WebsocketFrame frame;
      unsigned char buffer[9];

      s_read(websocket->ssl, websocket->sockfd, (char *) buffer, 2);

      frame.fin = ((buffer[0] >> 7) & 0x1);
      frame.rsv[0] = ((buffer[0] >> 6) & 0x1);
      frame.rsv[1] = ((buffer[0] >> 5) & 0x1);
      frame.rsv[2] = ((buffer[0] >> 4) & 0x1);
      frame.opcode = (buffer[0] & 0xF);

      frame.mask = ((buffer[1] >> 7) & 0x1);
      frame.payload_length = (buffer[1] & 0x7F);

      if (frame.payload_length == 126) {
        s_read(websocket->ssl, websocket->sockfd, (char *) buffer, 2);
        frame.payload_length = combine_bytes(buffer, 2);
      } else if (frame.payload_length == 127) {
        s_read(websocket->ssl, websocket->sockfd, (char *) buffer, 8);
        frame.payload_length = combine_bytes(buffer, 8);
      }

      if (frame.mask == 0x1) {
        s_read(websocket->ssl, websocket->sockfd, (char *) buffer, 4);
      }

      frame.payload = allocate(NULL, -1, frame.payload_length + 1, sizeof(char));
      frame.payload[frame.payload_length] = 0;

      switch (frame.opcode) {
        case 0x1: {
          unsigned long received = 0;

          while (received != frame.payload_length) {
            const unsigned long diff = (frame.payload_length - received);

            if (diff > 512) {
              received += s_read(websocket->ssl, websocket->sockfd, frame.payload + received, 512);
            } else {
              received += s_read(websocket->ssl, websocket->sockfd, frame.payload + received, diff);
            }
          }

          if (websocket->methods.onmessage) {
            websocket->methods.onmessage(frame);
          }

          break;
        }

        case 0x8: {
          s_read(websocket->ssl, websocket->sockfd, frame.payload, frame.payload_length);

          const short code = combine_bytes((unsigned char *) frame.payload, 2);
          const char *reason = (frame.payload + 2);
          close_websocket(websocket, code, (reason[0] == 0) ? NULL : reason);
          break;
        }
      }

      free(frame.payload);
    }
  } else if (FD_ISSET(websocket->sockfd, &(websocket->writefds))) {
    if (!websocket->connected && !websocket->closed) {
      switch_protocols(websocket);
    } else if (websocket->queue_size != 0) {
      const struct WebsocketQueueElement element = websocket->queue[0];

      s_write(websocket->ssl, websocket->sockfd, element.data, element.size);

      for (unsigned int i = 0; i < websocket->queue_size; ++i) {
        if ((i + 1) != websocket->queue_size) {
          const struct WebsocketQueueElement new_element = websocket->queue[i + 1];
          websocket->queue[i].data = allocate(websocket->queue[i].data, -1, new_element.size + 1, sizeof(char));
          websocket->queue[i].size = new_element.size;
          strncpy(websocket->queue[i].data, new_element.data, new_element.size);
        }
      }

      --websocket->queue_size;
      free(websocket->queue[websocket->queue_size].data);

      if (websocket->queue_size != 0) {
        websocket->queue = allocate(websocket->queue, -1, websocket->queue_size, sizeof(struct WebsocketQueueElement));
      }
    }
  }
}

static void check_response(struct Websocket *websocket, const char *response, char *key) {
  struct Split splitter = split(response, strlen(response), "\r\n");

  for (int i = 0; i < splitter.size; ++i) {
    if (strstr(splitter.data[i].data, ":") != NULL) {
      struct Split line_splitter = split(splitter.data[i].data, strlen(splitter.data[i].data), ":");
      char header_name[128];
      strtolower(header_name, line_splitter.data[0].data);

      if (streq(header_name, "sec-websocket-accept")) {
        const char *value = ltrim(line_splitter.data[1].data);

        const char websocket_guid[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
        const unsigned int key_length = strlen(key);
        const unsigned int websocket_guid_length = ((sizeof(websocket_guid) / sizeof(char)) - 1);
        const unsigned int final_length = (key_length + websocket_guid_length);

        char final[final_length + 1];
        memcpy(final, key, key_length);
        memcpy(final + key_length, websocket_guid, websocket_guid_length);
        final[final_length] = 0;

        unsigned char sha1_hash[SHA_DIGEST_LENGTH + 1];
        SHA1((unsigned char *) final, final_length, sha1_hash);
        char *result = base64_encode((char *) sha1_hash, SHA_DIGEST_LENGTH);

        free(key);

        if (!streq(result, value)) {
          free(result);
          split_free(line_splitter);
          split_free(splitter);
          close_websocket(websocket, -1, NULL);

          throw_network("switch_protocols(): unmatched websocket keys", false);
        }

        free(result);

        split_free(line_splitter);
        break;
      }

      split_free(line_splitter);
    }
  }

  split_free(splitter);
}

static void switch_protocols(struct Websocket *websocket) {
  unsigned char key_data[16];

  for (unsigned char i = 0; i < 16; ++i) {
    key_data[i] = ((rand() % 255) + 1);
  }

  websocket->key = base64_encode((char *) key_data, 16);

  char request_message[512];

  sprintf(request_message,
    "GET %s HTTP/1.1\r\n"
    "Host: %s:%d\r\n"
    "Upgrade: websocket\r\n"
    "Connection: Upgrade\r\n"
    "Sec-WebSocket-Key: %s\r\n"
    "Sec-WebSocket-Version: 13\r\n\r\n"
  , websocket->url.path, websocket->url.hostname, websocket->url.port, websocket->key);

  s_write(websocket->ssl, websocket->sockfd, request_message, strlen(request_message));
  websocket->connected = true;
}

void send_websocket_message(struct Websocket *websocket, const char *message) {
  unsigned char *data = NULL;
  const unsigned int message_length = strlen(message);
  unsigned int data_length = message_length;
  unsigned char masking_key[4];

  for (int i = 0; i < 4; ++i) {
    masking_key[i] = ((rand() % 255) + 1);
  }

  if (message_length > 65535) {
    data_length += 14;
    data = allocate(NULL, -1, data_length, sizeof(char));
    data[1] = 255;

    for (int i = 0; i < 8; ++i) {
      data[2 + i] = (message_length >> ((7 - i) * 8)) & 0xFF;
    }
  } else if (message_length > 125) {
    data_length += 8;
    data = allocate(NULL, -1, data_length, sizeof(char));
    data[1] = 254;
    data[2] = (message_length >> 8) & 0xFF;
    data[3] = message_length & 0xFF;
  } else {
    data_length += 6;
    data = allocate(NULL, -1, data_length, sizeof(char));
    data[1] = 128 + message_length;
  }

  data[0] = WEBSOCKET_FRAME_MAGIC;
  strncpy(((char *) data) + data_length - message_length - 4, (char *) masking_key, 4);

  for (int i = 0; i < message_length; ++i) {
    const char ch = (message[i] ^ masking_key[i % 4]);
    strncpy(((char *) data) + data_length - message_length + i, &ch, 1);
  }

  ++websocket->queue_size;
  websocket->queue = allocate(websocket->queue, -1, websocket->queue_size, sizeof(struct WebsocketQueueElement));
  websocket->queue[websocket->queue_size - 1].data = allocate(NULL, -1, data_length, sizeof(char));
  websocket->queue[websocket->queue_size - 1].size = data_length;
  memcpy(websocket->queue[websocket->queue_size - 1].data, data, data_length);

  free(data);
}


struct Websocket create_websocket(const char *url, const struct WebsocketMethods methods) {
  struct Websocket websocket = {0};
  websocket.methods = methods;

  initialize_websocket(&websocket, url);

  return websocket;
}

static void initialize_websocket(struct Websocket *websocket, const char *url) {
  websocket->url = parse_url(url);
  websocket->sockfd = socket(AF_INET, SOCK_STREAM, 0);
  websocket->ssl = NULL;

  if (websocket->sockfd == -1) {
    throw_network("socket()", !!websocket->ssl);
  }

  #if defined(__linux__)
    int keepalive = 1;

    if (setsockopt(websocket->sockfd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(int)) == -1) {
      throw_network("setsockopt()", !!websocket->ssl);
    }
  #endif

  if (streq(websocket->url.protocol, "wss")) {
    SSL_load_error_strings();
    SSL_library_init();

    SSL_CTX *ssl_ctx = SSL_CTX_new(SSLv23_client_method());

    // These settings are specified for shivers, if you're using this library independently, change them for what your needs
    SSL_CTX_set_cipher_list(ssl_ctx, "TLS_RSA_WITH_AES_256_CBC_SHA256");
    SSL_CTX_set_session_cache_mode(ssl_ctx, SSL_SESS_CACHE_OFF);
    SSL_CTX_set_options(ssl_ctx, SSL_OP_NO_TLSv1);

    websocket->ssl = SSL_new(ssl_ctx);
    SSL_set_fd(websocket->ssl, websocket->sockfd);
  }
}

void connect_websocket(struct Websocket *websocket) {
  struct hostent *host = resolve_hostname(websocket->url.hostname);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(websocket->url.port);
  memcpy(&addr.sin_addr, host->h_addr_list[0], (unsigned long) host->h_length);

  if (connect(websocket->sockfd, (const struct sockaddr *) &addr, sizeof(struct sockaddr_in)) == 0) {
    if (websocket->ssl) {
      SSL_connect(websocket->ssl);
    }

    do {
      FD_ZERO(&(websocket->readfds));
      FD_ZERO(&(websocket->writefds));
      FD_CLR(websocket->sockfd, &(websocket->readfds));
      FD_CLR(websocket->sockfd, &(websocket->writefds));
      FD_SET(websocket->sockfd, &(websocket->readfds));
      FD_SET(websocket->sockfd, &(websocket->writefds));

      websocket->tv.tv_sec = 3;
      websocket->tv.tv_usec = 0;

      #if defined(__linux__)
        int result = select(websocket->sockfd + 1, &(websocket->readfds), &(websocket->writefds), NULL, &(websocket->tv));
      #elif defined(_WIN32)
        int result = select(0, &(websocket->readfds), &(websocket->writefds), NULL, &(websocket->tv));
      #endif

      if (result != -1) {
        handle_events(websocket);
      }

      #if defined(__linux__)
        usleep(3000);
      #elif defined(_WIN32)
        Sleep(3);
      #endif
    } while (websocket->connected && !websocket->closed);
  } else {
    throw_network("connect()", !!websocket->ssl);
  }
}

void close_websocket(struct Websocket *websocket, const short code, const char *reason) {
  if (websocket->connected && !websocket->closed) {
    FD_ZERO(&(websocket->readfds));
    FD_ZERO(&(websocket->writefds));
    FD_CLR(websocket->sockfd, &(websocket->readfds));
    FD_CLR(websocket->sockfd, &(websocket->writefds));

    websocket->connected = false;
    websocket->closed = true;

    close_socket(websocket->sockfd, websocket->ssl);
  }


  for (unsigned int i = 0; i < websocket->queue_size; ++i) {
    free(websocket->queue[i].data);
  }

  free(websocket->queue);
  free_url(websocket->url);

  if (websocket->methods.onclose) {
    websocket->methods.onclose(code, reason);
  }
}
