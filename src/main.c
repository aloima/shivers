#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <json.h>

int main(void) {
	JSONElement *data = json_parse("{\"merhaba\": 20, \"deneme\": 9082.921}");
	printf("%s\n", json_stringify(data));
	printf("%f\n", json_get_val(data, "deneme").number);

	if (getenv("TOKEN") == NULL) {
		fprintf(stderr, "startup: not provided token\n");
		return EXIT_FAILURE;
	} else {
		return EXIT_SUCCESS;
	}
}
