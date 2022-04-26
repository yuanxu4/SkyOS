#include "ece391buddy.h"

int main()
{
    int32_t cnt, rval;
    uint8_t *args;
    uint8_t buf[BUFSIZE];
    ece391_fdputs(1, (uint8_t *)"Starting buddy demo. \n");
    ece391_fdputs(1, (uint8_t *)"This is a toy buddy system with up to 64 pages. \n");

    while (1)
    {
        ece391_fdputs(1, (uint8_t *)"HELP: alloc <order>; free <addr index>; size <addr index>");
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
            return 0;
        if ('\0' == buf[0])
            continue;
        args = parse_args(buf);
        if (args == NULL)
        {
            ece391_fdputs(1, (uint8_t *)"no argument");
            continue;
        }
        if (0 == ece391_strcmp(buf, (uint8_t *)"alloc"))
        {
            ece391_fdputs(1, (uint8_t *)"allocate at: ");
            print_int(bd_alloc(ece391_atoi(args, 10)));
            ece391_fdputs(1, (uint8_t *)"\n");
        }
        else if (0 == ece391_strcmp(buf, (uint8_t *)"free"))
        {
            ece391_fdputs(1, (uint8_t *)"free (0 for succ):");
            print_int(bd_alloc(ece391_atoi(args, 10)));
            ece391_fdputs(1, (uint8_t *)"\n");
        }
        else if (0 == ece391_strcmp(buf, (uint8_t *)"size"))
        {
            ece391_fdputs(1, (uint8_t *)"size is: ");
            print_int(bd_get_size(ece391_atoi(args, 10)));
            ece391_fdputs(1, (uint8_t *)"\n");
        }
    }
}
