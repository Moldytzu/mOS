#include <libcfg.h>
#include <string.h>
#include <stdlib.h>

char *cfgGet(config_t *cfg, const char *name)
{
    char *start = cfg;
    char *end = cfg + 4096;

    while (start < end)
    {
        if (memcmp(start, name, strlen(name)) != 0 || memcmp(start + strlen(name), " = ", strlen(" = ")) != 0) // check for the name then for the " = " suffix
        {
            start++;
            continue;
        }

        return start + strlen(name) + strlen(" = "); // return address of operand
    }

    return NULL;
}

bool cfgBool(config_t *cfg, const char *name)
{
    char *value = cfgGet(cfg, name);

    if (!value)
        return false;

    return *value == '1';
}

char *cfgStr(config_t *cfg, const char *name)
{
    char *start = cfgGet(cfg, name);
    char *end = cfg + 4096;

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

uint32_t cfgUint(config_t *cfg, const char *name)
{
    char *start = cfgGet(cfg, name);
    char *end = cfg + 4096;

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