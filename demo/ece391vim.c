#include "ece391vim.h"

int main()
{
    int32_t fd_root;
    int32_t fd;
    int32_t cnt, rval;
    uint8_t buf[BUFSIZE];
    uint8_t fname[FILE_NAME_LEN + 1];
    ece391_fdputs(1, (uint8_t *)"Starting 391 vim\n");

    if (0 != ece391_getargs(fname, FILE_NAME_LEN))
    {
        ece391_fdputs(1, (uint8_t *)"could not read argument\n");
        return 3;
    }

    if (-1 == (fd_root = ece391_open((uint8_t *)".")))
    {
        ece391_fdputs(1, (uint8_t *)"directory open failed\n");
        return 2;
    }

    rval = ece391_write(fd_root, fname, FILE_NAME_LEN);
    if (0 == rval) // old file
    {
        while (1)
        {
            ece391_fdputs(1, (uint8_t *)"existing file: use i to insert, use a to append, r to rewrite, exit to exit\n");
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
            else if (0 == ece391_strcmp(buf, (uint8_t *)"i"))
            {
                if (-1 == (fd = ece391_open((uint8_t *)fname)))
                {
                    ece391_fdputs(1, (uint8_t *)"file open failed\n");
                    return -1;
                }
                uint8_t *tmp_buf = ece391_alloc(SIZE_4MB);
                // int32_t cursor_position;
                int32_t file_size;
                // int32_t prev_idx, curr_idx;
                if ((file_size = file_disp(fd, 1, tmp_buf)) == -1)
                {
                    ece391_free(tmp_buf);
                    return -1;
                }
                while (1)
                {
                    if (-1 == (cnt = ece391_read(0, tmp_buf, BUFSIZE << 1)))
                    {
                        ece391_fdputs(1, (uint8_t *)"read from keyboard failed\n");
                        return 3;
                    }
                    if (cnt > 0 && '\n' == tmp_buf[cnt - 1])
                        tmp_buf[cnt] = '\0';
                    if (0 == ece391_strcmp(tmp_buf, (uint8_t *)":wq\n"))
                    {
                        break;
                    }
                    if (-1 == ece391_close(fd))
                    {
                        ece391_fdputs(1, (uint8_t *)"file close failed\n");
                        return -1;
                    }
                    // reopen to reset position in file
                    if (-1 == (fd = ece391_open((uint8_t *)fname)))
                    {
                        ece391_fdputs(1, (uint8_t *)"file open failed\n");
                        return -1;
                    }
                    if (-1 == ece391_write(fd, tmp_buf, cnt))
                        return 3;
                    // // todo
                    // // a function to update cursor and buf************************************
                    // int32_t i;
                    // prev_idx = 0;
                    // curr_idx = 0;
                    // cursor_position = file_size;
                    // while (buf[i] != '\n')
                    // {
                    //     if (buf[i] >= NUM_128 && buf[i + 1] >= NUM_128)
                    //     {
                    //         prev_idx = curr_idx;
                    //         curr_idx = i;
                    //         // put buf[prev_idx, curr_idx] starting at tmp_buf[cursor_position]*****************

                    //         file_size += (curr_idx - prev_idx);
                    //         cursor_position = (buf[i] - NUM_128) * NUM_CHAR_PER_LINE + (buf[i + 1] - NUM_128);
                    //         i += 2;
                    //         continue;
                    //     }
                    //     i++;
                    // }
                }

                // tmp_buf;
                // curr_position;
                // if (-1 == ece391_write(fd, tmp_buf, SIZE_4MB))
                //     return 3;
                ece391_free(tmp_buf);
                if (-1 == ece391_close(fd))
                {
                    ece391_fdputs(1, (uint8_t *)"file close failed\n");
                    return -1;
                }
            }
            else if (0 == ece391_strcmp(buf, (uint8_t *)"a"))
            {
                if (-1 == (fd = ece391_open((uint8_t *)fname)))
                {
                    ece391_fdputs(1, (uint8_t *)"file open failed\n");
                    return -1;
                }
                if (file_disp(fd, 0, NULL) == -1)
                {
                    return -1;
                }
                while (1)
                {
                    // ece391_fdputs(1, (uint8_t *)"please\n");
                    if (-1 == (cnt = ece391_read(0, buf, BUFSIZE - 1)))
                    {
                        ece391_fdputs(1, (uint8_t *)"read from keyboard failed\n");
                        return 3;
                    }
                    if (cnt > 0 && '\n' == buf[cnt - 1])
                        buf[cnt] = '\0';
                    if (0 == ece391_strcmp(buf, (uint8_t *)":wq\n"))
                    {
                        break;
                    }
                    if (-1 == ece391_write(fd, buf, cnt))
                        return 3;
                }
                if (-1 == ece391_close(fd))
                {
                    ece391_fdputs(1, (uint8_t *)"file close failed\n");
                    return -1;
                }
                return 0;
            }
            else if (0 == ece391_strcmp(buf, (uint8_t *)"r"))
            {
                if (-1 == (fd = ece391_open((uint8_t *)fname)))
                {
                    ece391_fdputs(1, (uint8_t *)"file open failed\n");
                    return -1;
                }
                while (1)
                {
                    if (-1 == (cnt = ece391_read(0, buf, BUFSIZE - 1)))
                    {
                        ece391_fdputs(1, (uint8_t *)"read from keyboard failed\n");
                        return 3;
                    }
                    if (cnt > 0 && '\n' == buf[cnt - 1])
                        buf[cnt] = '\0';
                    if (0 == ece391_strcmp(buf, (uint8_t *)":wq"))
                    {
                        break;
                    }
                    if (-1 == ece391_write(fd, buf, cnt))
                        return 3;
                }
                if (-1 == ece391_close(fd))
                {
                    ece391_fdputs(1, (uint8_t *)"file close failed\n");
                    return -1;
                }
                return 0;
            }
        }
    }
    else if (rval > 0)
    {
        // new file
        while (1)
        {
            ece391_fdputs(1, (uint8_t *)"new file: use a to append, exit to exit\n");
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
            else if (0 == ece391_strcmp(buf, (uint8_t *)"a"))
            {
                if (-1 == (fd = ece391_open((uint8_t *)fname)))
                {
                    ece391_fdputs(1, (uint8_t *)"file open failed\n");
                    return -1;
                }
                while (1)
                {
                    // ece391_fdputs(1, (uint8_t *)"please\n");
                    if (-1 == (cnt = ece391_read(0, buf, BUFSIZE - 1)))
                    {
                        ece391_fdputs(1, (uint8_t *)"read from keyboard failed\n");
                        return 3;
                    }
                    if (cnt > 0 && '\n' == buf[cnt - 1])
                        buf[cnt] = '\0';
                    if (0 == ece391_strcmp(buf, (uint8_t *)":wq\n"))
                    {
                        break;
                    }
                    if (-1 == ece391_write(fd, buf, cnt))
                        return 3;
                }
                if (-1 == ece391_close(fd))
                {
                    ece391_fdputs(1, (uint8_t *)"file close failed\n");
                    return -1;
                }
                return 0;
            }
        }
    }
    return -1;
}

int32_t file_disp(int32_t fd, int32_t type, uint8_t *buf)
{
    int32_t tmp;
    int32_t file_len = 0;
    uint8_t data[BUFSIZE + 1];
    do
    {
        tmp = ece391_read(fd, data, BUFSIZE);
        switch (tmp)
        {
        case 0:
            break;
        case -1:
            ece391_fdputs(1, (uint8_t *)"fail to read completely\n");
            return -1;
        default:
            file_len += tmp;
            if (-1 == ece391_write(1, data, tmp))
            {
                ece391_fdputs(1, (uint8_t *)"fail to display file\n");
                return -1;
            }
            if (type == 1)
            {
                ece391_memcpy(buf, data, BUFSIZE);
            }

            break;
        }
    } while (tmp);
    return file_len;
}