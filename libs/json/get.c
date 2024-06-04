#include <string.h>
#include <stdbool.h>

#include <json.h>
#include <utils.h>

jsonresult_t json_get_val(jsonelement_t *element, const char *search) {
	struct Split splitter = split(search, strlen(search), ".");
	jsonresult_t result = {
		.exist = true
	};

	const unsigned int splitter_size = splitter.size;

	for (unsigned int ki = 0; ki < splitter_size; ++ki) {
		switch (element->type) {
			case JSON_ARRAY: {
				const unsigned int index = atoi_s(splitter.data[ki].data, splitter.data[ki].length);

				if (element->size > index) {
					element = ((jsonelement_t **) element->value)[index];
				} else {
					result.exist = false;
					ki = splitter_size;
				}

				break;
			}

			case JSON_OBJECT: {
				const unsigned int size = element->size;

				if (size == 0) {
					result.exist = false;
					ki = splitter_size;
				} else {
					const unsigned int last_index = (size - 1);

					for (unsigned int i = 0; i < size; ++i) {
						jsonelement_t *data = ((jsonelement_t **) element->value)[i];

						if (strsame(data->key, splitter.data[ki].data)) {
							element = data;
							break;
						} else if (i == last_index) {
							result.exist = false;
							ki = splitter.size;
							break;
						}
					}
				}

				break;
			}

			default:
				result.exist = false;
				ki = splitter_size;
				break;
		}
	}

	split_free(splitter);

	if (result.exist) {
		result.element = element;

		switch (element->type) {
			case JSON_NUMBER:
				result.value.number = *((double *) element->value);
				break;

			case JSON_STRING:
				result.value.string = element->value;
				break;

			case JSON_BOOLEAN:
				result.value.boolean = *((bool *) element->value);
				break;

			default:
				break;
		}
	} else {
		result = (jsonresult_t) {0};
	}

	return result;
}
