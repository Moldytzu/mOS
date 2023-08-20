#include <ps2.h>

// commands
#define PS2_KB_SET_SCANCODE_SET 0xF0
#define PS2_KB_SET_TYPEMATIC_RATE 0xF3

// todo: support scancode set 2

// translation table for the scan code set 1
static const char scanCodeSet1[] = "\e1234567890-=\b\tqwertyuiop[]\n\0asdfghjkl;'`\0\\zxcvbnm,./\0*\0 ";
static const char scanCodeSet1Shifted[] = "\e!@#$%^&*()_+\b\tQWERTYUIOP{}\n\0ASDFGHJKL:\"~\0|ZXCVBNM<>?\0*\0 ";

// macro functions
#define KEY_RELEASED(x) (x + 0x80)

bool isShifted;

// initialize the keyboard
void kbInit()
{
    isShifted = false;

    // write to the right port
    if (port1Type == PS2_TYPE_KEYBOARD)
    {
        port1Write(PS2_DEV_SET_DEFAULTS); // set default parameters

        flush(); // flush the buffer

        port1Write(PS2_KB_SET_SCANCODE_SET); // set scan code set 1
        for (int i = 0; i < 2; i++)          // wait for the keyboard to send 0xFA 0xFA
            flush();

        port1Write(PS2_KB_SET_TYPEMATIC_RATE); // set typematic rate
        flush();

        port1Write(0b00100000); // 30hz repeat rate and 500 ms delay for repeat
        flush();
    }
    else if (port2Type == PS2_TYPE_KEYBOARD)
    {
        port2Write(PS2_DEV_SET_DEFAULTS); // set default parameters

        flush(); // flush the buffer

        port2Write(PS2_KB_SET_SCANCODE_SET); // set scan code set 1
        for (int i = 0; i < 2; i++)          // wait for the keyboard to send 0xFA 0xFA
            flush();

        port2Write(PS2_KB_SET_TYPEMATIC_RATE); // set typematic rate
        flush();

        port2Write(0b00100000); // 30hz repeat rate and 500 ms delay for repeat
        flush();
    }
}

// keyboard scancode handler
void kbHandle(uint8_t scancode)
{
    if (scancode > 0xDF) // over what we expect
        return;

    // find the first empty in the buffer
    int i = 0;
    for (; i < 16; i++)
        if (!contextStruct->keys[i])
            break;

    if (i == 15) // filled buffer
        return;  // drop key

    if (scancode == KEY_RELEASED(0x3A)) // caps lock released
    {
        isShifted = !isShifted;
        return;
    }

    if (scancode == 0x2A || scancode == 0x36) // left+right shift
    {
        isShifted = true;
        return;
    }

    if (scancode == KEY_RELEASED(0x2A) || scancode == KEY_RELEASED(0x36)) // left+right shift released
    {
        isShifted = false;
        return;
    }

    if (scancode > sizeof(scanCodeSet1)) // not in scan code set
        return;

    if (isShifted)
        contextStruct->keys[i] = scanCodeSet1Shifted[scancode - 1]; // set the key at that index
    else
        contextStruct->keys[i] = scanCodeSet1[scancode - 1]; // set the key at that index
}