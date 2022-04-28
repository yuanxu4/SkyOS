#include "ece391slab.h"

int32_t main()
{
    memory_system_t mem_sys;
    int32_t cnt;
    uint8_t *args1;
    uint8_t *args2;
    uint8_t buf[BUFSIZE];
    uint32_t num1;
    uint32_t num2;
    ece391_fdputs(1, (uint8_t *)"Starting slab demo. \n");
    ece391_fdputs(1, (uint8_t *)"This is a toy memory system with budd and slab. \n");
    memory_init(&mem_sys);
    while (1)
    {
        ece391_fdputs(1, (uint8_t *)"HELP: alloc <size>; free <addr>; size <addr index>; init <order>; exit\n");
        ece391_fdputs(1, (uint8_t *)"NOTE: -1 means operation failed. \n");
        ece391_fdputs(1, (uint8_t *)"SLAB> ");
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
        args1 = parse_args(buf);
        if (args1 == NULL)
        {
            ece391_fdputs(1, (uint8_t *)"no argument\n");
            continue;
        }
        if (0 == ece391_strcmp(buf, (uint8_t *)"alloc"))
        {
            args2 = parse_args(args1);
            num1 = (uint32_t)ece391_atoi(args1, 10);
            // print((uint8_t *)"num1: %d\n", num1);
            for (num2 = (uint32_t)ece391_atoi(args2, 10); num2 > 0; num2++)
            {
                print((uint8_t *)"allocate at: %#x\n", k_alloc(&mem_sys, num1));
            }
        }
        else if (0 == ece391_strcmp(buf, (uint8_t *)"free"))
        {
            num1 = (uint32_t)ece391_atoi(args1, 16);
            print((uint8_t *)"num1: %#x\n", num1);
            print((uint8_t *)"free(0 for succ): %d\n", k_free(&mem_sys, (void *)num1));
        }
        // else if (0 == ece391_strcmp(buf, (uint8_t *)"size"))
        // {
        //     num1 = (uint32_t)ece391_atoi(args1, 16);
        //     // print((uint8_t *)"num1: %#x\n", num1);
        //     print((uint8_t *)"size is: %d pages\n", bd_get_size(&buddy_sys, (void *)num1));
        // }
        // else if (0 == ece391_strcmp(buf, (uint8_t *)"init"))
        // {
        //     num1 = (uint32_t)ece391_atoi(args1, 10);
        //     num1 = min(num1, MAX_ORDER_PAGE);
        //     ece391_free((void *)base_addr);
        //     init_buddy_sys(&buddy_sys, num1);
        //     bd_display(&buddy_sys);
        // }
    }
    return 0;
}
