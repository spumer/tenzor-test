#ifndef _INCLUDE_TRIM_LIB_H_
#define _INCLUDE_TRIM_LIB_H_
extern "C" {
	/* Left-trim original string */
	char *ltrim(char *string, size_t len);
	
	/* Right-trim original string */
	char *rtrim(char *string, size_t len);
	
	/* Full-trim. Return copy. */
	char *trim(const char *string);
}
#endif