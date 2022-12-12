#include "Argument.h"
#include <stdio.h>

const char* GetArgText(va_list& args, const char* format)
{
	static char text[1024];
	vsprintf_s(text, sizeof(text), format, args);
	return text;
}