/* keyboard.h - Defines used in interactions with the keyboard
 * controller
 * created: 3/20/2022
 * by: Haina Lou
 */

#include "keyboard.h"
#include "lib.h"
#include "i8259.h"
/* keyboard buffer */
static uint8_t kb_buf[kb_bufsize];
/* flag used to decide when can copy */
static volatile uint8_t copy_flag;
static uint8_t char_num;
/* keycode flag*/
static uint8_t cap_on_flag, shift_on_flag, ctrl_on_flag;
/* no shift no capson character and numbers */
const char scancode_simple_lowcase[keynum] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', // left control
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,   // left shift, right shift
    0, 0, ' ', 0                                                    // caps_lock ~0x3A
};
const char scancode_shifton[keynum] = {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', // left control
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,   // left shift, right shift
    0, 0, ' ', 0                                                   // caps_lock ~0x3A
};
const char scancode_capson[keynum] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', // left control
    0, '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', 0,   // left shift, right shift
    0, 0, ' ', 0                                                    // caps_lock ~0x3A
};
const char scancode_bothon[keynum] = {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':', '"', '~', // left control
    0, '|', 'z', 'x', 'c', 'v', 'b', 'n', 'm', '<', '>', '?', 0,   // left shift, right shift
    0, 0, ' ', 0                                                   // caps_lock ~0x3A
};

void scancode_output(uint8_t scancode);
void set_flag(uint8_t scancode);
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
                putc('\b');
                putc('\b');
                putc('\b');
                putc('\b');
            }
            else
            {
                putc('\b');
            }
            kb_buf[char_num - 1] = 0; // reset to 0
            char_num--;               // number of characters in buffer decrement
        }
    }
    else
    {
        if (char_num < kb_bufsize - 1) // maximum char = 127
        {
            putc(output_char);
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
        if (char_num <= kb_bufsize - 1) // maximum char number are 127
        {
            kb_buf[char_num] = '\n';
        }
        else
        {
            kb_buf[kb_bufsize - 1] = '\n';
        }
        char_num = 0;
        putc('\n');
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
            printf("%s", kb_buf); // print buffer value after clear screen
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
    int i;
    copy_flag = 0;
    /* 25 is the number of rows in the terminal */
    enable_cursor(0, 25);
    /* clear all the keyboard buffer*/
    for (i = 0; i < kb_bufsize; i++)
    {
        kb_buf[i] = 0;
    }
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
    to = buf;
    from = kb_buf;
    if ((NULL == buf) || (NULL == kb_buf) || (nbytes < 0))
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
    if ('\n' == *from)
    {
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
    // /* add NULL to the bytes we not require */
    // while (nbytes > copied)
    // {
    //     *to++ = '\0';
    //     copied++;
    // }
    /* clear kb_board buffer value */
    for (i = 0; i < kb_bufsize; i++)
    {
        kb_buf[i] = 0;
    }
    copy_flag = 0;
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
    int i;
    uint8_t output_char;
    if ((NULL == buf) || (nbytes < 0))
        return -1; // if buf is null
    if (fd != 1)
        return -1; // if fd is not right number

    for (i = 0; i < nbytes; i++)
    {
        output_char = ((uint8_t *)buf)[i];
        putc(output_char); // put all the character out
    }
    // printf("\nterminal_write, return %d\n",nbytes);
    return nbytes;
}
