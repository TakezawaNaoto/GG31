#ifndef __ARGUMENT_H__
#define __ARGUMENT_H__

#include <stdarg.h>

#define VA_TEXT(ptr, format) \
const char* ptr; \
do { \
	va_list args; \
	va_start(args, format); \
	ptr = GetArgText(args, format); \
	va_end(args); \
} while(0)

const char* GetArgText(va_list& args, const char* format);

#endif // __ARGUMENT_H__