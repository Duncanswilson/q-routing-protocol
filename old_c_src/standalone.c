#define STANDALONE_EXTERN
#include "standalone.h"
#include <math.h>
#include <sys/time.h>

char *XNnewString(p)
     char *p;
{
  char *n = (char *) malloc(strlen(p)+1);
  strcpy(n,p);
  return n;
}

struct timeval barf_o_ghetti;

void init_randomizer(){
  int i;
  struct timeval *tp = &barf_o_ghetti;
  struct timezone *tzp = 0;

  gettimeofday(tp, tzp);
  random_seed48[0] = (tp->tv_usec) & 0177777;
  random_seed48[1] = getpid();
  random_seed48[2] = getppid();
  for (i=0;i<87;++i)		/* exercise out any startup transients */
    create(10);
}

void randomize(){init_randomizer();}

int create(maxnum){return nrand48(random_seed48)%Max(1,maxnum);}
int one_in(n){if (create(n)==0) return 1; else return 0;}
double fran() {return erand48(random_seed48);}
int with_prob(p)
     double p;
{
  if (fran()<=p) return 1; else return 0;
}

int create_n(n) {
  int i, v=0;

  for (i=0;i<n;++i) {
    v <<= 1;
    v |= create(2);
  }
  return v; }

double bran(lo, hi) double lo, hi; { return (hi-lo) * fran() + lo; }

double gaussian(mean,dev)
     double mean,dev;
{
  static int stashed_p = 0;
  static double stash;
  double v1,v2,r,fac;
  if (stashed_p) {
    stashed_p = 0;
    return stash*dev+mean;
  }
  do {
    v1 = bran(-1.0,1.0);
    v2 = bran(-1.0,1.0);
    r = v1*v1+v2*v2;
  } while (r>=1 || r==0);
  fac = sqrt(-2*log(r)/r);
  stashed_p = 1;
  stash = v1*fac;
  return (v2*fac)*dev+mean;
}

/* List processing support for XN */

void init_lisp(){}

List * cons(x,y)
{
  List *c = NEW(List);

  c->car = (List *) x;
  c->cdr = (List *) y;

  return(c); 
}

void free_list(l)
     List *l;
{
  List *t;
  while (l) {
    t = l;
    l = CDR(l);
    free(CAR(t));
    free(t);
  }
}

List * nlist(n,args)
     int n;
     List * args;
{
  List *l=NIL;

  for (;n>=0;--n) 
    PUSH((&args)[n],l);
  return l;
}

int list_length(l)
     List *l;
{
  if (l==NIL) return 0;
  else return 1+list_length(CDR(l));
}

List *nthcdr(n,list)
     register int n;
     register List *list;
{
  while (n-->0) list = CDR(list);
  return list;
}

List *nth(n,list)
     register int n;
     register List *list;
{
  return CAR(nthcdr(n,list));
}

List *nreverse(list)
     register List *list;
{
  register List *prev=NIL, *temp;

  while (list) {
    temp = CDR(list);
    CDR(list) = prev;
    prev = list;
    list = temp;
  }
  return prev;
}


List *copylist(list)
     register List *list;
{
  if (list)
    return cons(CAR(list), copylist(CDR(list)));
  else
    return NIL;
}

List *append(list1, list2)
     register List *list1, *list2;
{
  if (list1)
    return cons(CAR(list1), append(CDR(list1), list2));
  else
    return copylist(list2);
}
		
List *nconc(list1, list2)
     register List *list1, *list2;
{
  List *rlist = list1;
  if (list1 == NIL) return list2;
  while (CDR(list1)) list1 = CDR(list1);
  CDR(list1) = list2;
  return rlist;
}

List *memq(item, in_list)
     List *item, *in_list;
{
  for (; in_list; in_list = CDR(in_list))
    if (item == CAR(in_list)) return in_list;
  return NIL;
}

/* error.c - XN error handling performed here
 */

#include <varargs.h>
#include <stdio.h>

void XNfatal(va_alist)
     va_dcl
{
  char *form;
  va_list args;

  va_start(args);
  form = va_arg(args,char *);
  vsprintf(lowio_character_buffer,form,args);
  fprintf(stderr,"Fatal: %s",lowio_character_buffer);
  exit(3);
}

void XNwarning(va_alist)
     va_dcl
{
  char *form;
  va_list args;
  char buff[1000];

  va_start(args);
  form = va_arg(args,char *);
  vsprintf(buff,form,args);
  fprintf(stderr,"Warning: %s",buff);
}

#define fsleep(x) \
   { \
   struct timeval time; \
   time.tv_sec = 0; \
   time.tv_usec = x; \
   select(0,0,0,0,&time); \
   }

void millisecond_sleep(milliseconds)
{
  int sec = milliseconds/1000;
  int ms = milliseconds%1000;
  if (sec) second_sleep(sec);
  if (ms) fsleep(ms*1000);
}

void second_sleep(seconds){
  int i;
  for (i=0;i<10*seconds;++i) millisecond_sleep(100);
}


