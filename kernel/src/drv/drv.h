#pragma once
#include <misc/utils.h>

#define DRV_MAX_CONTEXTS 32

#define DRV_TYPE_FB 1
#define DRV_TYPE_INPUT 2

pstruct
{
    uint32_t pid; // driver pid
    void *base;   // base address
    uint32_t currentXres, currentYres;
    uint32_t requestedXres, requestedYres;
}
drv_context_fb_t;

void drvInit();
void drvExit(uint32_t drv);
void *drvRegister(uint32_t drv, uint32_t type);
void drvUpdateReference(uint32_t type, void *context);
void drvSocketCreate(uint32_t drv, uint32_t sockID);
void drvQuerySockets(uint32_t drv, uint32_t *sockets);
void drvQueryContexts(uint32_t type, void **contexts);
void *drvQueryActive(uint32_t type);