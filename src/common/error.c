#include "error.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

const char* error;

#define MAX_ERROR_LENGTH 1024
static char* errmsg = NULL;

void fail(const char* format, ...) {
	if (errmsg == NULL) {
		error = "Dynamic error messages not initialized.";
		return;
	}
	va_list arguments;

	va_start(arguments, format);

	if (vsnprintf(errmsg, MAX_ERROR_LENGTH, format, arguments) < 0)
		libfail();
	else
		error = errmsg;

	va_end(arguments);
}

void libfail() {
	error = strerror(errno);
}

int errorInit() {
	if (errmsg == NULL)
		errmsg = malloc(MAX_ERROR_LENGTH);
	if (errmsg == NULL) {
		libfail();
		return -1;
	}
	return 0;
}
