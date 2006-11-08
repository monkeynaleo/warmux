#ifndef DEBUG_H
#define DEBUG_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#ifdef DEBUG
#define DPRINT(ARGS...) \
{ \
	printf("%10s,%3i : ",__FILE__,__LINE__); \
	printf(ARGS); \
	printf("\n"); \
}
#else
#define DPRINT(ARGS...)
#endif

#define TELL_ERROR \
{ \
	fprintf(stderr, "%10s,%3i : ERROR! %s",__FILE__,__LINE__, strerror(errno)); \
	fprintf(stderr, "\n"); \
	exit(1); \
}


#endif

