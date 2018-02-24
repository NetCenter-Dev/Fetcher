noinst_PROGRAMS = bin/receiver bin/transmitter tests/tests

AM_CFLAGS =

if OSX
	AM_CFLAGS += ""
endif
if LINUX
    AM_CFLAGS += "-lrt"
endif

test: tests/tests
	id
	cd tests/
	./tests

common=src/common/conf.c src/common/error.c src/common/packet.c src/common/timer.c

bin_receiver_SOURCES = src/receiver/main.c ${common}

bin_transmitter_SOURCES = src/transmitter/main.c ${common}

tests_tests_SOURCES = tests/main.c tests/configParser.c tests/timer.c ${common}
