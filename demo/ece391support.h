#if !defined(ECE391SUPPORT_H)
#define ECE391SUPPORT_H

#include <stdint.h>

#define NULL 0
#define BUFSIZE 1024
#define FILE_NAME_LEN 32
#define SIZE_4MB 0x400000
#define KB_BUF_SIZE 128
#define NUM_CHAR_PER_LINE 80
#define NUM_128 128

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
extern void *ece391_memcpy(void *dest, const void *src, uint32_t n);
void *ece391_memset(void *s, int32_t c, uint32_t n);

#endif /* ECE391SUPPORT_H */
