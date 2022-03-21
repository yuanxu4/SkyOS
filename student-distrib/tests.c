#include "tests.h"
#include "x86_desc.h"
#include "lib.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
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
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

// add more tests here
int idt_div0_test(){
	TEST_HEADER;

	unsigned long div = 10;
	unsigned long zero = 0;
	unsigned result = div / zero;

	return FAIL;
}

int idt_dereference_test(){
	unsigned long i = 1;
	// unsigned long* invalid_ptr = (unsigned long *)i;
	// unsigned long j = *(invalid_ptr);
	unsigned long j = *((unsigned long *) i);

	return FAIL;
}

int keyboard_test(){
	TEST_HEADER;
	while(1){}
}

/* Checkpoint 2 tests */
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	TEST_OUTPUT("idt_test", idt_test());
	// TEST_OUTPUT("idt_div0_test", idt_div0_test());
	TEST_OUTPUT("idt_dereference_test", idt_dereference_test());
	// TEST_OUTPUT("Keyboard_test", keyboard_test());
	// launch your tests here
}
