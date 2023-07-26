#include <libcfg.h>
#include <string.h>
#include <stdlib.h>

// get start of contents of a marker
char *cfgGet(config_t *cfg, const char *name)
{
    char *start = cfg->buffer;
    char *end = cfg->buffer + cfg->length;

    while (start < end)
    {
        if (start + strlen(name) + strlen(" = ") >= end) // out of bounds
            return NULL;

        if (memcmp(start, name, strlen(name)) == 0 && memcmp(start + strlen(name), " = ", strlen(" = ")) == 0) // check for string "<name> = "
            return start + strlen(name) + strlen(" = ");                                                       // return everything after the equal

        start++;
    }

    return NULL;
}

// parse contents of a marker as a bool
bool cfgBool(config_t *cfg, const char *name)
{
    char *value = cfgGet(cfg, name);

    if (!value)
        return false;

    return *value == '1';
}

// parse contents of a marker as a string
char *cfgStr(config_t *cfg, const char *name)
{
    char *start = cfgGet(cfg, name);
    char *end = cfg->buffer + cfg->length;

    if (!start || *start != '\"') // not found or broken format
        return "?";

    start++; // skip quotes

    char *strStart = start;

    // find end of string
    while (start < end)
    {
        if (*start == '\"') // terminate string
        {
            *start = '\0';
            return strStart;
        }
        start++;
    }

    return "?";
}

// parse contents of a marker as an unsigned integer
uint32_t cfgUint(config_t *cfg, const char *name)
{
    char *start = cfgGet(cfg, name);
    char *end = cfg->buffer + cfg->length;

    if (!start)
        return 0;

    char *strStart = start;

    // find end of string
    while (start < end)
    {
        if (*start >= '0' && *start <= '9') // we want to find first invalid character (not a number)
        {
            start++;
            continue;
        }

        *start = '\0';
        return abs(atoi(strStart));
    }

    return 0;
}

// create a new context for a coresponding buffer
config_t cfgCreate(const char *buffer)
{
    config_t cfg;
    cfg.buffer = (char *)buffer;
    cfg.length = strlen(buffer);
    return cfg;
}