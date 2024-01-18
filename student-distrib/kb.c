// implementation of keyboard interrupts

#include "kb.h"

#define SCANCODES 0x3B

uint8_t caps_flag = 0;
uint8_t shift_flag = 0;
uint8_t ctrl_flag = 0;
uint8_t alt_flag = 0;


uint8_t kb_buffer[TERMINAL_COUNT][BUF_SIZE];
uint32_t count[TERMINAL_COUNT] = {0, 0, 0};

// scancodes for keyboard
// referenced from: 
// https://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html
unsigned char scancode_dict[SCANCODES][2] = 
{
    {0, 0},  // not a valid character
    {0, 0},
    {'1', '!'}, 
    {'2', '@'}, 
    {'3', '#'}, 
    {'4', '$'},  
    {'5', '%'},
    {'6', '^'},
    {'7', '&'},
    {'8', '*'},
    {'9', '('},
    {'0', ')'},
    {'-', '_'},
    {'=', '+'},
    {BACKSPACE, BACKSPACE}, 
    {'\t', '\t'}, 
    {'q', 'Q'},
    {'w', 'W'},
    {'e', 'E'},
    {'r', 'R'},
    {'t', 'T'},
    {'y', 'Y'},
    {'u', 'U'},
    {'i', 'I'},
    {'o', 'O'},
    {'p', 'P'},
    {'[', '{'},
    {']', '}'},
    {'\n', '\n'},
    {CTRL, CTRL},
    {'a', 'A'},
    {'s', 'S'},
    {'d', 'D'},
    {'f', 'F'},
    {'g', 'G'},
    {'h', 'H'},
    {'j', 'J'}, 
    {'k', 'K'},
    {'l', 'L'},
    {';', ':'},
    {'\'', '"'},
    {'`', '~'},
    {LSHIFT, LSHIFT},
    {'\\', '|'},
    {'z', 'Z'},
    {'x', 'X'},
    {'c', 'C'},
    {'v', 'V'},
    {'b', 'B'},
    {'n', 'N'},
    {'m', 'M'},
    {',', '<'},
    {'.', '>'},
    {'/', '?'},
    {RSHIFT, RSHIFT},
    {KEYPAD_STAR, KEYPAD_STAR},
    {ALT, ALT},
    {' ', ' '},
    {CAPS, CAPS}
};

// initialization function for keyboard
// link keyboard to PIC
void keyboard_init(void){
    enable_irq(KEYBOARD_IRQ);
    clear_buffer();
    clear_screen();
}

// handles interrupt for keyboard
// interacts with linkage/wrapper function
// Inputs: none
// Outputs: none
// Effects: prints keyboard input to screen
void keyboard_irq_handler(void){
    // stop interrupts for other things while handling keyboard
    cli();

    // send end of interrupt to PIC
    send_eoi(KEYBOARD_IRQ);

    // retrieve scancode from keyboard input
    uint8_t scancode = inb(KEYBOARD_PORT_DATA);  

    // convert scancode into ascii value
    // if a special key is pressed
    // we must handle it accordingly
    // i.e. set the corresponding flag to the correct value
    uint8_t pressed;
    switch(scancode){
        case CAPS:
            caps_flag = (~caps_flag) & 0x1;
            sti();
            return;
        case LSHIFT:
            shift_flag = 1;
            sti();
            return;
        case RSHIFT:
            shift_flag = 1;
            sti();
            return;
        case LSHIFT_RELEASE:
            shift_flag = 0;
            sti();
            return;
        case RSHIFT_RELEASE:
            shift_flag = 0;
            sti();
            return;
        case CTRL:
            ctrl_flag = 1;
            sti();
            return;
        case CTRL_RELEASE:
            ctrl_flag = 0;
            sti();
            return;
        case ALT:
            alt_flag = 1;
            sti();
            return;
        case ALT_RELEASE:
            alt_flag = 0;
            sti();
            return;
        default:
            // set pressed value to proper ascii
            // if not a valid scancode, set to an incorrect value which won't be printed
            if(scancode < SCANCODES){
                pressed = scancode_dict[scancode][0];
            } else {
                pressed = 0;
            }
            break;
    }
    if(pressed >= 'a' && pressed <= 'z'){
        pressed = scancode_dict[scancode][shift_flag ^ caps_flag];
    } else {
        pressed = scancode_dict[scancode][shift_flag];
    }

    // make sure valid, printable value
    if((scancode >= SCANCODES || pressed == 0) && !((scancode == F1_CODE) || (scancode == F2_CODE) || (scancode == F3_CODE))){
        return;
    }
    // print ascii to screen
    if(pressed == BACKSPACE){
        if(count[current_terminal] != 0){
            if(kb_buffer[current_terminal][count[current_terminal] - 1] == '\t'){
                int i;
                for(i = 0; i < TAB_SIZE; i++){
                    del_c();
                }
            } else {
                del_c();
            }
            // pop the last element off of buffer
            kb_buffer[current_terminal][count[current_terminal] - 1] = 0;
            count[current_terminal]--;
        }
    } else if (ctrl_flag && ((pressed == 'l') || (pressed == 'L'))){
        clear_screen();
    } else if (ctrl_flag && ((pressed == 'c') || (pressed == 'C'))){
        halt(USER_HALT);
    } else if(pressed == '\n' || pressed == '\r'){
        if(count[current_terminal] < BUF_SIZE){
            putc(pressed);
            kb_buffer[current_terminal][count[current_terminal]] = pressed;
            count[current_terminal]++;
            read_flag = 1;
        }
    } else if(alt_flag && (scancode == F1_CODE)){
        open_terminal(0);
    } else if(alt_flag && (scancode == F2_CODE)){
        open_terminal(1);
    } else if(alt_flag && (scancode == F3_CODE)){
        open_terminal(2);
    } else{
        // dont allow new characters that aren't special if the buffer is full
        // one space needs to be reserved for ENTER
        if(count[current_terminal] < BUF_SIZE - 1){
            putc(pressed);
            kb_buffer[current_terminal][count[current_terminal]] = pressed;
            count[current_terminal]++;
        }
    }
    // allow interrupts now
    sti();
    return;
}

void clear_buffer(){
    int i;
    for (i = 0; i < count[current_terminal]; i++){
        kb_buffer[current_terminal][i] = 0;
    }
    count[current_terminal] = 0;
}
