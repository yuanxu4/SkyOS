#include "ece391buddy.h"

int main()
{
    buddy_system_t buddy_sys;
    int32_t cnt;
    uint8_t *args;
    uint8_t buf[BUFSIZE];
    uint32_t num;
    ece391_fdputs(1, (uint8_t *)"Starting buddy demo. \n");
    ece391_fdputs(1, (uint8_t *)"This is a toy buddy system with up to 64 pages (order 6). \n");
    init_buddy_sys(&buddy_sys, 4);
    bd_display(&buddy_sys);
    while (1)
    {
        ece391_fdputs(1, (uint8_t *)"HELP: alloc <order>; free <addr>; size <addr>; init <max order>; exit\n");
        ece391_fdputs(1, (uint8_t *)"NOTE: -1 means operation failed. \n");
        ece391_fdputs(1, (uint8_t *)"BUDDY> ");
        if (-1 == (cnt = ece391_read(0, buf, BUFSIZE - 1)))
        {
            ece391_fdputs(1, (uint8_t *)"read from keyboard failed\n");
            return 3;
        }
        if (cnt > 0 && '\n' == buf[cnt - 1])
            cnt--;
        buf[cnt] = '\0';
        if (0 == ece391_strcmp(buf, (uint8_t *)"exit"))
        {
            return 0;
        }
        if ('\0' == buf[0])
            continue;
        args = parse_args(buf);
        if (args == NULL)
        {
            ece391_fdputs(1, (uint8_t *)"no argument\n");
            continue;
        }
        if (0 == ece391_strcmp(buf, (uint8_t *)"alloc"))
        {
            num = (uint32_t)ece391_atoi(args, 10);
            // print((uint8_t *)"num: %d\n", num);
            print((uint8_t *)"allocate at: %#x\n", bd_alloc(&buddy_sys, num));
            bd_display(&buddy_sys);
        }
        else if (0 == ece391_strcmp(buf, (uint8_t *)"free"))
        {
            num = (uint32_t)ece391_atoi(args, 16);
            print((uint8_t *)"num: %#x\n", num);
            print((uint8_t *)"free(0 for succ): %d\n", bd_free(&buddy_sys, (void *)num));
            bd_display(&buddy_sys);
        }
        else if (0 == ece391_strcmp(buf, (uint8_t *)"size"))
        {
            num = (uint32_t)ece391_atoi(args, 16);
            // print((uint8_t *)"num: %#x\n", num);
            print((uint8_t *)"size is: %d pages\n", bd_get_size(&buddy_sys, (void *)num));
        }
        else if (0 == ece391_strcmp(buf, (uint8_t *)"init"))
        {
            num = (uint32_t)ece391_atoi(args, 10);
            num = min(num, MAX_ORDER_PAGE);
            ece391_free((void *)base_addr);
            init_buddy_sys(&buddy_sys, num);
            bd_display(&buddy_sys);
        }
    }
}
