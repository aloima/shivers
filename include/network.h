#include <stdbool.h>

#if defined(_WIN32)
  #include <winsock2.h>
  #include <windows.h>
#elif defined(__linux__)
  #include <sys/select.h>
  #include <sys/time.h>
#endif

#include <openssl/ssl.h>

#include <utils.h>

#ifndef NETWORK_H_
  #define NETWORK_H_

  #define HTTP_BUFFER_SIZE 16384

  struct URL {
    char *protocol, *hostname, *path;
    unsigned short port;
  };

  struct Header {
    char *name, *value;
  };

  struct FormDataField {
    char *name, *data, *filename;
    struct Header *headers;
    unsigned long data_size;
    unsigned int header_size;
  };

  struct FormData {
    struct FormDataField *fields;
    unsigned int field_size;
    char *boundary;
  };

  struct RequestBody {
    union {
      char *data;
      struct FormData formdata;
    } payload;

    bool is_formdata;
  };

  struct RequestConfig {
    char *url, *method;
    struct Header *headers;
    unsigned int header_size;
    struct RequestBody body;
  };

  struct Response {
    bool success;

    struct {
      short code;
      char *message;
    } status;

    unsigned char *data;
    unsigned long data_size;
    struct Header *headers;
    unsigned int header_size;
  };

  struct Response request(struct RequestConfig config);
  void response_free(struct Response response);

  void throw_network(const char *value, bool tls);

  struct URL parse_url(const char *data);
  void free_url(struct URL url);
  char *percent_encode(const char *data);

  #if defined(_WIN32)
    unsigned long s_read(SSL *ssl, SOCKET sockfd, char *buffer, unsigned long size);
    unsigned long s_write(SSL *ssl, SOCKET sockfd, char *buffer, unsigned long size);

    void close_socket(SOCKET sockfd, SSL *ssl);
  #elif defined(__linux__)
    unsigned long s_read(SSL *ssl, int sockfd, char *buffer, unsigned long size);
    unsigned long s_write(SSL *ssl, int sockfd, char *buffer, unsigned long size);

    void close_socket(int sockfd, SSL *ssl);
  #endif

  unsigned long combine_bytes(unsigned char *bytes, unsigned long byte_count);
  struct Header get_header(struct Header *headers, const unsigned int header_size, const char *name);
  struct hostent *resolve_hostname(char *hostname);
  void add_field_to_formdata(struct FormData *formdata, const char *name, const char *data, const unsigned long long data_size, const char *filename);
  void add_header_to_formdata_field(struct FormData *formdata, const char *field_name, const char *header_name, const char *header_value);
  void free_formdata(struct FormData formdata);

  // Websocket Headers
  #define WEBSOCKET_FRAME_MAGIC 0x81;

  struct WebsocketFrame {
    bool fin, rsv[3], mask;
    unsigned char opcode;
    char *payload;
    unsigned long payload_length;
  };

  struct WebsocketQueueElement {
    char *data;
    unsigned long size;
  };

  struct WebsocketMethods {
    void (*onstart)(void);
    void (*onmessage)(const struct WebsocketFrame frame);
    void (*onclose)(const short code, const char *reason);
  };

  struct Websocket {
    #if defined(_WIN32)
      SOCKET sockfd;
    #elif defined(__linux__)
      int sockfd;
    #endif

    fd_set readfds, writefds;
    struct timeval tv;
    SSL *ssl;
    struct URL url;
    struct WebsocketMethods methods;
    bool connected, closed;
    char *key;

    struct WebsocketQueueElement *queue;
    unsigned int queue_size;
  };

  struct Websocket create_websocket(const char *url, const struct WebsocketMethods methods);
  void connect_websocket(struct Websocket *websocket);
  void close_websocket(struct Websocket *websocket, const short code, const char *reason);
  void send_websocket_message(struct Websocket *websocket, const char *message);
#endif
