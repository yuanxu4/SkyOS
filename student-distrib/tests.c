/*
 * ECE 391 SP 2022
 * History:
 * add paging test          - Mar 20, keyi
 *
 */

#include "tests.h"
#include "x86_desc.h"
#include "lib.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER \
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result) \
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

#define TEST_PRINT_MODE 1
#if TEST_PRINT_MODE
#define TEST_PRINT(fmt, ...)                                  \
	do                                                        \
	{                                                         \
		printf("[TEST %s]" fmt, __FUNCTION__, ##__VA_ARGS__); \
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

/* paging Test - Example
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
		TEST_PRINT("Error!!!\nCR0 is %x\n", cr_0);
		result = FAIL;
	}
	// if equal, means cr3 base addr,PCD(0),PWT(0) all right, ignored as 0
	if ((PD_t *)cr_3 != &page_directory)
	{
		TEST_PRINT("Error!!!\nCR3 is %x, and page directory addr is %x\n", cr_3, &page_directory);
		result = FAIL;
	}
	// check cr4 pse set
	if ((cr_4 & 0x00000010) == 0)
	{
		TEST_PRINT("Error!!!\nCR4 is %x\n", cr_4);
		result = FAIL;
	}
	// check pde
	// entry 0, check ptba, high20bits
	tmp = (uint32_t *)(page_directory.pde[0] & 0xFFFFF000);
	if ((PT_t *)tmp != &page_table)
	{
		TEST_PRINT("Error!!!\nPT base addr in PD is %x, but actually %x", tmp, &page_table);
		result = FAIL;
	}
	// entry 0, check others, like ps, p
	if ((page_directory.pde[0] & 0x000000FF) != 0x00000003)
	{
		TEST_PRINT("Error!!!\nPT is %x, set wrongly", page_directory.pde[0]);
		result = FAIL;
	}
	// entry 1, pba, high10bits
	tmp = (void *)(page_directory.pde[1] & 0xFFC00000);
	if (tmp != (void *)0x00400000)
	{
		TEST_PRINT("Error!!!\nKernel Page base addr in PD is %x, but actually %x", tmp, 0x00400000);
		result = FAIL;
	}
	// entry 1, check others, like ps, p
	if ((page_directory.pde[1] & 0x000000FF) != 0x00000083)
	{
		TEST_PRINT("Error!!!\nKernel Page is %x, set wrongly", page_directory.pde[0]);
		result = FAIL;
	}
	return result;
	// check video memory
	if (page_table.pte[VIDEO_MEM_INDEX] != 0x000B8003)
	{
		TEST_PRINT("Error!!!\nPTE for video memory is %x", page_table.pte[VIDEO_MEM_INDEX]);
		result = FAIL;
	}
	// check other present
	for (i = 0; i < VIDEO_MEM_INDEX; i++)
	{
		if ((page_table.pte[i] & 0x00000001))
		{
			TEST_PRINT("Error!!!\nPTE %d is present", i);
			result = FAIL;
		}
	}
	for (i = VIDEO_MEM_INDEX + 1; i < PT_SIZE; i++)
	{
		if ((page_table.pte[i] & 0x00000001))
		{
			TEST_PRINT("Error!!!\nPTE %d is present", i);
			result = FAIL;
		}
	}
	// dereference test
	// dereference in video memory
	tmp = (uint32_t *)(0xB8000 + 8);
	if (*tmp)
	{
		/* fine */
	}
	// dereference in kernel page 4-8MB
	tmp = (uint32_t *)(0x400000 + 8);
	if (*tmp)
	{
		/* fine */
	}

	// dereference in 0-4MB outside video memory
	tmp = (uint32_t *)(0xB8000 - 8);
	if (*tmp)
	{
		/* should fault */
		result = FAIL;
	}
	// dereference partly <8MB, partly >8MB
	tmp = (uint32_t *)(0x800000 - 4);
	if ((long long)*tmp)
	{
		/* should fault */
		result = FAIL;
	}
	return result;
}

/* Checkpoint 2 tests */
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */

/* Test suite entry point */
void launch_tests()
{
	TEST_OUTPUT("idt_test", idt_test());
	TEST_OUTPUT("paging_test", paging_test());
	// launch your tests here
}
