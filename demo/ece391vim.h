#if !defined(ECE391VIM_H)
#define ECE391VIM_H

#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

int32_t file_disp(int32_t fd, int32_t type, uint8_t *buf);

#endif /* ECE391VIM_H */
