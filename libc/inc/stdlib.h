#pragma once
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdfix.h>
#include <stdnoreturn.h>

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0
#define RAND_MAX 32768

void abort();
void exit(int status);
int abs(int x);
long int labs(long int x);
int atoi(const char *str);
long atol(const char *str);
void *malloc(size_t s);
void free(void *ptr);
int max(int a, int b);
int min(int a, int b);