#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

#define BUFSIZE 1024
#define FILE_NAME_LEN 32

int main()
{
	int32_t cnt, rval;
	uint8_t *args;
	uint8_t buf[(BUFSIZE << 1) + 64];
	uint8_t currdir[FILE_NAME_LEN + 1];
	uint32_t dir_path[5][FILE_NAME_LEN + 1];
	ece391_fdputs(1, (uint8_t *)"Starting 391 Shell\n");
	ece391_strcpy(currdir, (const uint8_t *)".");
	while (1)
	{
		print((uint8_t *)"391OS: %s> ", currdir);
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
		args = parse_args(buf);
		if (0 == ece391_strcmp(buf, (uint8_t *)"mkdir"))
		{
			if (args == NULL)
			{
				ece391_fdputs(1, (uint8_t *)"do not get dir name\n");
			}
			ece391_fs(1, args, currdir);
			continue;
		}
		else if (0 == ece391_strcmp(buf, (uint8_t *)"mk"))
		{
			if (args == NULL)
			{
				ece391_fdputs(1, (uint8_t *)"do not get file name\n");
			}
			ece391_fs(2, args, currdir);
			continue;
		}
		else if (0 == ece391_strcmp(buf, (uint8_t *)"cd"))
		{
			if (args == NULL)
			{
				ece391_fdputs(1, (uint8_t *)"do not get path\n");
			}
			uint8_t *dir_start = args;
			while (*args != '\0')
			{
				if (*args == '/')
				{
					*args = '\0';
					args++;
					if (ece391_strncmp(dir_start, (uint8_t *)"..", ece391_strlen(dir_start)))
					{
						ece391_fs(5, currdir, currdir);
					}
					else if (ece391_strncmp(dir_start, (uint8_t *)".", ece391_strlen(dir_start)))
					{
						ece391_strcpy(currdir, (uint8_t *)'.');
					}
					else
					{
						int32_t ret;
						ret = ece391_fs(7, currdir, dir_start); // 7, currdir don't care,
						if (-2 == ret)
						{
							ece391_strcpy(currdir, dir_start); // have such dir
						}
						else if (-3 == ret)
						{
							ece391_fdputs(1, (uint8_t *)"no such dir\n");
							break;
						}
					}
				}
			}
			continue;
		}
		else if (0 == ece391_strcmp(buf, (uint8_t *)"ls"))
		{
			ece391_fs(5, buf, currdir);
			ece391_fdputs(1, buf);
			continue;
		}

		if (args != NULL)
		{
			args--;
			*args = ' ';
		}
		if ('\0' == buf[0])
			continue;
		rval = ece391_execute(buf);
		if (-1 == rval)
			ece391_fdputs(1, (uint8_t *)"no such command\n");
		else if (256 == rval)
			ece391_fdputs(1, (uint8_t *)"program terminated by exception\n");
		else if (0 != rval)
			ece391_fdputs(1, (uint8_t *)"program terminated abnormally\n");
	}
}
