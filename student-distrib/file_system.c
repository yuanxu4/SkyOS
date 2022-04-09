/*
 * file_system.c
 *
 * Description:
 * sourse code of file system
 *
 * ECE 391 SP 2022
 * History:
 * create file              - Mar 23, keyi
 * implement version 1.0    - Mar 24, keyi
 *
 */

#include "file_system.h"
#include "lib.h"
#include "keyboard.h"
#include "task.h"

#define PRINT_MODE 1
#if PRINT_MODE
#define PRINT(fmt, ...)             \
    do                              \
    {                               \
        printf(fmt, ##__VA_ARGS__); \
    } while (0)

#else
#define PRINT(fmt, ...) \
    do                  \
    {                   \
    } while (0)

#endif
// get the min between a and b
#define MIN(a, b) (a > b ? b : a)

// File System Utilities, define a file system
static boot_block_t *boot_block;
static inode_t *inodes;
static data_block_t *data_blocks;

// File System Abstractions
static jump_table_t op_table_total[NUM_FILE_TYPE]; // op table with 12 ops

extern PCB_t *curr_task(); // defined in boot.S

// help functions
/*
 * void set_entry(file_array_t *fd_array, int32_t fd, int32_t file_type)
 * help function to set entry in file array when opening file
 * Inputs:  fd_array -- given pointer to file array
 *          fd -- The file desc(index) in file array
 *          file_type -- the type of file
 * Outputs: None
 * Side Effects: set entry in file array
 * return value: none
 */
void set_entry(file_array_t *fd_array, int32_t fd, int32_t file_type)
{
    fd_array->entries[fd].op_tbl_ptr = &op_table_total[file_type];
    // PRINT("%#d, %#d, %#d\n",fd_array->entries[fd].op_tbl_ptr, op_table_total, op_table_total+file_type);
    fd_array->entries[fd].inode = 0;
    fd_array->entries[fd].file_position = 0;
    fd_array->entries[fd].flags = IN_USE;
    return;
}

/*
 * int32_t find_unused_fd()
 * help function to find an unused file desc
 * Inputs:  none
 * Outputs: None
 * Side Effects: none
 * return value: an unused file desc or -1 for failure
 */
int32_t find_unused_fd()
{
    int fd;
    // start from 2 since 0/1 for stdin/out
    for (fd = 2; fd < MAX_NUM_OPEN; fd++)
    {
        if (!curr_task()->fd_array.entries[fd].flags)
        {
            return fd;
        }
    }
    return -1;
}

/*
 * int32_t get_file_num()
 * help function to get number of files in system
 * Inputs:  none
 * Outputs: None
 * Side Effects: none
 * return value: number of files
 */
int32_t get_file_num()
{
    return boot_block->dir_count;
}

// int32_t get_file_name()
// {
//     int i;
//     for (i = 0; i < 15; i++)
//     {
//         PRINT("index %d, name: %s, %d\n", i, boot_block->dentries[i].file_name, strlen((const int8_t *)&boot_block->dentries[i].file_name));
//     }
//     return 0;
// }

/*
 * int32_t get_file_size(uint32_t inode)
 * help function to get size of fegular file
 * Inputs:  inode
 * Outputs: None
 * Side Effects: none
 * return value: length or -1 for failure
 */
int32_t get_file_size(uint32_t inode)
{
    if (inode >= boot_block->inode_count)
    {
        PRINT("fail to get file size. bad inode\n");
        return -1;
    }
    return inodes[inode].length;
}

/*
 * int32_t get_num_opening()
 * help function to get number of opening files
 * Inputs:  nine
 * Outputs: None
 * Side Effects: none
 * return value: number of opening files
 */
int32_t get_num_opening()
{
    return curr_task()->fd_array.num_opening;
}

/*
 * int32_t close_opening()
 * help function to close all opening files
 * Inputs:  nine
 * Outputs: None
 * Side Effects: none
 * return value: number of closed files
 */
int32_t close_opening()
{
    int cnt;
    int fd;
    for (fd = 2; fd < MAX_NUM_OPEN; fd++)
    {
        if (curr_task()->fd_array.entries[fd].flags)
        {
            file_sys_close(fd);
            cnt++;
        }
        if (curr_task()->fd_array.num_opening == 2)
        {
            break;
        }
    }
    return cnt;
}

/* external functions of file system */

/*
 * int32_t file_sys_init(module_t *fs)
 * Initialize the file system
 * Inputs: fs -- The file system image that is loaded as module
 * Outputs: None
 * Side Effects: init the varabies above for file system
 * return value: 0 when succuss
 */
int32_t file_sys_init(module_t *fs)
{
    boot_block = (boot_block_t *)fs->mod_start;
    inodes = (inode_t *)(boot_block + 1);
    data_blocks = (data_block_t *)(inodes + boot_block->inode_count);

    // init operation jump tables
    op_table_total[0].open = rtc_user_open;
    op_table_total[0].read = rtc_user_read;
    op_table_total[0].write = rtc_user_write;
    op_table_total[0].close = rtc_user_close;

    op_table_total[1].open = dir_open;
    op_table_total[1].read = dir_read;
    op_table_total[1].write = dir_write;
    op_table_total[1].close = dir_close;

    op_table_total[2].open = file_open;
    op_table_total[2].read = file_read;
    op_table_total[2].write = file_write;
    op_table_total[2].close = file_close;

    op_table_total[3].open = terminal_open;
    op_table_total[3].read = terminal_read;
    op_table_total[3].write = stdin_write;
    op_table_total[3].close = terminal_close;

    op_table_total[4].open = terminal_open;
    op_table_total[4].read = stdout_read;
    op_table_total[4].write = terminal_write;
    op_table_total[4].close = terminal_close;

    init_file_array(&curr_task()->fd_array);
    PRINT("init fs\n");
    return 0;
}

/*
 * int32_t init_file_array(file_array_t *fd_array)
 * Initialize a file array
 * Inputs: fd_array -- The pointer to file array
 * Outputs: None
 * Side Effects: init file array
 * return value: 0 when succuss
 */
int32_t init_file_array(file_array_t *fd_array)
{
    int i; // loop counter
    if (fd_array == NULL)
    {
        return -1;
    }
    // init stdin and stdout
    // stdin is a read-only file which corresponds to keyboard input.
    // stdout is a write-only file corresponding to terminal output.
    // terminal operations should be implemented in terminal.c by haina

    set_entry(fd_array, STDIN_FD, 3);
    set_entry(fd_array, STDOUT_FD, 4);

    fd_array->num_opening = 2;

    // set others as not used
    for (i = 2; i < MAX_NUM_OPEN; i++)
    {
        fd_array->entries[i].flags = NOT_IN_USE;
    }
    return 0;
}

/*
 * int32_t deactivate_file_array(file_array_t *fd_array)
 * deactivate a file array
 * Inputs: fd_array -- The pointer to file array
 * Outputs: None
 * Side Effects: init file array
 * return value: 0 when succuss
 */
int32_t deactivate_file_array(file_array_t *fd_array)
{
    int fd;
    if (fd_array == NULL)
    {
        return -1;
    }
    // close all files
    for (fd = 2; fd < MAX_NUM_OPEN; fd++)
    {
        file_sys_close(fd); // may print info for unopened file
    }
    return 0;
}

// opertions of file system, called when system call
/*
 * int32_t file_sys_open(const uint8_t *filename)
 * try to open a file in the file system
 * Inputs:  filename -- The name of the file to open
 * Outputs: None
 * Side Effects: change curr_task()->fd_array
 * return value: -1 for failure, the file descriptor for succuss
 */
int32_t file_sys_open(const uint8_t *filename)
{
    dentry_t copied_dentry;
    // PRINT("addr copy dentry: %#x\n", &copied_dentry);
    int32_t fd = -1; // the returned file descriptor if succuss
    // reach max opening
    if (curr_task()->fd_array.num_opening >= MAX_NUM_OPEN)
    {
        PRINT("fail to open %s . number of opening files is max.\n", filename);
        return -1;
    }
    // PRINT("start read dentry\n");
    // return != 0, fail to find the file
    if (read_dentry_by_name(filename, &copied_dentry))
    {
        PRINT("fail to open %s . non-existent file.\n", filename);
        return -1;
    }
    // unknown file type, should never happen correctly
    if (copied_dentry.file_type > 2)
    {
        PRINT("fail to open %s . unknown file type\n", filename);
        return -1;
    }
    fd = op_table_total[copied_dentry.file_type].open(filename); // set flag included
    // fail to open, should never happen correctly
    if (fd < 0)
    {
        PRINT("fail to open file. unknown reason\n");
        return -1;
    }
    curr_task()->fd_array.num_opening++; // inc
    return fd;
}

/*
 * int32_t file_sys_close(int32_t fd)
 * try to close a file in the file system
 * Inputs:  fd -- The file descriptor of the file to close
 * Outputs: None
 * Side Effects: change curr_task()->fd_array
 * return value: 0 for success, -1 for failure
 */
int32_t file_sys_close(int32_t fd)
{
    // fd is invalid
    if (fd == 0 || fd == 1)
    {
        PRINT("fail to close. cannot close stdin/stdout\n");
        return -1;
    }
    if (fd < 0 || fd >= MAX_NUM_OPEN)
    {
        PRINT("fail to close file. invaild file descriptor %d\n", fd);
        return -1;
    }
    // file not opening
    if (!curr_task()->fd_array.entries[fd].flags)
    {
        PRINT("fail to close file. unopen file\n");
        return -1;
    }
    // fail to close, should never happen correctly
    if (curr_task()->fd_array.entries[fd].op_tbl_ptr->close(fd))
    {
        PRINT("fail to close file. unknown reason\n");
        return -1;
    }
    curr_task()->fd_array.entries[fd].flags = NOT_IN_USE; // set flag not included in close
    curr_task()->fd_array.num_opening--;                  // dec
    return 0;
}

/*
 * int32_t file_sys_read(int32_t fd, void *buf, int32_t nbytes)
 * try to read data from file to buf
 * Inputs:  fd -- The file descriptor of the file to read
 *          buf -- The buffer to store data
 *          nbytes -- The number of bytes to read
 * Outputs: None
 * Side Effects: change file array
 * return value: the number of bytes read, 0 if offset reach the end of the file,
 *          -1 for failure
 *
 */
int32_t file_sys_read(int32_t fd, void *buf, int32_t nbytes)
{
    // not opening
    if (!curr_task()->fd_array.entries[fd].flags)
    {
        PRINT("fail to read. file is not open\n");
        return -1;
    }
    // update position field included in specfic read functions
    return curr_task()->fd_array.entries[fd].op_tbl_ptr->read(fd, buf, nbytes);
}

/*
 * int32_t file_sys_write(int32_t fd, const void *buf, int32_t nbytes)
 * try to write data from file to buf
 * Inputs:  fd -- The file descriptor of the file to write
 *          buf -- The buffer storing data
 *          nbytes -- The number of bytes to write
 * Outputs: None
 * Side Effects: none
 * return value: 0 for success(do nothing since read only), -1 for failure
 *
 */
int32_t file_sys_write(int32_t fd, const void *buf, int32_t nbytes)
{
    // not opening
    if (!curr_task()->fd_array.entries[fd].flags)
    {
        PRINT("fail to write. file is not open\n");
        return -1;
    }

    return curr_task()->fd_array.entries[fd].op_tbl_ptr->write(fd, buf, nbytes);
}

/*
 * int32_t file_load(dentry_t *file_dentry, void *vir_addr)
 * copy a program image in the file system into contiguous physical memory
 * Inputs:  file_dentry -- The dentry of the program file in system
 *          vir_addr -- The target address in virtual memory
 * Outputs: None
 * Side Effects: none
 * return value: The number of bytes loaded
 */
int32_t file_load(dentry_t *file_dentry, uint8_t *vir_addr)
{
    return read_data(file_dentry->inode_num, 0, vir_addr, get_file_size(file_dentry->inode_num));
}

/*
 * int32_t is_exe_file(dentry_t *file_dentry)
 * check if the file is executable
 * Inputs:  file_dentry -- The dentry of the file
 * Outputs: None
 * Side Effects: none
 * return value: 1 if exe, else 0
 */
int32_t is_exe_file(dentry_t *file_dentry)
{
    uint8_t buf[SIZE_4B + 1];                           // buffer, for safty
    read_data(file_dentry->inode_num, 0, buf, SIZE_4B); // read first 4B
    return (buf[0] == 0x7F) & (buf[1] == 0x45) & (buf[2] == 0x4C) & (buf[3] == 0x46);
}

/*
 * uint32_t get_eip(dentry_t *exe_dentry)
 * get the eip for the exe file
 * Inputs:  exe_dentry -- The dentry of the exe file
 * Outputs: None
 * Side Effects: none
 * return value: eip
 */
uint32_t get_eip(dentry_t *exe_dentry)
{
    uint32_t eip = 0;
    if (SIZE_4B != read_data(exe_dentry->inode_num, EIP_POS, (uint8_t *)&eip, SIZE_4B))
    {
        return -1;
    }
    return eip;
}

// 3 reading routines
/*
 * read_dentry_by_name(const uint8_t *fname, dentry_t *dentry)
 * find the directory entry according name and store it
 * Inputs:  fname -- The file name of the dir entry
 *          dentry -- The dentry to store result
 * Outputs: None
 * Side Effects: none
 * return value: 0 for success, -1 for failure
 *
 */
int32_t read_dentry_by_name(const uint8_t *fname, dentry_t *dentry)
{
    int i; // just for loop

    // length of the file name > 32
    if (strlen((const int8_t *)fname) > MAX_LEN_FILE_NAME + 1)
    {
        PRINT("name too loooooong, which is %d\n", strlen((const int8_t *)fname));
        return -1;
    }
    // traverse dentries
    for (i = 0; i < boot_block->dir_count; i++)
    {
        // check names, 0 means the same, i.e. find
        if (!strncmp((const int8_t *)fname, (const int8_t *)boot_block->dentries[i].file_name, MAX_LEN_FILE_NAME))
        {
            // copy and return
            *dentry = boot_block->dentries[i];
            return 0;
        }
    }
    // not find the corresponding dentry
    return -1;
}

/*
 * int32_t read_dentry_by_index(uint32_t index, dentry_t *dentry)
 * find the directory entry according index and store it
 * Inputs:  index -- The index the dir entry
 *          dentry -- The dentry to store result
 * Outputs: None
 * Side Effects: none
 * return value: 0 for success, -1 for failure
 *
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t *dentry)
{
    int i; // just for loop
    // traverse dentries
    for (i = 0; i < boot_block->dir_count; i++)
    {
        // index find
        if (index == boot_block->dentries[i].inode_num)
        {
            // copy and return
            *dentry = boot_block->dentries[i];
            return 0;
        }
    }
    // not find the corresponding dentry
    return -1;
}

/*
 * int32_t read_data(uint32_t inode, uint32_t offset, uint8_t *buf, uint32_t length)
 * read data with up to length starting from the offset in the file and store into buffer
 * Inputs:  inode -- The inode number of the file to be read
 *          offset -- The position offset in the file
 *          buf -- The buffer to store data
 *          length -- The length in bytes to read
 * Outputs: None
 * Side Effects: none
 * return value: The number of bytes read and placed into buffer
 *          0 if offset reach the end of the file, -1 for failure
 *
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t *buf, uint32_t length)
{
    // inode invalid
    if ((inode >= boot_block->inode_count))
    {
        PRINT("fail to read. bad input\n");
        return -1;
    };
    uint32_t file_len = inodes[inode].length;
    // offset reach the end of the file
    if (offset >= file_len)
    {
        PRINT("\n\nread nothing. reach to the end\n");
        return 0;
    }
    uint32_t copy_size = MIN((file_len - offset), length);                       // size to copy
    uint32_t dt_blk_idx_in_inode = offset / BLOCK_SIZE;                          // index of data block in inode, init as the starting block
    uint32_t local_offset = offset % BLOCK_SIZE;                                 // offset in the data block
    uint32_t dt_blk_idx_abs = inodes[inode].data_block_num[dt_blk_idx_in_inode]; // index of data block absolutely
    uint32_t dt_blk_num = boot_block->data_block_count;                          // number of data block in system
    int32_t copied;                                                              // copied size
    // start copy
    for (copied = 0; copied < copy_size; copied++)
    {
        // index invalid, should never happen
        if (dt_blk_idx_abs >= dt_blk_num)
        {
            PRINT("stop copying. out of range\n");
            return copied;
        }
        // copy 1 bit
        buf[copied] = data_blocks[dt_blk_idx_abs].data[local_offset];
        local_offset++;
        // reach to end of one data block, move to the next block
        if (local_offset == BLOCK_SIZE)
        {
            local_offset = 0;
            dt_blk_idx_in_inode++;
            dt_blk_idx_abs = inodes[inode].data_block_num[dt_blk_idx_in_inode];
        }
    }
    return copied;
}

// Operation of the regular file
/*
 * int32_t file_open(const uint8_t *filename)
 * try to open a regular file in the file system
 * Inputs:  filename -- The name of the file to open
 * Outputs: None
 * Side Effects: change file array
 * return value: -1 for failure, else the file descriptor
 */
int32_t file_open(const uint8_t *filename)
{
    dentry_t file_dentry;
    int32_t fd;
    // actually need not check but need to get dentry
    if (read_dentry_by_name(filename, &file_dentry))
    {
        return -1;
    }
    fd = find_unused_fd();
    // available fd, actually need not check
    if (fd != -1)
    {
        set_entry(&curr_task()->fd_array, fd, 2); // type is 2 for regular file
        curr_task()->fd_array.entries[fd].inode = file_dentry.inode_num;
    }
    return fd;
}

/*
 * int32_t file_close(int32_t fd)
 * try to close a regular file in the file system
 * Inputs:  fd -- The file descriptor of the file to close
 * Outputs: None
 * Side Effects: none
 * return value: 0 for success, -1 for failure
 */
int32_t file_close(int32_t fd)
{
    return 0;
}

/*
 * int32_t file_read(int32_t fd, void *buf, int32_t nbytes)
 * try to read data from a regular file to buf
 * Inputs:  fd -- The file descriptor of the file to read
 *          buf -- The buffer to store data
 *          nbytes -- The number of bytes to read
 * Outputs: None
 * Side Effects: change file array
 * return value: the number of bytes read, 0 if offset reach the end of the file,
 *          -1 for failure
 *
 */
int32_t file_read(int32_t fd, void *buf, int32_t nbytes)
{

    int32_t copy_size;                                                 // the size of copied data
    uint32_t offset = curr_task()->fd_array.entries[fd].file_position; // current position in file

    // Place the data into buffer
    copy_size = read_data(curr_task()->fd_array.entries[fd].inode, offset, buf, nbytes);

    if (copy_size > 0)
    {
        curr_task()->fd_array.entries[fd].file_position += copy_size;
    }

    return copy_size;
}

/*
 * nt32_t file_write(int32_t fd, const void *buf, int32_t nbytes)
 * try to write data from a regular file to buf(do nothing since read only)
 * Inputs:  fd -- The file descriptor of the file to write
 *          buf -- The buffer storing data
 *          nbytes -- The number of bytes to write
 * Outputs: None
 * Side Effects: none
 * return value: 0 for success, -1 for failure
 *
 */
int32_t file_write(int32_t fd, const void *buf, int32_t nbytes)
{
    PRINT("fail to write. read only\n");
    return -1;
}

// Operation of the directory
/*
 * int32_t dir_open(const uint8_t *filename)
 * try to open a directory in the file system
 * Inputs:  filename -- The name of the file to open
 * Outputs: None
 * Side Effects: change file array
 * return value: -1 for failure, else the file descriptor
 */
int32_t dir_open(const uint8_t *filename)
{
    int32_t fd = find_unused_fd();
    // available fd, actually need not check
    if (fd != -1)
    {
        set_entry(&curr_task()->fd_array, fd, 1); // type is 1 for directory
    }

    return fd;
}

/*
 * int32_t dir_close(int32_t fd)
 * try to close a directory in the file system
 * Inputs:  fd -- The file descriptor of the file to close
 * Outputs: None
 * Side Effects: none
 * return value: 0 for success, -1 for failure
 */
int32_t dir_close(int32_t fd)
{
    return 0;
}

/*
 * int32_t dir_read(int32_t fd, void *buf, int32_t nbytes)
 * try to read data from a directory to buf
 * only read one file name at a time.
 * Inputs:  fd -- The file descriptor of the directory to read
 *          buf -- The buffer to store data
 *          nbytes -- The number of bytes to read
 * Outputs: None
 * Side Effects: change file array
 * return value: the number of bytes read, 0 if offset reach the end of the file,
 *          -1 for failure
 *
 */
int32_t dir_read(int32_t fd, void *buf, int32_t nbytes)
{
    uint32_t pst = curr_task()->fd_array.entries[fd].file_position; // position in directory
    int32_t copy_size = MIN(nbytes, MAX_LEN_FILE_NAME);             // size to copy
    if (pst >= boot_block->dir_count)
    {
        PRINT("\nread nothing in directory. reach to the end\n");
        return 0;
    }
    // copy
    strncpy(buf, (const int8_t *)(boot_block->dentries[pst].file_name), copy_size);
    curr_task()->fd_array.entries[fd].file_position++;
    return copy_size;
}

/*
 * nt32_t dir_write(int32_t fd, const void *buf, int32_t nbytes)
 * try to write data from a directory to buf (do nothing since read only)
 * Inputs:  fd -- The file descriptor of the file to write
 *          buf -- The buffer storing data
 *          nbytes -- The number of bytes to write
 * Outputs: None
 * Side Effects: none
 * return value: 0 for success, -1 for failure
 *
 */
int32_t dir_write(int32_t fd, const void *buf, int32_t nbytes)
{
    PRINT("fail to write . read only\n");
    return -1;
}

// do not implement for now
// can refer to rtc ops implemented by Yuan
int32_t rtc_user_open(const uint8_t *filename)
{
    return 0;
}
int32_t rtc_user_close(int32_t fd)
{
    return 0;
}
int32_t rtc_user_read(int32_t fd, void *buf, int32_t nbytes)
{
    return 0;
}
int32_t rtc_user_write(int32_t fd, const void *buf, int32_t nbytes)
{
    return 0;
}

/*
 * int32_t stdout_read(int32_t fd, void *buf, int32_t nbytes)
 * try to read data from stdout to buf
 * only read one file name at a time.
 * Inputs:  fd -- The file descriptor of the directory to read
 *          buf -- The buffer to store data
 *          nbytes -- The number of bytes to read
 * Outputs: None
 * Side Effects: none
 * return value:
 *          -1 for failure
 *
 */
int32_t stdout_read(int32_t fd, void *buf, int32_t nbytes)
{
    PRINT("fail to read in stdout. write only\n");
    return -1;
}

/*
 * nt32_t stdin_write(int32_t fd, const void *buf, int32_t nbytes)
 * try to write data from stdin to buf (do nothing since read only)
 * Inputs:  fd -- The file descriptor of the file to write
 *          buf -- The buffer storing data
 *          nbytes -- The number of bytes to write
 * Outputs: None
 * Side Effects: none
 * return value: 0 for success, -1 for failure
 *
 */
int32_t stdin_write(int32_t fd, const void *buf, int32_t nbytes)
{
    PRINT("fail to write in stdin. read only\n");
    return -1;
}


/*
 * get_file_array
 * description: return the file_array's pointer
 * input: none
 * output: none
 * return pointer to file_array
 */
// file_array_t* get_file_array(){
//     return &file_array;
// }

