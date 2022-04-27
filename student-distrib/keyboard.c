/* keyboard.h - Defines used in interactions with the keyboard
 * controller
 * created: 3/20/2022
 * by: Haina Lou
 */

#include "keyboard.h"
#include "lib.h"
#include "i8259.h"
#include "task.h"
#include "paging.h"
#include "types.h"
#include "x86_desc.h"

/* keyboard buffer */
static uint8_t kb_buf[kb_bufsize];
/* flag used to decide when can copy */
static volatile uint8_t copy_flag;
static uint8_t char_num;
/* keycode flag*/
static uint8_t cap_on_flag, shift_on_flag, ctrl_on_flag, alt_on_flag;
// PT_t kernel_pt ;
/* three terminals */
terminal_t _terminal_dp[MAX_TERMINAL_NUM];
uint32_t cur_terminal_id;
terminal_t *curr_terminal;

extern void flush_TLB();   // defined in boot.S
extern PCB_t *curr_task(); // defined in boot.
/* no shift no capson character and numbers */
const char scancode_simple_lowcase[keynum] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', // left control
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,   // left shift, right shift
    0, 0, ' ', 0, '\f', '\f', '\f'                                  // caps_lock ~0x3D
};
const char scancode_shifton[keynum] = {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', // left control
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,   // left shift, right shift
    0, 0, ' ', 0, '\f', '\f', '\f'                                // caps_lock ~0x3A
};
const char scancode_capson[keynum] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', // left control
    0, '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', 0,   // left shift, right shift
    0, 0, ' ', 0,'\f', '\f', '\f'                                   // caps_lock ~0x3A
};
const char scancode_bothon[keynum] = {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':', '"', '~', // left control
    0, '|', 'z', 'x', 'c', 'v', 'b', 'n', 'm', '<', '>', '?', 0,   // left shift, right shift
    0, 0, ' ', 0, '\f', '\f', '\f'                                 // caps_lock ~0x3A
};

void scancode_output(uint8_t scancode);
void set_flag(uint8_t scancode);
void printkey_on_curr_terminal(uint8_t keystroke);
void printf_on_curr_terminal(int8_t* string);

/* keyboard init
 *
 * Inputs: void
 * Outputs: void
 * Side Effects: handle keyboard output to terminal when
 * interrupt occurs
 */
void keyboard_init(void)
{
    cap_on_flag = 0;
    shift_on_flag = 0;
    ctrl_on_flag = 0;
    alt_on_flag = 0;
    enable_irq(KEYBARD_IRQ);
}

/* Keyboard_handler()
 * called when IDT want to handle the interrupt
 *
 * Inputs: void
 * Outputs: void
 * Side Effects: handle keyboard output to terminal when
 * interrupt occurs
 */
void keyboard_handler(void)
{
    cli();
    uint8_t scancode;
    /* read scancode input from keyboard */
    scancode = inb(KEYBOARD_PORT);
    /* set flag */
    set_flag(scancode);
    /* handle scancode and print to terminal */
    if ((scancode < keynum) && (scancode_simple_lowcase[scancode] != 0))
    {
        scancode_output(scancode);
    }

    send_eoi(KEYBARD_IRQ);
    sti();
}

/* set_flag()
 * called when key pressed
 * Inputs: scancode -- the key we press
 * Outputs: void
 * Side Effects: handle keyboard scancode
 * if it is shift turn on the shift on mode
 * if caps on turn on the capslock on mode
 * and if release the keypress, clear it
 * interrupt occurs
 */
void set_flag(uint8_t scancode)
{
    switch (scancode)
    {
    case l_shift:
        shift_on_flag = 1; // turn on th shift on flag
        break;
    case l_shift_release:
        shift_on_flag = 0; // turn off the shift on flag if release
        break;
    case r_shift:
        shift_on_flag = 1;
        break;
    case r_shift_release:
        shift_on_flag = 0;
        break;
    case caps:
        if (cap_on_flag == 1)
        {                    // if the caps has on
            cap_on_flag = 0; // caps change back to 0
        }
        else
        {
            cap_on_flag = 1;
        }
        break;
    case control:
        ctrl_on_flag = 1;
        break;
    case control_release:
        ctrl_on_flag = 0;
        break;
    case ALT:
        alt_on_flag = 1;
        break;
    case ALT_RELEASE:
        alt_on_flag = 0;
        break;
    default:
        break;
    }
}

/* put_changebuf()
 *
 * Inputs: output_char -- the output char
 * Outputs: void
 * return: none
 */
void put_changebuf(uint8_t output_char)
{
    if (output_char == '\b')
    { // if backspace
        if (char_num != 0)
        {
            if (kb_buf[char_num - 1] == '\t')
            { // delete all space if \t
                ONTO_DISPLAY_WRAP(putc('\b'));
                ONTO_DISPLAY_WRAP(putc('\b'));
                ONTO_DISPLAY_WRAP(putc('\b'));
                ONTO_DISPLAY_WRAP(putc('\b'));
            }else{
                ONTO_DISPLAY_WRAP(putc('\b'));             
            } 
            kb_buf[char_num - 1] = 0; // reset to 0
            char_num--;               // number of characters in buffer decrement         
        }
    }
    else
    {
        if (char_num < kb_bufsize - 1) //maximum char = 127
        {
            ONTO_DISPLAY_WRAP(putc(output_char));
            char_num++;
            kb_buf[char_num - 1] = output_char;
        }
        else
        {
            kb_buf[kb_bufsize - 1] = '\n';  
        }
    }
}

/* scancode_output()
 *
 * Inputs: void
 * Outputs: voidf
 * Side Effects: output corresponding keycode to console
 * interrupt occurs
 */
void scancode_output(uint8_t scancode)
{
    uint8_t output_char;

    /* press Enter */
    if (scancode == ENTER && (copy_flag == 0))
    {
        /* clean all the numbers we count */
        if (char_num <= kb_bufsize-1)   //maximum char number are 127
        {
            kb_buf[char_num] = '\n';
        }
        else
        {
            kb_buf[kb_bufsize - 1] = '\n';
        }
        char_num = 0;
        ONTO_DISPLAY_WRAP(putc('\n'));
        copy_flag = 1;
    }

    /* if scancode is in press range */
    if (scancode <= keynum && (copy_flag == 0))
    {
        /* find corresponding keycode */
        /* press ctrl+ l */
        if (ctrl_on_flag && (scancode == L))
        {
            clear();
            printf("%s", kb_buf);     // print buffer value after clear screen
        }
        else if (alt_on_flag)
        {
            switch (scancode)
            {
                case F1:
                    terminal_switch(&(_terminal_dp[0]));
                    break;
                case F2:
                    terminal_switch(&(_terminal_dp[1]));
                    break;
                case F3:
                    terminal_switch(&(_terminal_dp[2]));
                    break;
                default:break;
            }
        }
        else if (scancode_simple_lowcase[scancode] == '\f')
        {
            /* do nothing */
        }
        /* shift and capslock all on */
        else if (shift_on_flag && cap_on_flag)
        {
            output_char = scancode_bothon[scancode];
            put_changebuf(output_char); // show char and change keyboard buffer
        }
        /* only shift on */
        else if (shift_on_flag)
        {
            output_char = scancode_shifton[scancode];
            put_changebuf(output_char); // change the keyboard buffer
            /* only capslock on */
        }
        else if (cap_on_flag)
        {
            output_char = scancode_capson[scancode];
            put_changebuf(output_char); // change the keyboard buffer
        }
        else
        {
            output_char = scancode_simple_lowcase[scancode];
            put_changebuf(output_char); // change the keyboard buffer
        }
    }
}
/* terminal_init()
 *
 * Inputs: void
 * Outputs: void
 * Side Effects: init terminal
 * initliza the copy flag to 0 and
 * enable cursor and clear keyboard buffer
 *
 */
int32_t terminal_init()
{
    int j;
    copy_flag = 0;
    /* 25 is the number of rows in the terminal */
    enable_cursor(0, 25);
    /* clear all the keyboard buffer*/
    // for (i = 0; i < kb_bufsize; i++)
    // {
    //     kb_buf[i] = 0;
    // }
    
    for (j = 0; j < MAX_TERMINAL_NUM; j++)
    {
        _terminal_dp[j].character_num = 0;
        _terminal_dp[j].cursor_x = 0;
        _terminal_dp[j].cursor_y = 0;
        _terminal_dp[j].terminal_id = j+1;
        _terminal_dp[j].character_num = 0;
        _terminal_dp[j].num_task = 0;
        memset((void*)_terminal_dp[j].keyboard_buf,0,(uint32_t)kb_bufsize); 
    }
    _terminal_dp[0].page_addr = TERM1_ADDR;
    _terminal_dp[1].page_addr = TERM2_ADDR;
    _terminal_dp[2].page_addr = TERM3_ADDR;

    cur_terminal_id = 1;
    curr_terminal = &_terminal_dp[0];
    memcpy((void*)kb_buf,_terminal_dp[0].keyboard_buf,kb_bufsize);
    _terminal_dp[0].character_num = 0;
    screen_x = _terminal_dp[0].cursor_x;
    screen_y = _terminal_dp[0].cursor_y;
    char_num = _terminal_dp[0].character_num;

    /* MAPPING */
    // PDE_4KB_t *physical_vid_pde = (PDE_4KB_t *)(&page_directory.pde[0]);
    // set_PDE_4KB(physical_vid_pde, (uint32_t)(&kernel_pt), 1, 0, 1);
    page_directory.pde[1] = 0x00400083;
    set_PTE_4KB((PTE_4KB_t *)(&page_table.pte[VIDEO_MEM_INDEX-1]),VIDEO_MEM_INDEX*SIZE_4KB, 1, 0, 1);
    set_PTE_4KB((PTE_4KB_t *)(&page_table.pte[VIDEO_MEM_INDEX]),VIDEO_MEM_INDEX*SIZE_4KB, 1, 0, 1);
    set_PTE_4KB((PTE_4KB_t *)(&page_table.pte[VIDEO_MEM_INDEX+1]),TERM1_ADDR, 1, 0, 1);  
    set_PTE_4KB((PTE_4KB_t *)(&page_table.pte[VIDEO_MEM_INDEX+2]), TERM2_ADDR, 1, 0, 1);
    set_PTE_4KB((PTE_4KB_t *)(&page_table.pte[VIDEO_MEM_INDEX+3]), TERM3_ADDR, 1, 0, 1);
    flush_TLB();
    
    return 0;
}

/* terminal_open()
 *
 * Inputs: void
 * Outputs: void
 * Side Effects: open terminal
 *
 */
int32_t terminal_open(const uint8_t *filename)
{
    return 0;
}

/* terminal_close()
 *
 * Inputs: void
 * Outputs: void
 * Side Effects: close terminal
 *
 */
int32_t terminal_close(int32_t fd)
{
    return 0;
}

/* terminal_read()
 *
 * Inputs: fd
 * buf--buffer we store the input of keyboard
 * nbytes-- the number of bytes we want to read
 * Outputs: void
 * Side Effects: read keyboard buf and copy it
 * into buf, the maximum bytes in the buf is 128
 * bytes
 *
 */
int32_t terminal_read(int32_t fd, void *buf, int32_t nbytes)
{
    int i;
    int32_t copied; // number has copied
    uint8_t *to;    // copy to
    uint8_t *from;  // copy from
    sti();          // interrupt gate has set IF to 0, we need set 1 back
    while (copy_flag == 0)
    {
    } // read function waiting
    cli();
    to = buf;
    from = kb_buf;
    if ((NULL == buf) || (NULL == kb_buf)||(nbytes < 0))
        return -1;
    if (fd != 0)
        return -1; // if fd is not right

    /* the read bytes cannot be more than maximum buffer size */
    if (nbytes > kb_bufsize)
    { // if the nbytes write larger than bufsize
        nbytes = kb_bufsize;
    }

    copied = 0;
    while ('\n' != *from)
    { // while has not read the \n
        *to++ = *from++;
        if (++copied == nbytes)
        {
            /* clear kb_board buffer value */
            for (i = 0; i < kb_bufsize; i++)
            {
                kb_buf[i] = 0;
            }
            copy_flag = 0;
            return nbytes;
        }
    }

    /* add NULL to the bytes we not require */
    while (nbytes > copied)
    {
        *to++ = '\0';
        copied++;
    }
    /* clear kb_board buffer value */
    for (i = 0; i < kb_bufsize; i++)
    {
        kb_buf[i] = 0;
    }
    copy_flag = 0;
    sti();
    return copied;
}

/* terminal_write()
 *
 * Inputs: fd
 * buf--buffer we store the input of keyboard
 * nbytes-- the number of bytes we want to write
 * Outputs: void
 * Side Effects: write from keyboard buf
 *
 *
 */
int32_t terminal_write(int32_t fd, const void *buf, int32_t nbytes)
{
    cli();
    int i;
    uint8_t output_char;
    if ((NULL == buf)||(nbytes < 0))
        return -1; // if buf is null
    if (fd != 1)
        return -1; // if fd is not right number

    for (i = 0; i < nbytes; i++)
    {
        output_char = ((uint8_t *)buf)[i];
        putc(output_char); // put all the character out
    }
    sti();
    // printf("\nterminal_write, return %d\n",nbytes);
    return nbytes;
}


/*########################## FOR CP5 #################################*/

/* 
 * printkey_on_curr_termianl()
 *  DESCRIPTION: put one key on the current terminal we are looking at
 *  INPUTS: none
 *  OUTPUTS: put the cur_terminal
 *  RETURN VALUE: none
*/
void printkey_on_curr_terminal(uint8_t keystroke){
    video_mem_map_linear();
    putc(keystroke);
    update_cursor(screen_x, screen_y);
    video_mem_map_switch();
}


/* 
 * printf_on_curr_terminal()
 *  DESCRIPTION: put a string on the current terminal we are looking at
 *                 
 *  INPUTS: none
 *  OUTPUTS: put a string on the current terminal we are looking at   
 *  RETURN VALUE: none
*/
void printf_on_curr_terminal(int8_t* string){
    video_mem_map_linear();
    printf(string);
    update_cursor(screen_x, screen_y);
    video_mem_map_switch();
}

/* video_mem_map()
 * description: used for schedule, called in switch terminal
 * map the virtual video Memory to correct video page
 * Inputs: terminal_t * terminal_next -- next terminal information
 * Outputs: 0 success, -1 failure
 * Side Effects: 
 * if now scheduled running terminal is in current terminal -- direct mapping
 * if now scheduled running terminal is not in current terminal -- remapping
 *
 */
int32_t video_mem_map_linear()
{    

    set_PTE_4KB((PTE_4KB_t *)(&page_table.pte[VIDEO_MEM_INDEX]),VIDEO_MEM_INDEX*SIZE_4KB, 1, 0, 1);
    flush_TLB();
    return 0;
}

/* video_mem_map()
 * description: used for schedule, called in switch terminal
 * map the virtual video Memory to correct video page
 * Inputs: terminal_t * terminal_next -- next terminal information
 * Outputs: 0 success, -1 failure
 * Side Effects: 
 * if now scheduled running terminal is in current terminal -- direct mapping
 * if now scheduled running terminal is not in current terminal -- remapping
 *
 */
int32_t video_mem_map_switch()
{    

    if (curr_task()->terminal->terminal_id == cur_terminal_id)
    {
        set_PTE_4KB((PTE_4KB_t *)(&page_table.pte[VIDEO_MEM_INDEX]),VIDEO_MEM_INDEX*SIZE_4KB, 1, 0, 1);
    }
    else
    {
        uint32_t term_id = curr_task()->terminal->terminal_id;
        set_PTE_4KB((PTE_4KB_t *)(&page_table.pte[VIDEO_MEM_INDEX]),(VIDEO_MEM_INDEX+term_id)*SIZE_4KB, 1, 0, 1);
    }
    flush_TLB();
    return 0;
}

/* terminal_switch()
 * description: call when press ALT + Function key
 * switch correct terminal content to video memory
 * Inputs: 
 * terminal_next -- next terminal shown on the screen
 * Outputs: 0 success, -1 failure
 * Side Effects: 
 * copy current terminal content to video page buffer
 * copying next video page buffer content to video memory 
 *
 */
int32_t terminal_switch(terminal_t *terminal_next)
{
    send_eoi(KEYBARD_IRQ);
    if (terminal_next->terminal_id == cur_terminal_id)
    {
        printf("Still in terminal <%d>",cur_terminal_id);
        return 0; 
    }
    
    terminal_t *pre_terminal = &(_terminal_dp[cur_terminal_id-1]);
    

    /* set current terminal structure used by task.c */
    curr_terminal = terminal_next;
    cur_terminal_id = terminal_next->terminal_id;
    pre_terminal->cursor_x = screen_x;
    pre_terminal->cursor_y = screen_y;
    pre_terminal->character_num = char_num;

    cli();
    /* mapping */
    video_mem_map_linear();
    /* copy current terminal content to video page buffer */
    memcpy((void*)pre_terminal->page_addr,(void*)VIDEO_MEM_ADDR,(uint32_t)VIDEO_MEM_SIZE);   
    /* copying next video page buffer content to video memory  */
    memcpy((void*)VIDEO_MEM_ADDR, (void*)terminal_next->page_addr,(uint32_t)VIDEO_MEM_SIZE);
    video_mem_map_switch();

    update_cursor(terminal_next->cursor_x,terminal_next->cursor_y);
    screen_x = terminal_next->cursor_x;
    screen_y = terminal_next->cursor_y;
    char_num = terminal_next->character_num;
    /* change buffer content */
    /* change buffer content */
    memcpy((void*)pre_terminal->keyboard_buf,kb_buf,kb_bufsize);
    memcpy((void*)kb_buf,terminal_next->keyboard_buf,kb_bufsize);
    

    sti();
    

    
    
    return 0;
}



