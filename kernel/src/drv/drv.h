#pragma once
#include <misc/utils.h>

#define DRV_MAX_CONTEXTS 32

#define DRV_TYPE_FB 1
#define DRV_TYPE_INPUT 2

#define MOUSE_BUTTON_LEFT 0
#define MOUSE_BUTTON_MIDDLE 1
#define MOUSE_BUTTON_RIGHT 2

typedef struct
{
    uint64_t pid;
    uint8_t keys[64];        // key buffers
    int mouseX, mouseY;      // mouse relative coordonates
    uint8_t mouseButtons[5]; // mouse button states
} drv_context_input_t;

typedef struct
{
    uint64_t pid; // driver pid
    void *base;   // base address
    uint32_t currentXres, currentYres;
    uint32_t requestedXres, requestedYres;
} drv_context_fb_t;

typedef struct
{
    char friendlyName[128];
} drv_metadata_section_t;

void drvInit();
void drvExit(uint32_t drv);
void *drvRegister(uint32_t drv, uint32_t type);
void drvUpdateReference(uint32_t type, void *context);
void drvSocketCreate(uint32_t drv, uint32_t sockID);
void drvQuerySockets(uint32_t drv, uint32_t *sockets);
void drvQueryContexts(uint32_t type, void **contexts, uint32_t *size);
void *drvQueryActive(uint32_t type);