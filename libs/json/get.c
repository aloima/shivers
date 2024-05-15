#include <string.h>
#include <stdbool.h>

#include <json.h>
#include <utils.h>

jsonresult_t json_get_val(jsonelement_t *element, const char *search) {
	struct Split splitter = split(search, strlen(search), ".");
	jsonelement_t *value = element;
	jsonresult_t result = {
		.exist = true
	};

	const unsigned int splitter_size = splitter.size;

	for (unsigned int ki = 0; ki < splitter_size; ++ki) {
		if (value->type == JSON_ARRAY) {
			const unsigned int index = atoi_s(splitter.data[ki].data, splitter.data[ki].length);

			if (value->size > index) {
				value = ((jsonelement_t **) value->value)[index];
			} else {
				result.exist = false;
				break;
			}
		} else if (value->type == JSON_OBJECT) {
			const unsigned int size = value->size;

			if (size == 0) {
				result.exist = false;
				break;
			} else {
				const unsigned int last_index = (size - 1);

				for (unsigned int i = 0; i < size; ++i) {
					jsonelement_t *data = ((jsonelement_t **) value->value)[i];

					if (strsame(data->key, splitter.data[ki].data)) {
						value = data;
						i = size;
					} else if (i == last_index) {
						result.exist = false;
						ki = splitter.size;
					}
				}
			}
		} else {
			result.exist = false;
			break;
		}
	}

	split_free(splitter);

	if (result.exist) {
		result.type = value->type;
		result.element = value;

		switch (value->type) {
			case JSON_NUMBER:
				result.value.number = *((double *) value->value);
				break;

			case JSON_STRING:
				result.value.string = value->value;
				break;

			case JSON_BOOLEAN:
				result.value.boolean = *((bool *) value->value);
				break;

			case JSON_OBJECT:
				result.value.object = value;
				break;

			case JSON_ARRAY:
				result.value.array = value;
				break;
		}
	} else {
		result = (jsonresult_t) {0};
	}

	return result;
}
