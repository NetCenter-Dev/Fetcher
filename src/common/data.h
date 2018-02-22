#ifndef DATA_H
#define DATA_H

#include <stdint.h>

typedef uint8_t type_t;
#define VOID 0
#define INT 1
#define DOUBLE 2
#define STRING 3

typedef uint8_t data_t;
#define NONE 0
#define MESSAGE 1
#define	DATA_VALUE 2
#define PROPERTY 3

typedef uint8_t class_t;
#define META 0
#define INFO 5
#define WARNING 10
#define ALARM 15
#define ERROR 20
#define EMERGENCY 25

#endif
