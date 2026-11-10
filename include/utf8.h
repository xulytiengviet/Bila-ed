#ifndef CVED_UTF8_H
#define CVED_UTF8_H
#include <stddef.h>
#include <stdint.h>
typedef struct { char *p; size_t n,cap; } CvedStr;
void str_init(CvedStr *s);
void str_free(CvedStr *s);
int str_add(CvedStr *s, const char *p, size_t n);
int str_putc(CvedStr *s, char c);
char *str_take(CvedStr *s);
int utf8_valid(const char *s, size_t n);
size_t utf8_char(const char *s, size_t n, uint32_t *cp);
size_t utf8_emit(uint32_t cp, char out[4]);
uint32_t utf8_upper(uint32_t cp);
uint32_t utf8_lower(uint32_t cp);
int utf8_isletter(uint32_t cp);
char *utf8_case(const char *s, int casing); /* 0 lowercase; 1 Title; 2 UPPER */
int utf8_equal(const char *a, const char *b);
#endif
