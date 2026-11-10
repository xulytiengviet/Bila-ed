#ifndef CVNSS_ENGINE_H
#define CVNSS_ENGINE_H
#include <stddef.h>
/* Both functions return malloc'd output or NULL on allocation/invalid input.
   Decode passes unknown ASCII words through verbatim (literal fallback). */
char *cvnss_decode(const char *raw);
char *cvnss_encode(const char *unicode); /* NULL means no provably exact raw form */
int cvnss_valid_raw(const char *raw);
size_t cvnss_rule_count(void);
int base27_encode(const char *raw, char **out); /* three trits per symbol, 0..2 */
int base27_decode(const char *trits, char **out);
/* Internal helpers shared between source modules. */
const char *cvnss_lookup_decode(const char *code);
const char *cvnss_lookup_encode(const char *rime);
int cvnss_is_onset(const char *s);
const char *cvnss_restore_onset(const char *code, const char *rime);
int cvnss_is_vowel_ie(const char *rime);
int cvnss_is_vowel_u(const char *rime);
#endif
