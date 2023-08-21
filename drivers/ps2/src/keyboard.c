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

bool isShifted, capsLocked;

// initialize the keyboard
void kbInit()
{
    isShifted = capsLocked = false;

    // write to the right port
    if (port1Type == PS2_TYPE_KEYBOARD)
    {
        port1Write(PS2_DEV_SET_DEFAULTS); // set default parameters
        flush();

        port1Write(PS2_KB_SET_SCANCODE_SET); // set scan code command
        flush();
        port1Write(2); // scan code set 1 (on osdev it's wrong?)
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

        port2Write(PS2_KB_SET_SCANCODE_SET); // set scan code command
        flush();
        port2Write(2); // scan code set 1 (on osdev it's wrong?)
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
    for (; i < 64; i++)
        if (!contextStruct->keys[i])
            break;

    // handle special keys
    switch (scancode)
    {
    case KEY_RELEASED(0x3A): // caps lock released
        capsLocked = !capsLocked;
        return;

    case 0x2A: // left shift
    case 0x36: // right shift
    {
        isShifted = true;
        return;
    }

    case KEY_RELEASED(0x2A): // left shift released
    case KEY_RELEASED(0x36): // right shift released
    {
        isShifted = false;
        return;
    }

    default:
        if (scancode > sizeof(scanCodeSet1)) // not in scan code set
            return;
    }

    // choose the right set for all cases of shift and caps
    if (capsLocked && !isShifted) // caps without shift -> capsed
        contextStruct->keys[i] = scanCodeSet1Shifted[scancode - 1];
    else if (capsLocked && isShifted) // caps with shift -> normal
        contextStruct->keys[i] = scanCodeSet1[scancode - 1];
    else if (isShifted) // shift -> shifted
        contextStruct->keys[i] = scanCodeSet1Shifted[scancode - 1];
    else // normal
        contextStruct->keys[i] = scanCodeSet1[scancode - 1];
}