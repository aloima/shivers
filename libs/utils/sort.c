#include <utils.h>

void sort(struct Sort *data, const unsigned int size) {
	const unsigned int fbound = (size - 1);

	for (unsigned int a = 0; a < fbound; ++a) {
		const unsigned int sbound = (fbound - a);

		for (unsigned int i = 0; i < sbound; ++i) {
			if (data[i + 1].number > data[i].number) {
				struct Sort temp = data[i];
				data[i] = data[i + 1];
				data[i + 1] = temp;
			}
		}
	}
}
