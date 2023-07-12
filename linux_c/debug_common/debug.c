// for add debug print&append log file
//
#include <stdio.h>
#include <stdarg.h>

#define LC_DEBUG
#ifdef LC_DEBUG
#define lcp(...) printf("lc_debug"__VA_ARGS__)

void lclp(char *filename, char *format, ...) 
{
	FILE * keyFile;
	int i=0;
	keyFile = fopen(filename, "a");   // "w" will overwrite but "a" will append

	
	if (keyFile != NULL)
	{
		va_list args;
		va_start(args, format);
		char buffer[256];
		vsnprintf(buffer, sizeof(buffer), format, args);   // note here use the vxxx version like vprintf/vsnprintf
														// see https://stackoverflow.com/questions/5977326/call-printf-using-va-list
		fprintf(keyFile, "%s", buffer);
		va_end(args);
	}
	fclose(keyFile);
}
#else
#define lcp(...) do {} while(0)
void lclp(char *str) {}
#endif
