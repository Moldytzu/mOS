#pragma once

// page 318 of https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1256.pdf

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0
#define RAND_MAX 32768

#define NULL ((void *)0)

void abort();
void exit(int status);
int abs(int x);
long int labs(long int x);
int atoi(const char *str);
long atol(const char *str);
void *malloc(unsigned long s);
void free(void *ptr);
int max(int a, int b);
int min(int a, int b);