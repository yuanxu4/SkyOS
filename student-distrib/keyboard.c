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
static uint8_t cap_on_flag, l_shift_on_flag, r_shift_on_flag, l_ctrl_on_flag, r_ctrl_on_flag;
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
    l_shift_on_flag = 0;
    r_shift_on_flag = 0;
    l_ctrl_on_flag = 0;
    r_ctrl_on_flag = 0;
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
    scancode_output(scancode);

    send_eoi(KEYBARD_IRQ);
}

void set_flag(uint8_t scancode)
{
    switch (scancode)
    {
    case l_shift:
        l_shift_on_flag = 1;
        break;
    case l_shift_release:
        l_shift_on_flag = 0;
        break;
    case r_shift:
        r_shift_on_flag = 1;
        break;
    case r_shift_release:
        r_shift_on_flag = 0;
        break;
    case caps:
        if (cap_on_flag == 1)
        {
            cap_on_flag = 0;
        }
        else
        {
            cap_on_flag = 1;
        }
        break;
    case l_control:
        l_ctrl_on_flag = 1;
        break;
    case l_control_release:
        l_ctrl_on_flag = 0;
        break;
    default:
        break;
    }
}

/* scancode_output()
 *
 * Inputs: void
 * Outputs: void
 * Side Effects: output corresponding keycode to console
 * interrupt occurs
 */
void scancode_output(uint8_t scancode)
{
    uint8_t output_char;

    /* press Enter */
    if (scancode == ENTER && (copy_flag == 0))
    {
        char_num = 0;
        copy_flag = 1;
        kb_buf[kb_bufsize - 1] = '\n';
    }

    /* if scancode is in press range */
    if (scancode <= keynum && (copy_flag == 0))
    {
        /* find corresponding keycode */
        if (l_ctrl_on_flag && (scancode == L))
        {
            clear();
        }
        else if ((l_shift_on_flag || r_shift_on_flag) && (cap_on_flag))
        {
            output_char = scancode_bothon[scancode];
            putc(output_char);
            if (char_num < kb_bufsize)
            {
                kb_buf[char_num - 1] = output_char;
                char_num++;
            }
            else
            {
                kb_buf[kb_bufsize - 1] = '\n';
            }
        }
        else if (l_shift_on_flag || r_shift_on_flag)
        {
            output_char = scancode_shifton[scancode];
            putc(output_char);
            if (char_num < kb_bufsize)
            {
                kb_buf[char_num - 1] = output_char;
                char_num++;
            }
            else
            {
                kb_buf[kb_bufsize - 1] = '\n';
            }
        }
        else if (cap_on_flag)
        {
            output_char = scancode_capson[scancode];
            putc(output_char);
            if (char_num < kb_bufsize)
            {
                kb_buf[char_num - 1] = output_char;
                char_num++;
            }
            else
            {
                kb_buf[kb_bufsize - 1] = '\n';
            }
        }
        else
        {
            output_char = scancode_simple_lowcase[scancode];
            putc(output_char);
            if (char_num < kb_bufsize)
            {
                kb_buf[char_num - 1] = output_char;
                char_num++;
            }
            else
            {
                kb_buf[kb_bufsize - 1] = '\n';
            }
        }
    }
}

int32_t terminal_init()
{
    int i;
    copy_flag = 0;
    /* 25 is the number of rows in the terminal */
    enable_cursor(0, 25);
    for (i = 0; i < kb_bufsize; i++)
    {
        kb_buf[i] = 0;
    }
    return 0;
}

int32_t terminal_open(const uint8_t *filename)
{
    return 0;
}

int32_t terminal_close(int32_t fd)
{
    return 0;
}

int32_t terminal_read(int32_t fd, void *buf, int32_t nbytes)
{
    // printf("terminal_read");
    int i;
    int32_t copied;
    uint8_t *to;
    uint8_t *from;
    while (copy_flag == 0)
    {
    }
    to = buf;
    from = kb_buf;
    if ((NULL == buf) || (NULL == kb_buf))
        return -1;

    copied = 0;
    while ('\n' != *from)
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
    return copied;
}

int32_t terminal_write(int32_t fd, const void *buf, int32_t nbytes)
{

    int i;
    uint8_t output_char;
    if (NULL == buf)
        return -1;

    for (i = 0; i < nbytes; i++)
    {
        output_char = ((uint8_t *)buf)[i];
        putc(output_char);
    }
    // printf("\nterminal_write, return %d\n",nbytes);
    return nbytes;
}
