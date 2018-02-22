#ifndef ERROR_H
#define ERROR_H

extern const char* error;

void fail(const char*, ...);
void libfail(void);

int errorInit(void);

#endif
