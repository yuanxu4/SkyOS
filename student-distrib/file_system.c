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
// determine whether the num is an addr from buddysystem or index
#define ADDR_or_ID(num) (num & BASE_ADDR_BD_SYS)

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
        if (curr_task()->fd_array.entries[fd].flags)
        {
            file_sys_close(fd); // may print info for unopened file
        }
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
    if (filename == NULL)
    {
        printf("fail to open: get NULL filename\n");
        return -1;
    }
    dentry_t copied_dentry;
    // PRINT("addr copy dentry: %#x\n", &copied_dentry);
    int32_t fd = -1; // the returned file descriptor if succuss
    // reach max opening
    if (curr_task()->fd_array.num_opening >= MAX_NUM_OPEN)
    {
        PRINT("fail to open %s: number of opening files is max.\n", filename);
        return -1;
    }
    // PRINT("start read dentry\n");
    // return != 0, fail to find the file
    if (read_dentry_by_name(filename, &copied_dentry))
    {
        PRINT("fail to open %s: non-existent file.\n", filename);
        return -1;
    }
    // unknown file type, should never happen correctly
    if (copied_dentry.file_type > 2)
    {
        PRINT("fail to open %s: unknown file type\n", filename);
        return -1;
    }
    fd = op_table_total[copied_dentry.file_type].open(filename); // set flag included
    // fail to open, should never happen correctly
    if (fd < 0)
    {
        PRINT("fail to open file: unknown reason\n");
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
        PRINT("fail to close: cannot close stdin/stdout\n");
        return -1;
    }
    if (fd < 0 || fd >= MAX_NUM_OPEN)
    {
        PRINT("fail to close file: invaild file descriptor %d\n", fd);
        return -1;
    }
    // file not opening
    if (!curr_task()->fd_array.entries[fd].flags)
    {
        PRINT("fail to close file: unopen file\n");
        return -1;
    }
    // fail to close, should never happen correctly
    if (curr_task()->fd_array.entries[fd].op_tbl_ptr->close(fd))
    {
        PRINT("fail to close file: unknown reason\n");
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
    if (buf == NULL)
    {
        printf("fail to read: get NULL buf\n");
        return -1;
    }
    if (nbytes < 0)
    {
        printf("fail to read: invaild nbytes, %d\n", nbytes);
        return -1;
    }
    if (fd < 0 || fd >= MAX_NUM_OPEN)
    {
        PRINT("fail to read: invaild file descriptor %d\n", fd);
        return -1;
    }
    // not opening
    if ((!curr_task()->fd_array.entries[fd].flags))
    {
        // PRINT("%x, %d, %d", curr_task(), fd, nbytes);
        PRINT("fail to read: file is not open\n");
        return -1;
    }
    // update position field included in specfic read functions
    return curr_task()->fd_array.entries[fd].op_tbl_ptr->read(fd, buf, nbytes);
}

/*
 * int32_t file_sys_write(int32_t fd, const void *buf, int32_t nbytes)
 * try to write data from file to bufcurr_task()-
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
    if (buf == NULL)
    {
        printf("fail to write: get NULL buf\n");
        return -1;
    }
    if (nbytes < 0)
    {
        printf("fail to write: invaild nbytes, %d\n", nbytes);
        return -1;
    }
    if (fd < 0 || fd >= MAX_NUM_OPEN)
    {
        PRINT("fail to write: invaild file descriptor %d\n", fd);
        return -1;
    }
    // not opening
    if (!curr_task()->fd_array.entries[fd].flags)
    {
        PRINT("fail to write: file is not open\n");
        return -1;
    }
    return curr_task()->fd_array.entries[fd].op_tbl_ptr->write(fd, buf, nbytes);
}

/*
 * int32_t file_load(dentry_t *file_dentry, uint8_t *vir_addr)
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
    uint32_t file_size = inodes[inode].length;
    uint32_t size_uncopied = MIN((file_size - offset), length); // size to copy
    uint32_t dt_blk_idx_in_inode = offset / BLOCK_SIZE;         // index of data block in inode, init as the starting block
    uint32_t local_offset = offset % BLOCK_SIZE;                // offset in the data block
    int32_t size_single_copy;                                   // copied size
    uint32_t data_block;                                        // value of data block in inode

    // copy content in the starting block
    size_single_copy = MIN((BLOCK_SIZE - local_offset), size_uncopied);
    data_block = inodes[inode].data_block_num[dt_blk_idx_in_inode];
    if (ADDR_or_ID(data_block))
    { // addr
        memcpy(buf, (const uint8_t *)data_block + local_offset, size_single_copy);
    }
    else
    {
        memcpy(buf, (const uint8_t *)&data_blocks[data_block] + local_offset, size_single_copy);
    }
    size_uncopied -= size_single_copy;
    buf += size_single_copy;
    dt_blk_idx_in_inode++;
    if (size_uncopied == 0)
    {
        return size_single_copy;
    }
    // copy remaining content
    while (size_uncopied > 0)
    {
        data_block = inodes[inode].data_block_num[dt_blk_idx_in_inode];
        size_single_copy = MIN(BLOCK_SIZE, size_uncopied);
        if (ADDR_or_ID(data_block))
        { // addr
            memcpy(buf, (const uint8_t *)data_block, size_single_copy);
        }
        else
        {
            memcpy(buf, (const uint8_t *)&data_blocks[data_block], size_single_copy);
        }
        size_uncopied -= size_single_copy;
        buf += size_single_copy;
        dt_blk_idx_in_inode++;
    }
    // return copied size
    return MIN((file_size - offset), length);
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
    int32_t copy_size; // the size of copied data
    file_array_entry_t *file = &curr_task()->fd_array.entries[fd];
    uint32_t offset = file->file_position; // current position in file

    // Place the data into buffer
    copy_size = read_data(curr_task()->fd_array.entries[fd].inode, offset, (uint8_t *)buf, nbytes);

    if (copy_size > 0)
    {
        file->file_position += copy_size;
        // // if reach the end, restart
        // if (file->file_position == inodes[file->inode].length)
        // {
        //     file->file_position = 0;
        // }
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
/*

int32_t file_write(int32_t fd, const void *buf, int32_t nbytes)
{
    inode_t *inode = &inodes[curr_task()->fd_array.entries[fd].inode];
    uint32_t new_file_len = nbytes;
    uint32_t file_len = inode->length;
    uint32_t file_size_align = ((file_len + BLOCK_SIZE - 1) >> 12) << 12; // size rewrote, should be file_len align to 4KB
    uint32_t diff_size;
    uint32_t data_block;              // value of data block in inode
    uint32_t dt_blk_idx_in_inode = 0; // index of data block in inode
    if (file_size_align >= nbytes)
    {
        diff_size = file_size_align - nbytes; // size to clear
        while (nbytes >= BLOCK_SIZE)
        {
            data_block = inode->data_block_num[dt_blk_idx_in_inode];
            if (ADDR_or_ID(data_block))
            {
                memcpy((uint8_t *)data_block, (const uint8_t *)buf, BLOCK_SIZE);
            }
            else
            {
                memcpy((uint8_t *)&data_blocks[data_block], (const uint8_t *)buf, BLOCK_SIZE);
            }
            nbytes -= BLOCK_SIZE;
            buf += BLOCK_SIZE;
            dt_blk_idx_in_inode++;
        }
        if (nbytes > 0)
        {
            data_block = inode->data_block_num[dt_blk_idx_in_inode];
            if (ADDR_or_ID(data_block))
            {
                memcpy((uint8_t *)data_block, (const uint8_t *)buf, nbytes);
                memset((uint8_t *)(data_block + nbytes), 0, BLOCK_SIZE - nbytes);
            }
            else
            {
                memcpy((uint8_t *)&data_blocks[data_block], (const uint8_t *)buf, nbytes);
                memset((uint8_t *)&data_blocks[data_block] + nbytes, 0, BLOCK_SIZE - nbytes);
            }
            nbytes -= nbytes;
            buf += nbytes;
        }
    }
    else // will alloc
    {
        diff_size = nbytes - file_size_align; // size to alloc
        while (file_size_align >= BLOCK_SIZE)
        {
            data_block = inode->data_block_num[dt_blk_idx_in_inode];
            if (ADDR_or_ID(data_block))
            {
                memcpy((uint8_t *)data_block, (const uint8_t *)buf, BLOCK_SIZE);
            }
            else
            {
                memcpy((uint8_t *)&data_blocks[data_block], (const uint8_t *)buf, BLOCK_SIZE);
            }
            file_size_align -= BLOCK_SIZE;
            buf += BLOCK_SIZE;
            dt_blk_idx_in_inode++;
        }
        while (diff_size >= BLOCK_SIZE)
        {
            data_block = (uint32_t)bd_alloc(0); // alloc 1 pages i.e., 1 datablock
            if (data_block == 0)
            {
                inode->length = new_file_len - diff_size;
                PRINT("finish writing: file length: %d B, %d B unwriting\n", inode->length, diff_size);
                return inode->length;
            }
            memcpy((uint8_t *)data_block, (const uint8_t *)buf, BLOCK_SIZE);
            diff_size -= BLOCK_SIZE;
            buf += BLOCK_SIZE;
        }
        if (diff_size > 0)
        {
            data_block = (uint32_t)bd_alloc(0); // alloc 1 pages i.e., 1 datablock
            if (data_block == 0)
            {
                inode->length = new_file_len - diff_size;
                PRINT("finish writing: file length: %d B, %d B unwriting\n", inode->length, diff_size);
                return inode->length;
            }
            memcpy((uint8_t *)data_block, (const uint8_t *)buf, diff_size);
            diff_size -= diff_size;
            buf += diff_size;
        }
    }
    inode->length = new_file_len;
    return new_file_len;
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
        // PRINT("\nread nothing in directory. reach to the end\n");
        return 0;
    }
    // copy
    strncpy((int8_t *)buf, (const int8_t *)(boot_block->dentries[pst].file_name), copy_size);
    curr_task()->fd_array.entries[fd].file_position++;
    return copy_size;
}

/*
 * int32_t dir_write(int32_t fd, const void *buf, int32_t nbytes)
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
    if (boot_block->dir_count >= NUM_DIR_ENTRY)
    {
        PRINT("fail to write . reach max num of file\n");
        return -1;
    }
    int32_t copy_size = MIN(nbytes, MAX_LEN_FILE_NAME); // size to copy
    int32_t index = boot_block->dir_count;
    boot_block->dir_count++;
    strncpy((int8_t *)(boot_block->dentries[index].file_name), (const int8_t *)buf, copy_size);
    boot_block->dentries[index].file_type = 2;
    boot_block->dentries[index].inode_num = index;
    // memset((uint8_t *)&inodes[index], -1, SIZE_4KB);
    inodes[index].length = 0;
    PRINT("create new file at %d\n", index);
    return copy_size;
}

// do not implement for now
// can refer to rtc ops implemented by Yuan
int32_t rtc_user_open(const uint8_t *filename)
{
    int32_t fd = find_unused_fd();
    // available fd, actually need not check
    if (fd != -1)
    {
        set_entry(&curr_task()->fd_array, fd, 0); // type is 1 for directory
    }
    // rtc_open(filename);
    return fd;
}
int32_t rtc_user_close(int32_t fd)
{
    return rtc_close(fd);
}
int32_t rtc_user_read(int32_t fd, void *buf, int32_t nbytes)
{
    return rtc_read(fd, buf, nbytes);
}
int32_t rtc_user_write(int32_t fd, const void *buf, int32_t nbytes)
{
    return rtc_write(fd, buf, nbytes);
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
