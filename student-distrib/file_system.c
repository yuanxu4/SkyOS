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
// #include "terminal.h"

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

#define MIN(a, b) (a > b ? b : a)

// File System Utilities
static boot_block_t *boot_block;
static inode_t *inodes;
static data_block_t *data_blocks;

// File System Abstractions
static jump_table_t op_table_total[NUM_FILE_TYPE];

static jump_table_t dir_op_table;
static jump_table_t file_op_table;
// static jump_table_t terminal_op_table;

static file_array_t file_array;

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
    int i; // loop counter

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
    op_table_total[3].write = terminal_write;
    op_table_total[3].close = terminal_close;

    // init stdin and stdout
    // stdin is a read-only file which corresponds to keyboard input.
    // terminal operations should be implemented in terminal.c by haina
    set_entry(0, 3);
    file_array.entries[0].op_tbl_ptr->write = NULL; // maybe change

    // stdout is a write-only file corresponding to terminal output.
    set_entry(1, 3);
    file_array.entries[0].op_tbl_ptr->read = NULL; // maybe change

    file_array.num_opening = 2;

    // set others as not used
    for (i = 2; i < MAX_NUM_OPEN; i++)
    {
        file_array.entries[i].flags = NOT_IN_USE;
    }

    return 0;
}

// help function to set entry in file array when init
void set_entry(int32_t index, int32_t file_type)
{
    file_array.entries[index].op_tbl_ptr = op_table_total + file_type;
    file_array.entries[index].inode = 0;
    file_array.entries[index].file_position = 0;
    file_array.entries[index].flags = IN_USE;
    return;
}

int32_t find_not_used_fd()
{
    int fd;
    for (fd = 2; fd < MAX_NUM_OPEN; fd++)
    {
        if (!file_array.entries[fd].flags)
        {
            return fd;
        }
    }
    return -1;
}

// opertions of file system, called when system call
/*
 * int32_t file_sys_open(const uint8_t *filename)
 * try to open a file in the file system
 * Inputs:  filename -- The name of the file to open
 * Outputs: None
 * Side Effects: none
 * return value: -1 for failure, the file descriptor for succuss
 */
int32_t file_sys_open(const uint8_t *filename)
{
    dentry_t *copied_dentry;
    int32_t fd = -1; // the returned file descriptor if succuss
    if (file_array.num_opening >= MAX_NUM_OPEN)
    {
        PRINT("fail to open %s. number of opening files is max.\n", filename);
        return -1;
    }
    // return != 0, fail to find the file
    if (read_dentry_by_name(filename, copied_dentry))
    {
        PRINT("fail to open %s. non-existent file.\n", filename);
        return -1;
    }

    switch (copied_dentry->file_type)
    {
    case 0: // RTC
        fd = rtc_user_open(filename);
        break;
    case 1: // directory
        fd = dir_open(filename);
        break;
    case 2: // regular file
        fd = file_open(filename);
        break;
    default:
        PRINT("fail to open %s. unknown file type\n", filename);
        return -1;
    }

    return fd;
}

/*
 * int32_t file_sys_close(int32_t fd)
 * try to close a file in the file system
 * Inputs:  fd -- The file descriptor of the file to close
 * Outputs: None
 * Side Effects: none
 * return value: 0 for success, -1 for failure
 */
int32_t file_sys_close(int32_t fd)
{
    // Check whether fd is valid
    if (fd < 2 || fd > MAX_NUM_OPEN)
    {
        PRINT("fail to close file. invaild file descriptor\n");
        return -1;
    }
    if (file_array.entries[fd].flags == NOT_IN_USE)
    {
        PRINT("fail to close file. unopen file");
        return -1;
    }
    if (file_array.entries[fd].op_tbl_ptr->close(fd))
    {
        return -1;
    }
    file_array.entries[fd].flags = NOT_IN_USE;
    file_array.num_opening--;
    return 0;
}

/*
 * int32_t file_sys_read(int32_t fd, void *buf, int32_t nbytes)
 * try to read data from file to buf
 * Inputs:  fd -- The file descriptor of the file to read
 *          buf -- The buffer to store data
 *          nbytes -- The number of bytes to read
 * Outputs: None
 * Side Effects: none
 * return value: the number of bytes read, 0 if offset reach the end of the file,
 *          -1 for failure
 *
 */
int32_t file_sys_read(int32_t fd, void *buf, int32_t nbytes)
{
    if (file_array.entries[fd].flags == NOT_IN_USE)
    {
        PRINT("fail to read. file is not open\n");
        return -1;
    }

    return file_array.entries[fd].op_tbl_ptr->read(fd, buf, nbytes);
}

/*
 * nt32_t file_sys_write(int32_t fd, const void *buf, int32_t nbytes)
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
    if (file_array.entries[fd].flags == NOT_IN_USE)
    {
        PRINT("fail to write. file is not open\n");
        return -1;
    }
    PRINT("fail to write. read only\n");
    return file_array.entries[fd].op_tbl_ptr->write(fd, buf, nbytes);
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
    int i; // loop counter

    // check the length of the file name
    if (strlen((const int8_t *)fname) > FILE_NAME_LENGTH)
    {
        return -1;
    }

    for (i = 0; i < boot_block->dir_count; i++)
    {
        // check names, 0 means the same, i.e. find
        if (!strncmp((const int8_t *)fname, (const int8_t *)boot_block->dentries[i].file_name, FILE_NAME_LENGTH))
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
    int i; // loop counter

    for (i = 0; i < boot_block->dir_count; i++)
    {
        // check index
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
    if ((inode >= boot_block->inode_count) || (offset > inodes[inode].length))
    {
        return -1;
    };

    uint32_t file_len = inodes[inode].length;
    if (offset == file_len)
    {
        return 0;
    }
    uint32_t copy_size = MIN((file_len - offset), length);
    uint32_t dt_blk_idx_in_inode = offset / BLOCK_SIZE;                          // index of data block in inode, init as the starting block
    uint32_t local_offset = offset % BLOCK_SIZE;                                 // offset in the data block
    uint32_t dt_blk_idx_abs = inodes[inode].data_block_num[dt_blk_idx_in_inode]; // index of data block absolutely
    int32_t copied;
    // may need to check dt_blk_idx_abs??
    for (copied = 0; copied < copy_size; copied++)
    {
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

    return 0;
}

// Operation of the regular file
/*
 * int32_t file_open(const uint8_t *filename)
 * try to open a regular file in the file system
 * Inputs:  filename -- The name of the file to open
 * Outputs: None
 * Side Effects: none
 * return value: -1 for failure, else the file descriptor
 */
int32_t file_open(const uint8_t *filename)
{
    dentry_t *file_dentry;
    // actually need not check
    if (read_dentry_by_name(filename, file_dentry))
    {
        return -1;
    }

    int32_t fd = find_not_used_fd();
    if (fd != -1)
    {
        set_entry(fd, 2);
        file_array.entries[fd].inode = file_dentry->inode_num;
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
 * Side Effects: none
 * return value: the number of bytes read, 0 if offset reach the end of the file,
 *          -1 for failure
 *
 */
int32_t file_read(int32_t fd, void *buf, int32_t nbytes)
{

    int32_t copy_size = 0;                                  // the size of copied data
    uint32_t offset = file_array.entries[fd].file_position; // current position in file

    // Place the data into buffer
    copy_size = read_data(file_array.entries[fd].inode, offset, buf, nbytes);

    if (copy_size > 0)
    {
        file_array.entries[fd].file_position += copy_size;
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
    return -1;
}

// Operation of the directory
/*
 * int32_t dir_open(const uint8_t *filename)
 * try to open a directory in the file system
 * Inputs:  filename -- The name of the file to open
 * Outputs: None
 * Side Effects: none
 * return value: -1 for failure, else the file descriptor
 */
int32_t dir_open(const uint8_t *filename)
{
    int32_t fd = find_not_used_fd();
    if (fd != -1)
    {
        set_entry(fd, 1);
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
 * Side Effects: none
 * return value: the number of bytes read, 0 if offset reach the end of the file,
 *          -1 for failure
 *
 */
int32_t dir_read(int32_t fd, void *buf, int32_t nbytes)
{
    uint32_t pst = file_array.entries[fd].file_position;
    int32_t copy_size = MIN(nbytes, FILE_NAME_LENGTH);
    if (pst >= boot_block->dir_count)
    {
        PRINT("read nothing in directory. reach to the end\n");
        return 0;
    }
    // Copy the file name into buf
    strncpy(buf, (int8_t *)(boot_block->dentries[pst].file_name), copy_size);
    file_array.entries[fd].file_position++;
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
    return -1;
}

// do not used in cp2
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
