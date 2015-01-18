#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "qmalloc.h"

#include "cmdlib.h"


#define NoMem(x) \
   Error("Out of memory! %s failed for %i bytes!", \
      x,size)

void *Q_malloc(size_t size)
{
   void *r;
   r=malloc(size);
   if (!r)
      NoMem("malloc");
   return r;
}

void *Q_realloc(void *ptr,size_t size)
{
   void *r;
   r=realloc(ptr,size);
   if (!r)
      NoMem("realloc");
   return r;
}

void Q_free(void *ptr)
{
   if (!ptr)
      return;
   free(ptr);
}

void PrintMax(void)
{
}


#ifdef DJGPP
// this is probably DJGPP specific
// all utils need a larger stack than the default 256kb
unsigned int _stklen=1024*1024; // 1mb should work
#endif


#ifdef TRACK_MEM
/*
memory tracking and error reporting malloc, realloc, and free
*/

extern void end;   // DJGPP specific, this is the end of the 'static' memory


#define NoMem(x) \
   Error("Out of memory! %s failed for %i bytes! %i bytes already used!", \
      x,size,cur)


static int max=(int)&end;
static int cur=(int)&end;

#define Add(x) \
   cur+=(x); \
   if (cur>max) \
   { \
      max=cur; \
   }

#define Sub(x) cur-=(x);

void *Q_malloc(size_t size)
{
   void *r;

   r=malloc(size+4);
   if (!r)
      NoMem("malloc");

   *(int *)r=size;
   Add(size);

   return (r+4);
}

void *Q_realloc(void *ptr,size_t size)
{
   void *r;

   if (ptr)
   {
      ptr-=4;
      Sub(*(int *)ptr);
   }

   r=realloc(ptr,size+4);
   if (!r)
      NoMem("realloc");

   *(int *)r=size;
   Add(size);

   return (r+4);
}

void Q_free(void *ptr)
{
   if (!ptr)
      return;

   ptr-=4;
   Sub(*(int *)ptr);
   free(ptr);
}

void PrintMax(void)
{
   printf("%10i (%8.2fkb, %6.2fmb) bytes peak\n",
      max,(float)max/1024,(float)max/1024/1024);
   printf("%10i (%8.2fkb, %6.2fmb) bytes now\n",
      cur,(float)cur/1024,(float)cur/1024/1024);
}
#endif
