#include <libcfg.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

// get start of contents of a marker
char *cfgGet(config_t *cfg, const char *name)
{
    char *start = cfg->buffer;
    char *end = cfg->buffer + cfg->length;
    size_t nameLength = strlen(name);

    // we want to return first occurence in buffer of <name>=, thus we iterate over all bytes
    while (start < end)
    {
        char *contents = start + nameLength + 1;

        if (contents >= end) // out of bounds
            return NULL;

        if (memcmp(start, name, nameLength) == 0 && start[nameLength] == '=') // check for string "<name>="
            return contents;

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
    cfg.scratchBuffer = (char *)buffer;
    cfg.length = cfg.scratchLength = strlen(buffer);

    cfg.buffer = malloc(cfg.scratchLength); // allocate a new buffer
    assert(cfg.buffer != NULL);

    // parse all bytes of the original bufer

    // PASS 1: remove comments
    bool inComment = false;
    char *currentByte = cfg.buffer; // current byte in the new buffer

    for (int i = 0; i < cfg.scratchLength; i++)
    {
        char toCopy = cfg.scratchBuffer[i]; // the character we process

        // comments are one line only and start with #
        if (toCopy == '#')
            inComment = true;

        if (toCopy == '\n')
            inComment = false;

        if (!inComment)
            *currentByte++ = toCopy;
    }

    memset(cfg.scratchBuffer, 0, cfg.scratchLength);           // zero the scratch buffer
    memcpy(cfg.scratchBuffer, cfg.buffer, strlen(cfg.buffer)); // copy the buffer in the scratch buffer
    cfg.scratchLength = strlen(cfg.scratchBuffer);             // set new scratch length

    // PASS 2: remove spaces
    // we want to remove all spaces so we can have syntax like <marker name>    =                  <contents>
    // that is parsed perfectly the same way like <marker name>=contents
    memset(cfg.buffer, 0, cfg.length); // clean the buffer
    bool inMarker = false;             // is set when we process a marker's contents
    currentByte = cfg.buffer;          // current byte in the new buffer

    for (int i = 0; i < cfg.scratchLength; i++)
    {
        char toCopy = cfg.scratchBuffer[i]; // the character we process

        if (toCopy == '=') // we use = to mark a marker's contents
        {
            inMarker = true;

            while (cfg.scratchBuffer[i + 1] == ' ' && i + 1 < cfg.scratchLength) // skip all spaces after = until another character is reached while not going out of bounds
                i++;
        }

        if (toCopy == '\n' || toCopy == '#') // a new line or a new comment marks the end of a marker's contents
            inMarker = false;

        if (!inMarker && toCopy == ' ') // skip all spaces outside the markers' contents
        {
            cfg.length--;
            continue;
        }

        *currentByte++ = toCopy;
    }

    cfg.length = strlen(cfg.buffer); // calculate new length

    return cfg;
}