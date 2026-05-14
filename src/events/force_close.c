#include <shivers.h>

#if defined(_WIN32)
  #include <winsock2.h>
  #include <windows.h>
#endif

void on_force_close(struct Shivers *shivers) {
  free_hashmap(shivers->cooldowns);
  puts("\nFree'd cooldowns.");

  free_hashmap(shivers->commands);
  puts("Free'd commands.");

  database_save();
  database_destroy();
  puts("Database is saved and destroyed.");

  free_fonts();
  puts("Free'd fonts.");

  #if defined(_WIN32)
    WSACleanup();
    puts("Cleaned up Windows API.");
  #endif

  puts("Force quitting...");
}
