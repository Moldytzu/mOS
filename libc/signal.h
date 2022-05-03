#pragma once

// not used internaly, kept for source code compatibilty

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