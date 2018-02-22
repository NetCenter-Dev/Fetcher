noinst_PROGRAMS = bin/receiver bin/transmitter tests/tests

test: tests/tests
	id
	cd tests/
	./tests

common=src/common/conf.c src/common/error.c

bin_receiver_SOURCES = src/receiver/main.c ${common}

bin_transmitter_SOURCES = src/transmitter/main.c ${common}

tests_tests_SOURCES = tests/main.c tests/configParser.c ${common}
