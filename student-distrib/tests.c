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
	TEST_HEADER;
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
	TEST_HEADER;
	int result = PASS;

	int32_t fd;					  // file desc
	int8_t buf[FILE_NAME_LENGTH]; // buffer, avoid out of range
	// 8 files, 1 root, 2 small files, 2 executables, 2 largefiles, 1 invaild file
	const char *flie_list[8] = {".", "frame0.txt", "frame1.txt", "grep", "ls", "fish", "verylargetextwithverylongname.tx", "wqevfwrtvwev"};
	int32_t tmp;
	dentry_t *tmp_dentry;
	int i;

	clear();

	// open root
	fd = file_sys_open((const uint8_t *)flie_list[0]);
	if (fd != 2)
	{
		TEST_PRINT("fail in test open root, fd is %d\n", fd);
		close_opening();
		return FAIL;
	}

	// read file names in root
	TEST_PRINT("read the root directory\n");
	for (i = 0; i < NUM_DIR_ENTRY; i++)
	{
		tmp = file_sys_read(fd, buf, FILE_NAME_LENGTH);
		if (tmp > 0)
		{
			// write -1 fail, 0 succ
			if (file_sys_write(STDOUT_FD, buf, tmp))
			{
				TEST_PRINT("fail to write in stdout\n");
				close_opening();
				return FAIL;
			}
			// check file type and size
			// -1 fail, 0 succ
			if (read_dentry_by_name((const uint8_t *)buf, tmp_dentry))
			{
				TEST_PRINT("\nfail to find file name\n");
				close_opening();
				return FAIL;
			}
			switch (tmp_dentry->file_type)
			{
			case 0:
				TEST_PRINT("file type: RTC\n");
				break;
			case 1:
				TEST_PRINT("file type: directory\n");
				break;
			case 2:
				TEST_PRINT("file type: regular file, size(B): %d\n", get_file_size(tmp_dentry->inode_num));
				break;
			default:
				TEST_PRINT("file type: ???\n");
				break;
			}
		}
		else
		{
			TEST_PRINT("\nfail to read file name at %d\n", i);
		}
	}
	// should return 0 and print reach to the end\n
	tmp = file_sys_read(fd, buf, FILE_NAME_LENGTH);
	if (!tmp)
	{
		TEST_PRINT("fail in test reading at end., read returns %d\n", tmp);
		close_opening();
		return FAIL;
	}
	// try to test stdin and stdout
	if (!file_sys_write(STDIN_FD, buf, tmp))
	{
		TEST_PRINT("wrongly write in stdin\n");
		close_opening();
		return FAIL;
	}
	if (!file_sys_read(STDOUT_FD, buf, FILE_NAME_LENGTH))
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

	close_opening();

	TEST_PRINT("cooooool!!!! pass test about root\n ");
	TEST_PRINT("then try to open 6 vaild regular files\n");
	TEST_PRINT("but the file system wants to sleep! \n");

	// open 6 vaild regular files, never close until fail
	for (i = 1; i < 7; i++)
	{
		TEST_PRINT("press a key to awake it (and test stdin craftily)\n");
		file_sys_read(STDIN_FD, buf, FILE_NAME_LENGTH);
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
		if (read_dentry_by_name((const uint8_t *)buf, tmp_dentry))
		{
			TEST_PRINT("fail to find file\n");
			close_opening();
			return FAIL;
		}
		if (tmp_dentry->file_type != 2)
		{
			TEST_PRINT("get wrong file type\n");
			close_opening();
			return FAIL;
		}
		TEST_PRINT("the size of file is %d. below is the content\n", get_file_size(tmp_dentry->inode_num));

		// read file 32B one time
		do
		{
			tmp = file_sys_read(fd, buf, FILE_NAME_LENGTH);
			switch (tmp)
			{
			case 0:
				break;
			case -1:
				TEST_PRINT("fail to read completely\n");
				close_opening();
				return FAIL;
			default:
				if (file_sys_write(STDOUT_FD, buf, tmp))
				{
					TEST_PRINT("fail to display file\n");
					close_opening();
					return FAIL;
				}
				break;
			}
		} while (tmp);
		TEST_PRINT("done...\n");
	}
	// here, 8 files opening
	TEST_PRINT("now we should reach the max number of opening file\n");
	TEST_PRINT("CHECK: the number of opening files is %d\n", get_num_opening());
	if (get_num_opening() != 8)
	{
		TEST_PRINT("fail in test opening files\n");
		close_opening();
		return FAIL;
	}

	TEST_PRINT("press a key to open root again\n");

	fd = file_sys_open((const uint8_t *)flie_list[0]);
	if (fd != -1)
	{
		TEST_PRINT("wrongly open %s\n", flie_list[0]);
		// should close other opening files for safty
		close_opening();
		return FAIL;
	}
	TEST_PRINT("to test opening invaild file, close one file first\n");
	file_sys_close(get_num_opening() - 1);
	TEST_PRINT("CHECK: the number of opening files is %d\n", get_num_opening());
	TEST_PRINT("try to open invaild file\n");
	fd = file_sys_open((const uint8_t *)flie_list[7]);
	if (fd != -1)
	{
		TEST_PRINT("wrongly open an invaild file %s\n", flie_list[0]);
		// should close other opening files for safty
		close_opening();
		return FAIL;
	}
	clear();
	TEST_PRINT("The file system wins :((((\n");
	close_opening();
	return result;
}

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */

/* Test suite entry point */
void launch_tests()
{
	// TEST_OUTPUT("idt_test", idt_test());
	// TEST_OUTPUT("idt_div0_test", idt_div0_test());
	// TEST_OUTPUT("paging_test", paging_test());
	// TEST_OUTPUT("idt_dereference_test", idt_dereference_test());
	// TEST_OUTPUT("Keyboard_test", keyboard_test());
	// TEST_OUTPUT("pic_garbage_test", pic_garbage_test());
	TEST_OUTPUT("file_sys_test", file_sys_test());
}
