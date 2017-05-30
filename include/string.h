#ifndef _STRING_H_
#define _STRING_H_

extern inline char *strcpy(char *dest, const char *src);
extern inline char *strncpy(char *dest, const char *src, int count);
extern inline char *strcat(char *dest, const char *src);
extern inline char *strncat(char *dest, const char *src, int count);
extern inline int strncmp(const char *cs, const char *ct, int count);
extern inline char *strchr(const char *s, char c);
extern inline int strcmp(const char *cs, const char *ct);
extern inline int strlen(const char *s);
#endif
