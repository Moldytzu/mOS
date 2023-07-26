#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    char *buffer;
    unsigned long length;
} config_t;

char *cfgGet(config_t *cfg, const char *name);
bool cfgBool(config_t *cfg, const char *name);
char *cfgStr(config_t *cfg, const char *name);
uint32_t cfgUint(config_t *cfg, const char *name);
config_t cfgCreate(const char *buffer);