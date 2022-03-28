/*
 * ECE 391 SP 2022
 * History:
 * add paging test          - Mar 20, keyi
 *
 */

#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "file_system.h"
#include "keyboard.h"
#include "rtc.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER \
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result) \
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

#define TEST_PRINT_MODE 1
#if TEST_PRINT_MODE
#define TEST_PRINT(fmt, ...)        \
	do                              \
	{                               \
		printf("[INFO] ");        \
		printf(fmt, ##__VA_ARGS__); \
	} while (0)

#else
#define TEST_PRINT(fmt, ...) \
	do                       \
	{                        \
	} while (0)

#endif

static inline void assertion_failure()
{
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}

int8_t file_buf[FILE_NAME_LENGTH]; // buffer, avoid out of range

/* Checkpoint 1 tests */

/* IDT Test - Example
 *
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test()
{
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i)
	{
		if ((idt[i].offset_15_00 == NULL) &&
			(idt[i].offset_31_16 == NULL))
		{
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

// add more tests here
/* IDT_div0_test
 *
 * Inputs: None
 * Outputs: the exception 0 or fail
 * Side Effects: None
 * Coverage: the exception 0 ---- div 0
 * Files: x86_desc.h/S
 */
int idt_div0_test()
{
	TEST_HEADER;

	unsigned long div = 10; // random chosed num
	unsigned long zero = 0; // 0
	unsigned result;
	result = div / zero; // implement div 0

	return FAIL;
}

/*
 *
 * check for the dereference
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: paging, idt
 * Files: x86_desc.h/S
 */
int idt_dereference_test()
{
	TEST_HEADER;
	int result = PASS;
	// // test1: dereference in 0-4MB outside video memory
	// uint32_t *tmp;
	// tmp = (uint32_t *)(0xB8000 - 8);
	// // TEST_PRINT("tmp is %x", tmp);
	// if (*tmp)
	// {
	// 	/* should fault */
	// 	result = FAIL;
	// }

	// test2: dereference partly <8MB, partly >8MB
	long long *tmp2;
	tmp2 = (long long *)(0x800000 - 4);
	TEST_PRINT("tmp2 is %x\n", tmp2);
	if (*tmp2)
	{
		/* should fault */
		result = FAIL;
	}

	return result;
}

/* pic_garbage_test - Example
 *
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: enable_irq
 * Files: x86_desc.h/S
 */
int pic_garbage_test()
{
	TEST_HEADER;
	int result = PASS;

	enable_irq(17);
	enable_irq(-1);
	return result;
}

/*
 *
 * keyboard test
 * Inputs: None
 * Outputs: PASS
 * Side Effects: None
 * Coverage: keyboard test
 * Files: x86_desc.h/S
 */
int keyboard_test()
{
	clear();
	return PASS;
}

/* paging Test
 *
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: enable paging, paging definition
 * Files: x86_desc.h/S, boot.S
 */
int paging_test()
{
	TEST_HEADER;
	// clear();
	int i;
	int result = PASS;
	uint32_t cr_0, cr_3, cr_4;
	uint32_t *tmp;
	// get regs
	asm volatile("movl %%cr0, %0\n\t"
				 "movl %%cr3, %1\n\t"
				 "movl %%cr4, %2"
				 : "=r"(cr_0), "=r"(cr_3), "=r"(cr_4)
				 :
				 : "memory", "cc");
	// check cr0 pg,pe set
	if ((cr_0 & 0x80000001) == 0)
	{
		TEST_PRINT("Error!!!  CR0 is %x\n", cr_0);
		result = FAIL;
	}
	// if equal, means cr3 base addr,PCD(0),PWT(0) all right, ignored as 0
	if ((PD_t *)cr_3 != &page_directory)
	{
		TEST_PRINT("Error!!!  CR3 is %x, and page directory addr is %x\n", cr_3, &page_directory);
		result = FAIL;
	}
	// check cr4 pse set
	if ((cr_4 & 0x00000010) == 0)
	{
		TEST_PRINT("Error!!!  CR4 is %x\n", cr_4);
		result = FAIL;
	}
	// check pde
	// entry 0, check ptba, high20bits
	tmp = (uint32_t *)(page_directory.pde[0] & 0xFFFFF000);
	if ((PT_t *)tmp != &page_table)
	{
		TEST_PRINT("Error!!!  PT base addr in PD is %x, but actually %x", tmp, &page_table);
		result = FAIL;
	}
	// entry 0, check others, like ps, p
	if ((page_directory.pde[0] & 0x00000003) != 0x00000003)
	{
		TEST_PRINT("PT is %x, set wrongly", page_directory.pde[0]);
		result = FAIL;
	}
	// entry 1, pba, high10bits
	tmp = (void *)(page_directory.pde[1] & 0xFFC00000);
	if (tmp != (void *)0x00400000)
	{
		TEST_PRINT("Error!!!  Kernel Page base addr in PD is %x, but actually %x", tmp, 0x00400000);
		result = FAIL;
	}
	// entry 1, check others, like ps, p
	if ((page_directory.pde[1] & 0x00000083) != 0x00000083)
	{
		TEST_PRINT("Error!!!  Kernel Page is %x, set wrongly\n", page_directory.pde[0]);
		result = FAIL;
	}
	// check video memory
	if ((page_table.pte[VIDEO_MEM_INDEX] & 0x000B8003) != 0x000B8003)
	{
		TEST_PRINT("Error!!!  PTE for video memory is %x", page_table.pte[VIDEO_MEM_INDEX]);
		result = FAIL;
	}
	// check other present
	for (i = 0; i < VIDEO_MEM_INDEX; i++)
	{
		if ((page_table.pte[i] & 0x00000001))
		{
			TEST_PRINT("Error!!!  PTE %d is present", i);
			result = FAIL;
		}
	}
	for (i = VIDEO_MEM_INDEX + 1; i < PT_SIZE; i++)
	{
		if ((page_table.pte[i] & 0x00000001))
		{
			TEST_PRINT("Error!!!  PTE %d is present", i);
			result = FAIL;
		}
	}
	// dereference test
	// dereference in video memory
	tmp = (uint32_t *)(0xB8000 + 8);
	// TEST_PRINT("tem is %x\n", tmp);
	if ((*tmp))
	{
		/* fine */
	}
	// dereference in kernel page 4-8MB
	tmp = (uint32_t *)(0x400000 + 8);
	// TEST_PRINT("tem is %x\n", tmp);
	if (*tmp)
	{
		/* fine */
	}
	return result;
}

/* Checkpoint 2 tests */


int file_sys_test()
{
	clear();
	TEST_HEADER;
	int result = PASS;
	int32_t fd;					  // file desc

	// 8 files, 1 root, 2 small files, 2 executables, 2 largefiles, 1 invaild file
	char *flie_list[8] = {".", "frame0.txt", "frame1.txt", "grep", "ls", "fish", "verylargetextwithverylongname.tx", "wqevfwrtvwev"};
	int32_t tmp;
	dentry_t tmp_dentry;
	int i;

	// open root
	TEST_PRINT("try to open the root directory\n\n");
	fd = file_sys_open((const uint8_t *)flie_list[0]);
	if (fd != 2)
	{
		TEST_PRINT("fail in test open root, fd is %d\n", fd);
		close_opening();
		return FAIL;
	}
	// read file names in root
	TEST_PRINT("read the root directory and display all files\n\n");
	// get_file_name();
	for (i = 0; i < get_file_num(); i++)
	{
		tmp = file_sys_read(fd, file_buf, FILE_NAME_LENGTH);
		if (tmp > 0)
		{
			// write -1 fail
			if (-1==file_sys_write(STDOUT_FD, file_buf, tmp))
			{
				TEST_PRINT("fail to write in stdout\n");
				close_opening();
				return FAIL;
			}
			// check file type and size
			// -1 fail, 0 succ
			if (read_dentry_by_name((const uint8_t *)file_buf, &tmp_dentry))
			{
				TEST_PRINT("fail to find file name\n");
				close_opening();
				return FAIL;
			}
			align_space(FILE_NAME_LENGTH+4);
			if (tmp_dentry.file_type ==2)
			{
				printf("file type: 2, size(B): %d\n", get_file_size(tmp_dentry.inode_num));
			}
			else
			{
				printf("file type: %d, size(B): 0\n", tmp_dentry.file_type);
			}
		}
		else
		{
			TEST_PRINT("fail to read file name at %d\n", i);
		}
	}
continue_test();
TEST_PRINT("continue to read, should print \"reach to the end\"", i);
	// should return 0 and print reach to the end\n
	tmp = file_sys_read(fd, file_buf, FILE_NAME_LENGTH);
	if (tmp)
	{
		TEST_PRINT("fail in test reading at end., read returns %d\n", tmp);
		close_opening();
		return FAIL;
	}
continue_test();
	// try to test stdin and stdout
	TEST_PRINT("try to write in stdin, read in stdout, close stdout, all should fail\n");
	if (!file_sys_write(STDIN_FD, file_buf, tmp))
	{
		TEST_PRINT("wrongly write in stdin\n");
		close_opening();
		return FAIL;
	}
	if (!file_sys_read(STDOUT_FD, file_buf, FILE_NAME_LENGTH))
	{
		TEST_PRINT("wrongly read in stdout\n");
		close_opening();
		return FAIL;
	}
	if (!file_sys_close(STDOUT_FD))
	{
		TEST_PRINT("wrongly close stdout\n");
		close_opening();
		return FAIL;
	}
	printf("\n");
	TEST_PRINT("Cooooool!!!! pass all tests about root, so close the root\n");
	close_opening();
	TEST_PRINT("then try to open and read 6 vaild regular files without closing anyone\n");

	// open 6 vaild regular files, never close until fail
	for (i = 1; i < 7; i++)
	{
continue_test();
		clear();
		TEST_PRINT("try to open file %s\n", flie_list[i]);
		fd = file_sys_open((const uint8_t *)flie_list[i]);
		if (fd < 2)
		{
			TEST_PRINT("fail to open %s\n", flie_list[i]);
			// should close other opening files for safty
			close_opening();
			return FAIL;
		}
		if (read_dentry_by_name((const uint8_t *)flie_list[i], &tmp_dentry))
		{
			TEST_PRINT("fail to find file, %s\n", file_buf);
			close_opening();
			return FAIL;
		}
		if (tmp_dentry.file_type != 2)
		{
			TEST_PRINT("get wrong file type\n");
			close_opening();
			return FAIL;
		}
		TEST_PRINT("the size of file is %d. below is the content\n", get_file_size(tmp_dentry.inode_num));

		// read file 32B one time
		do
		{
			tmp = file_sys_read(fd, file_buf, FILE_NAME_LENGTH);
			switch (tmp)
			{
			case 0:
				break;
			case -1:
				TEST_PRINT("fail to read completely\n");
				close_opening();
				return FAIL;
			default:
				if (-1==file_sys_write(STDOUT_FD, file_buf, tmp))
				{
					TEST_PRINT("fail to display file, %d\n", tmp);
					close_opening();
					return FAIL;
				}
				break;
			}
		} while (tmp);
		TEST_PRINT("have read %d th file(%s) with size %d B\n", i, flie_list[i],get_file_size(tmp_dentry.inode_num));
	}
continue_test();
	// here, 8 files opening
	TEST_PRINT("now we should reach the max number of opening file\n");
	printf("Check: the number of opening files is %d\n", get_num_opening());
	if (get_num_opening() != 8)
	{
		TEST_PRINT("fail in test opening files\n");
		close_opening();
		return FAIL;
	}

TEST_PRINT("try to open root again, should fail\n");
	fd = file_sys_open((const uint8_t *)flie_list[0]);
	if (fd != -1)
	{
		TEST_PRINT("wrongly open %s\n", flie_list[0]);
		close_opening();
		return FAIL;
	}
continue_test();
	TEST_PRINT("to test opening invaild file, close one file first\n");
	file_sys_close(get_num_opening() - 1);
	printf("Check: the number of opening files is %d\n", get_num_opening());
	TEST_PRINT("try to open invaild file, should fail\n");
	fd = file_sys_open((const uint8_t *)flie_list[7]);
	if (fd != -1)
	{
		TEST_PRINT("wrongly open an invaild file %s\n", flie_list[0]);
		close_opening();
		return FAIL;
	}
	continue_test();

	TEST_PRINT("The file system WINs... Sooooooooo sad :((((\n");
	close_opening();
	return result;
}


int rtc_test() {
	clear();
    TEST_HEADER;
	int result = PASS;
	
	
    int32_t rate;
    int32_t ret,i,j;

    const uint32_t valid_rate[5] = {2, 4, 32, 128, 1024};
    const uint32_t invalid_rate[5] = {3, 6, 48, 1000, 2048};
    const uint32_t test_count = 6;

	printf("try to open the rtc !!\n");
    int32_t fd = rtc_open((uint8_t *) "rtc");
	printf("rtc open success\n");

    for (i = 0; i < 5; i++) {
        rate = valid_rate[i];
        printf("setting the rate into %uHz ing...\n", rate);
		ret = rtc_write(fd, &rate, 4);
        if (0 != ret) {
            printf("fail with %d\n", ret);
            result = FAIL;
        }
        for (j = 0; j < test_count*valid_rate[i]; j++) {
            rtc_read(fd, NULL, 0);
            printf("1");
		}
    }
	for (i = 0; i < 5; i++) {
        rate = invalid_rate[i];
        printf("setting the rate into invalid %uHz...\n", rate);
		ret = rtc_write(fd, &rate, 4);
        if (0 == ret) {
			printf("fail with the invalid argument test\n");
            result = FAIL;
        }else {
			printf("faild!!!\n");
		}
    }
    rtc_close(fd);
	return result;
}

int terminal_test(){
	clear();
	int32_t cnt;
    uint8_t buf[32];
	int8_t* buf2 = "391OS> ";

   while (1){
	   if (-1 == (cnt = terminal_write (1, buf2, 7))) {
			printf("ERROR writing the terminal! \n");
		}
	    if (-1 == (cnt = terminal_read (0, buf, 31))) {
		printf("ERROR reading the terminal! \n");
		}else{
			printf("keyboard buffer is %s \n", buf); 
		}
   }

   return 0;
    
    
}

void continue_test()
{
	printf("\n-------press ENTER to awake it (and test stdin craftily)---------\n");
	file_sys_read(STDIN_FD, file_buf, FILE_NAME_LENGTH);
}

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */

/* Test suite entry point */

void launch_tests(){
	//keyboard_test();
	//TEST_OUTPUT("idt_test", idt_test());
	//TEST_OUTPUT("idt_div0_test", idt_div0_test());	
	//TEST_OUTPUT("paging_test", paging_test());
	//TEST_OUTPUT("idt_dereference_test", idt_dereference_test());
	//TEST_OUTPUT("Keyboard_test", keyboard_test());
	//TEST_OUTPUT("pic_garbage_test", pic_garbage_test());
	//TEST_OUTPUT("terminal test", terminal_test());
	//TEST_OUTPUT("rtc_test", rtc_test());
	TEST_OUTPUT("file_sys_test", file_sys_test());

}
