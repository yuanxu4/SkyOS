/*
 * file_system.h
 *
 * Description:
 * head file of file system
 *
 * ECE 391 SP 2022
 * History:
 * create file              - Mar 23, keyi
 * implement version 1.0    - Mar 24, keyi
 *
 */

#ifndef _FILE_SYSTEM_H
#define _FILE_SYSTEM_H

#include "multiboot.h"
#include "types.h"
#include "rtc.h"
#include "memory.h"
//#include "GUI/gui_desktop.h"

#define MAX_LEN_FILE_NAME 32                                      // 32B, the max length of file name
#define BLOCK_SIZE 0x1000                                         // 4KB, size of block in this file system
#define DIR_ENTRY_SIZE 64                                         // 64B, size of directory entry in boot block
#define NUM_DIR_ENTRY ((BLOCK_SIZE / DIR_ENTRY_SIZE) - 1)         // 63, number of directory entries in boot block
#define INDEX_DATA_BLOCK_SIZE 4                                   // 4B, size of the index of data block in inode
#define NUM_DATA_BLOCK ((BLOCK_SIZE / INDEX_DATA_BLOCK_SIZE) - 1) // 1023, number of data blocks in inode
#define MAX_NUM_OPEN 8                                            // max number of opening files in one task
#define NUM_FILE_TYPE 5                                           // number of file types, rtc, dir, file, stdin, stdout
// value of “flags” in file arrat to indicate this file descriptor is “in-use.” or not
#define IN_USE 1
#define NOT_IN_USE 0

#define STDIN_FD 0  // file desc of stdin
#define STDOUT_FD 1 // file desc of stdout

#define SIZE_4B 4  // 4B,
#define EIP_POS 24 // the starting point in the exe file

/* structures of File System Utilities in 8.1 */

// 64B directory entry

typedef struct dentry dentry_t;
struct dentry
{
    uint8_t file_name[MAX_LEN_FILE_NAME]; // file name
    uint32_t file_type;                   // file type, 0 RTC; 1 directory; 2 regular file
    uint32_t inode_num;                   // inode number, for regular file
    uint8_t *parent_fname;                // parent dir dentry id
    dentry_t *dentry_addr;                // dentry_addr
    uint8_t reserved[16];                 // reserve 8B
};

// 4KB boot block
typedef struct boot_block
{
    uint32_t dir_count;        // # of directories
    uint32_t inode_count;      // # of inodes (N), 64
    uint32_t data_block_count; // # of data blocks (D)
    uint8_t reserved[52];      // reserve 52B
    dentry_t dentries[NUM_DIR_ENTRY];
} boot_block_t;

// 4KB index node
typedef struct inode
{
    int32_t length; // the length of the file in Bytes
    int32_t data_block_num[NUM_DATA_BLOCK];
    // only those blocks necessary to contain the specified size need be valid,
} inode_t;

// 4KB data block
typedef struct data_block
{
    uint8_t data[BLOCK_SIZE];
} data_block_t;

/* structure of File System Abstractions in 8.2 */

// function pointers of operations

// The file operations jump table
typedef struct jump_table
{
    // interfaces march that of system calls in apppedix B
    // open is used for performing type-specific initialization
    int32_t (*open)(const uint8_t *filename);
    int32_t (*read)(int32_t fd, void *buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void *buf, int32_t nbytes);
    int32_t (*close)(int32_t fd);
} jump_table_t;

// entry in file array
typedef struct file_array_entry
{
    jump_table_t *op_tbl_ptr; // file operations jump table
    uint32_t inode;           // inode number for this file, 0 for dir/RTC
    uint32_t file_position;   // “file position” member
    uint32_t flags;           // “flags” member
    uint8_t *fname;
} file_array_entry_t;

typedef struct file_array
{
    int32_t num_opening;                      // number of opening files, self-defined for convenience
    file_array_entry_t entries[MAX_NUM_OPEN]; // 8 entries
    // the index is called file descriptor
} file_array_t;

/* external functions of file system */

// init file system, called in kernel.c
int32_t file_sys_init(module_t *fs);
// opertions of file system, called when system call
int32_t file_sys_open(const uint8_t *filename);
int32_t file_sys_read(int32_t fd, void *buf, int32_t nbytes);
int32_t file_sys_write(int32_t fd, const void *buf, int32_t nbytes);
int32_t file_sys_close(int32_t fd);

// added in cp3
int32_t file_load(dentry_t *file_dentry, uint8_t *vir_addr);
int32_t is_exe_file(dentry_t *file_dentry);
uint32_t get_eip(dentry_t *exe_dentry);
int32_t init_file_array(file_array_t *fd_array);
int32_t deactivate_file_array(file_array_t *fd_array);

/* internal functions of file system */

// reading routines

int32_t read_dentry_by_name(const uint8_t *fname, dentry_t *dentry);
int32_t read_dentry_by_index(uint32_t index, dentry_t *dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t *buf, uint32_t length);

// Operation of the regular file

int32_t file_open(const uint8_t *filename);
int32_t file_read(int32_t fd, void *buf, int32_t nbytes);
int32_t file_write(int32_t fd, const void *buf, int32_t nbytes);
int32_t file_close(int32_t fd);

// Operation of the directory file

int32_t dir_open(const uint8_t *filename);
int32_t dir_read(int32_t fd, void *buf, int32_t nbytes);
int32_t dir_write(int32_t fd, const void *buf, int32_t nbytes);
int32_t dir_close(int32_t fd);

// seems not necessary now
// Operation of rtc for user

int32_t rtc_user_open(const uint8_t *filename);
int32_t rtc_user_read(int32_t fd, void *buf, int32_t nbytes);
int32_t rtc_user_write(int32_t fd, const void *buf, int32_t nbytes);
int32_t rtc_user_close(int32_t fd);

// tmp declaration!!!
// used for stdin / stdout
// int32_t terminal_open(const uint8_t *filename);
// int32_t terminal_read(int32_t fd, void *buf, int32_t nbytes);
// int32_t terminal_write(int32_t fd, const void *buf, int32_t nbytes);
// int32_t terminal_close(int32_t fd);

int32_t stdout_read(int32_t fd, void *buf, int32_t nbytes);
int32_t stdin_write(int32_t fd, const void *buf, int32_t nbytes);

// help function

void set_entry(file_array_t *fd_array, int32_t fd, int32_t file_type, uint8_t *fname);
int32_t find_unused_fd();
// used in test

int32_t get_file_size(uint32_t inode);
int32_t get_num_opening();
int32_t close_opening();
// int32_t get_file_name();
int32_t get_file_num();

uint8_t *get_all_file_name();
dentry_t *get_dentry(int index);

int32_t del_file(uint8_t *fname);

file_array_t *get_file_array();

int32_t disp_dentry(dentry_t *file_dentry);
int32_t check_parent(uint8_t *dir_name, dentry_t *file_dentry);
int32_t fs_create(int32_t type, uint8_t *fname, dentry_t *dir_dentry);
int32_t fs_delete(int32_t type, uint8_t *fname, dentry_t *dir_dentry);
int32_t fs_read(int32_t type, uint8_t *buf, dentry_t *dir_dentry);
int32_t fs_getparent(int32_t type, uint8_t *buf, dentry_t *file_dentry);
int32_t fs_ifkid(int32_t type, uint8_t *fname, dentry_t *dir_dentry);
int32_t if_belong_dir(dentry_t *file_dentry, dentry_t *dir_dentry);
#endif // _FILE_SYSTEM_H
