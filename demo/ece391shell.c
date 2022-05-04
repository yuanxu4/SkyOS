#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

int32_t parse_path(uint8_t* args, uint8_t*currdir,int32_t type );

int main()
{
	int32_t cnt, rval;
	uint8_t *args;
	uint8_t buf[KB_BUF_SIZE+1];
	uint8_t buf1[(BUFSIZE << 1) + 64];
	uint8_t currdir[FILE_NAME_LEN + 1];
	uint8_t tmpdir[FILE_NAME_LEN + 1];
	uint32_t dir_path[5][FILE_NAME_LEN + 1];
	ece391_fdputs(1, (uint8_t *)"Starting 391 Shell\n");
	ece391_strcpy(currdir, (const uint8_t *)".");
	while (1)
	{
		print((uint8_t *)"391OS: ");
		ece391_fdputs(1, (uint8_t *)currdir);
		print((uint8_t *)"> ");
		if (-1 == (cnt = ece391_read(0, buf, KB_BUF_SIZE)))
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
			ece391_memcpy(tmpdir, currdir,FILE_NAME_LEN);
			parse_path(args, tmpdir, 1);
			// ece391_fs(2, args, tmpdir);
			continue;
		}
		else if (0 == ece391_strcmp(buf, (uint8_t *)"mk"))
		{
			ece391_memcpy(tmpdir, currdir,FILE_NAME_LEN);
			parse_path(args, tmpdir,2);
			// ece391_fs(1, args, tmpdir);
			continue;
		}
		if (0 == ece391_strcmp(buf, (uint8_t *)"rmdir"))
		{
			ece391_memcpy(tmpdir, currdir,FILE_NAME_LEN);
			parse_path(args, tmpdir, 3);
			// ece391_fs(2, args, tmpdir);
			continue;
		}
		else if (0 == ece391_strcmp(buf, (uint8_t *)"rm"))
		{
			ece391_memcpy(tmpdir, currdir,FILE_NAME_LEN);
			parse_path(args, tmpdir,4);
			// ece391_fs(1, args, tmpdir);
			continue;
		}
		else if (0 == ece391_strcmp(buf, (uint8_t *)"cd"))
		{
			parse_path(args, currdir,0);
			continue;
		}
		else if (0 == ece391_strcmp(buf, (uint8_t *)"ls"))
		{

			ece391_memset(buf1, 0, (BUFSIZE << 1) + 64);
			ece391_fs(5, buf1, currdir);
			ece391_fdputs(1, buf1);
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


int32_t parse_path(uint8_t* args, uint8_t*currdir,int32_t type )
{
			if (args == NULL)
			{
				ece391_fdputs(1, (uint8_t *)"do not get path\n");
				return 0;
			}
			uint8_t *dir_start = args;
			int32_t ret;
			// ece391_fdputs(1, (uint8_t *)dir_start);
			while (*args != '\0')
			{
				if (*args == '/')
				{
					*args = '\0';
					args++;
					if (!ece391_strncmp(dir_start, (uint8_t *)"..", ece391_strlen(dir_start)))
					{
						ece391_fs(6, currdir, currdir);
						// print((uint8_t*)"1");
						// ece391_fdputs(1, (uint8_t *)currdir);
					}
					else if (!ece391_strncmp(dir_start, (uint8_t *)".", ece391_strlen(dir_start)))
					{
						ece391_strcpy(currdir, (uint8_t *)'.');
						// print((uint8_t*)"2");
						// ece391_fdputs(1, (uint8_t *)currdir);
					}
					else
					{
						ret = ece391_fs(7,  dir_start, currdir); 
						// print("%d\n",ret);
						if (0 == ret)
						{
							ece391_strcpy(currdir, dir_start); // have such dir
							// 							print((uint8_t*)"3");
							// ece391_fdputs(1, (uint8_t *)currdir);
						}
						else 
						{
							if (type==0)
							{
							ece391_fdputs(1, (uint8_t *)"no such dir\n");
							// break;
							}
							

						}
					}
					dir_start=args;
					*(args-1) = '/';
				}
				else{args++;}
			}

					if (!ece391_strncmp(dir_start, (uint8_t *)"..", ece391_strlen(dir_start)))
					{
						ece391_fs(6, currdir, currdir);
						// 						print((uint8_t*)"1");
						// ece391_fdputs(1, (uint8_t *)currdir);
					}
					else if (!ece391_strncmp(dir_start, (uint8_t *)".", ece391_strlen(dir_start)))
					{
						ece391_strcpy(currdir, (uint8_t *)'.');
						// 						print((uint8_t*)"2");
						// ece391_fdputs(1, (uint8_t *)currdir);
					}
					else
					{
						ret = ece391_fs(7,  dir_start, currdir); 
						// print("%d\n",ret);
						if (0 == ret)
						{
							if (type==3)
							{
								ece391_fs(type, dir_start, currdir);
							}
							else if (type==0)
							{
								ece391_strcpy(currdir, dir_start); // have such dir
							}	
						}
						else if (1 == ret)
						{
							if (type==4)
							{
								ece391_fs(type, dir_start, currdir);
							}
							else if (type==3)
							{
								ece391_fdputs(1, (uint8_t *)"this is a file not a dir\n");
							}	
							else if (type==0)
							{
								ece391_fdputs(1, (uint8_t *)"no such dir\n");
							}	
						}
						else if (-1 == ret)
						{
							if (type==0)
							{
							ece391_fdputs(1, (uint8_t *)"no such dir\n");
							// break;
							}
							else if (type==1 || type ==2){
								ece391_fs(type, dir_start, currdir);
							}
						}
					}

			return 0;
}
