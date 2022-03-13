#include <utils.h>
#include <stivale2.h>

void *bootloaderGetTag(struct stivale2_struct *stivale2_struct, uint64_t id);
void bootloaderInit(struct stivale2_struct *stivale2_struct);

void bootloaderTermWrite(const char *str);