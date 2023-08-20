#include <mos/sys.h>
#include <mos/drv.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// ports
#define PS2_DATA 0x60
#define PS2_STATUS 0x64
#define PS2_COMMAND 0x64

// types
#define PS2_TYPE_INVALID 0x4
#define PS2_TYPE_MOUSE 0x00
#define PS2_TYPE_MOUSE_SCROLL 0x01
#define PS2_TYPE_MOUSE_5BTN 0x2
#define PS2_TYPE_KEYBOARD 0x3

// commands
#define PS2_CTRL_READ_CFG 0x20
#define PS2_CTRL_WRITE_CFG 0x60
#define PS2_CTRL_DISABLE_P2 0xA7
#define PS2_CTRL_ENABLE_P2 0xA8
#define PS2_CTRL_TEST_P2 0xA9
#define PS2_CTRL_TEST_P1 0xAB
#define PS2_CTRL_TEST_CTRL 0xAA
#define PS2_CTRL_ENABLE_P1 0xAE
#define PS2_CTRL_DISABLE_P1 0xAD
#define PS2_CTRL_READ_OUTPUT 0xD0
#define PS2_CTRL_WRITE_P2 0xD4
#define PS2_CTRL_SELF_TEST 0xAA
#define PS2_DEV_SET_DEFAULTS 0xF6
#define PS2_DEV_DISABLE_SCANNING 0xF5
#define PS2_DEV_IDENTIFY 0xF2
#define PS2_DEV_ENABLE_SCANNING 0xF4
#define PS2_DEV_RESET 0xFF

// constants
#define PS2_TIMEOUT_YIELDS 3

// macro functions
#define waitOutput()                                               \
    for (int i = 0; i < PS2_TIMEOUT_YIELDS && status() & 0b1; i++) \
    {                                                              \
        sys_yield();                                               \
    }
#define waitInput()                                                 \
    for (int i = 0; i < PS2_TIMEOUT_YIELDS && status() & 0b10; i++) \
    {                                                               \
        sys_yield();                                                \
    }
#define status() inb(PS2_STATUS)
#define output() inb(PS2_DATA)
#define write(data)           \
    {                         \
        waitInput();          \
        outb(PS2_DATA, data); \
    }
#define command(cmd)            \
    {                           \
        waitInput();            \
        outb(PS2_COMMAND, cmd); \
    }
#define port1Write(data) \
    {                    \
        write(data);     \
    }
#define port2Write(data)                      \
    {                                         \
        outb(PS2_COMMAND, PS2_CTRL_WRITE_P2); \
        write(data);                          \
    }
#define flush()       \
    {                 \
        waitOutput(); \
        output();     \
    }

// these will be implemented by i8042.c
extern bool port1Present;
extern uint8_t port1Type;

extern bool port2Present;
extern uint8_t port2Type;

extern drv_type_input_t *contextStruct;

// keyboard
void kbInit();
void kbHandle(uint8_t scancode);

// mouse
void mouseInit();
void mouseHandle(uint8_t scancode);