#ifndef _LIBC_STDIO_H
#define _LIBC_STDIO_H 1
 
#include <sys/cdefs.h>
 
#define EOF (-1)
 
#ifdef __cplusplus
extern "C" {
#endif
 
int puts(const char *);
 
#ifdef __cplusplus
}
#endif
 
#endif
