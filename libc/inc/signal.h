#pragma once

// page 258 of https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1256.pdf

typedef int sig_atomic_t;

#define SIG_DFL 0
#define SIG_ERR 0
#define SIG_IGN 0

#define SIGABRT 0
#define SIGFPE 0
#define SIGILL 0
#define SIGINT 0
#define SIGSEV 0
#define SIGTERM 0

#define signal(ignore,ignore2)
#define raise(ignore)