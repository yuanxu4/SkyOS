#if !defined(ECE391SUPPORT_H)
#define ECE391SUPPORT_H

#include <stdint.h>

#define NULL 0

extern uint32_t ece391_strlen(const uint8_t *s);
extern void ece391_strcpy(uint8_t *dst, const uint8_t *src);
extern int32_t ece391_fdputs(int32_t fd, const uint8_t *s);
extern void ece391_fdputc(int32_t fd, const uint8_t *s);
extern int32_t print(uint8_t *format, ...);
extern int32_t ece391_strcmp(const uint8_t *s1, const uint8_t *s2);
extern int32_t ece391_strncmp(const uint8_t *s1, const uint8_t *s2, uint32_t n);
extern uint8_t *ece391_itoa(uint32_t value, uint8_t *buf, int32_t radix);
extern uint32_t ece391_atoi(uint8_t *buf, int32_t radix);
extern uint8_t *ece391_strrev(uint8_t *s);
extern uint8_t *parse_args(uint8_t *command);
extern uint8_t *parse_dir(uint8_t *path);

#endif /* ECE391SUPPORT_H */
