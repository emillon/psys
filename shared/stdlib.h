#ifndef STDLIB_H_
#define STDLIB_H_

#ifndef NULL
#define NULL ((void*)0)
#endif

long int strtol(const char *nptr, char **endptr, int base);
unsigned long int strtoul(const char *nptr, char **endptr, int base);

#endif /*STDLIB_H_*/
