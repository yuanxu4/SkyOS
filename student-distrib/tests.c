#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "rtc.h"

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

/* Checkpoint 2 tests */
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
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	TEST_OUTPUT("idt_test", idt_test());
	// launch your tests here
}
