/*
*	Simple Trim Library
*/

#include <stddef.h> // for size_t
#include <string.h> // for strdup
#include <stdlib.h>

char *ltrim(char *string, size_t len) {
    char *newstr = string;
	for( char ch = *string; 
				len && (
				 (ch == ' ')   // space
                 || (ch == '\n')                // LF
                 || (ch == '\t')                // TAB
                 || (ch == '\r' && *(string+1) == '\n' )        // CRLF
				)
                 ; --len ) {
                ch = *(++string);
        }
	memmove(newstr, string, len - (newstr-string));
	return newstr;
}

char *rtrim(char *string, size_t len) {
	char ch;
	do {
		ch = string[--len];
		if((ch == ' ')	// space
            || (ch == '\n')                // LF
            || (ch == '\t')                // TAB
            || (ch == '\r' && *(string+1) == '\n' )        // CRLF
		  ) {
			string[len] = '\0';
		}
		else break;
	} while( len );
	return string;
}

char *trim(const char *string) {
	size_t len = strlen(string);
	if( !len ) return NULL;
	
	char *t_string = strdup(string);
	t_string = ltrim(t_string, len);
	t_string = rtrim(t_string, len);
	
	len = strlen(t_string);
	if( !len ) { free(t_string); return NULL; }
	
	return t_string;
}