#pragma once

#include <bot.h>          // IWYU pragma: export
#include <database.h>     // IWYU pragma: export
#include <discord.h>      // IWYU pragma: export
#include <hash.h>         // IWYU pragma: export
#include <json.h>         // IWYU pragma: export
#include <network.h>      // IWYU pragma: export
#include <png.h>          // IWYU pragma: export
#include <utils.h>        // IWYU pragma: export

#include <ctype.h>        // IWYU pragma: export
#include <errno.h>        // IWYU pragma: export
#include <math.h>         // IWYU pragma: export
#include <signal.h>       // IWYU pragma: export
#include <stdarg.h>       // IWYU pragma: export
#include <stdio.h>        // IWYU pragma: export
#include <stdlib.h>       // IWYU pragma: export
#include <stdint.h>       // IWYU pragma: export
#include <string.h>       // IWYU pragma: export
#include <sys/stat.h>     // IWYU pragma: export
#include <sys/types.h>    // IWYU pragma: export
#include <time.h>         // IWYU pragma: export

#include <openssl/err.h>  // IWYU pragma: export
#include <openssl/sha.h>  // IWYU pragma: export
#include <openssl/ssl.h>  // IWYU pragma: export

#include <ft2build.h>     // IWYU pragma: export
#include FT_FREETYPE_H

#include <zconf.h>        // IWYU pragma: export
#include <zlib.h>         // IWYU pragma: export

#if defined(__WIN32__)
  #include <sys/timeb.h>  // IWYU pragma: export
  #include <winsock2.h>   // IWYU pragma: export
  #include <windows.h>    // IWYU pragma: export
#elif defined(__linux__) || defined(__APPLE__)
  #include <arpa/inet.h>  // IWYU pragma: export
  #include <fcntl.h>      // IWYU pragma: export
  #include <netdb.h>      // IWYU pragma: export
  #include <pthread.h>    // IWYU pragma: export
  #include <sys/select.h> // IWYU pragma: export
  #include <sys/socket.h> // IWYU pragma: export
  #include <sys/time.h>   // IWYU pragma: export
  #include <unistd.h>     // IWYU pragma: export
#endif
