#pragma once

#define SYS_STDIN 1

#define PHYSICAL(virtual) vmmGetPhys(schedulerGetCurrent()->pageTable,(void *)(virtual))

#include <utils.h>
#include <sys/exit.h>
#include <sys/write.h>
#include <sys/read.h>
#include <sys/input.h>
#include <sys/display.h>
#include <sys/exec.h>