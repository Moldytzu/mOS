#pragma once

// page 318 of https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1256.pdf

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0
#define RAND_MAX 32768

#define NULL ((void *)0)

typedef unsigned long size_t;

void abort();
void exit(int status);
int abs(int x);
long int labs(long int x);
int atoi(const char *str);
long atol(const char *str);
double atof(const char *nptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
void *malloc(unsigned long s);
void free(void *ptr);
int max(int a, int b);
int min(int a, int b);
int system(const char *string);
void mkdir(const char *n, int attr);