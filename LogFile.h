#pragma once

#include <stdio.h>
#include <stdarg.h>

extern FILE *dbgFile;

void LOG(char *message, ...)
{
	char text[512];
	va_list ap;
	va_start(ap, message);
	vsnprintf(text, 512, message, ap);
	va_end(ap);
	fputs(text, dbgFile);
}

void FileLOG(FILE *f, char *message, ...)
{
	char text[512];
	va_list ap;
	va_start(ap, message);
	vsnprintf(text, 512, message, ap);
	va_end(ap);
	fputs(text, f);
}