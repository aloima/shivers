const unsigned int get_all_intents() {
	unsigned int result = 0;

	for (int i = 0; i <= 21; ++i) {
		result |= (1 << i);
	}

	return (const unsigned int) result;
}
