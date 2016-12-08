/* Header file for XN
 */

/* Prologue */

#ifndef _libXN_LOADED

#include <math.h>

/* List handling */

#define NIL 0
#define T 1

struct _cons {
  struct _cons *car;
  struct _cons *cdr;
};

typedef struct _cons List;

#define CAR(cons) (((List *) cons)->car)
#define CDR(cons) (((List *) cons)->cdr)

#define CAAR(cons) CAR(CAR(cons))
#define CADR(cons) CAR(CDR(cons))
#define CDAR(cons) CDR(CAR(cons))
#define CDDR(cons) CDR(CDR(cons))

#define CAAAR(cons) CAR(CAR(CAR(cons)))
#define CAADR(cons) CAR(CAR(CDR(cons)))
#define CADAR(cons) CAR(CDR(CAR(cons)))
#define CADDR(cons) CAR(CDR(CDR(cons)))
#define CDAAR(cons) CDR(CAR(CAR(cons)))
#define CDADR(cons) CDR(CAR(CDR(cons)))
#define CDDAR(cons) CDR(CDR(CAR(cons)))
#define CDDDR(cons) CDR(CDR(CDR(cons)))

#define FIRST_ELT(cons) CAR(cons)
#define SECOND_ELT(cons) CADR(cons)
#define THIRD_ELT(cons) CADDR(cons)
#define FOURTH_ELT(cons) FIRST_ELT(CDDDR(cons))
#define FIFTH_ELT(cons) SECOND_ELT(CDDDR(cons))
#define SIXTH_ELT(cons) THIRD_ELT(CDDDR(cons))
#define SEVENTH_ELT(cons) FOURTH_ELT(CDDDR(cons))
#define EIGHTH_ELT(cons) FIFTH_ELT(CDDDR(cons))
#define NINTH_ELT(cons) SIXTH_ELT(CDDDR(cons))
#define TENTH_ELT(cons) SEVENTH_ELT(CDDDR(cons))

#define PUSH(item, on_list) (on_list) = cons((item), (on_list))

#ifndef STANDALONE_EXTERN
#define STANDALONE_EXTERN extern
#endif

STANDALONE_EXTERN List *cons(),*nreverse(),*copylist(),*append(),*nconc(),*memq();
STANDALONE_EXTERN List *nthcdr(), *nth();
STANDALONE_EXTERN int list_length();
STANDALONE_EXTERN void free_list();

/* Macros */

#define ID(x) x
#define CONC(x,y) ID(x)y

#define NEW(structure_type) \
 ((structure_type *) malloc(sizeof (structure_type)))
#define NEW_ARRAY(structure_type,count) \
  ((structure_type *) malloc((count)*(sizeof (structure_type))))

#define COPY_STRUCTURE(type,top,fromp) \
{ register int XN__i = sizeof(type); \
  register char *XN__f = (char *) (fromp); \
  register char *XN__t = (char *) (top); \
  while (XN__i--) *XN__t++ = *XN__f++; \
}

#define WRITE_STRUCTURE(type,filep,fromp) \
{ register int XN__i = sizeof(type); \
  register char *XN__f = (char *) (fromp); \
  while (XN__i--) putc(*XN__f++,(filep)); \
}

#define READ_STRUCTURE(type,filep,top) \
{ register int XN__i = sizeof(type); \
  register char *XN__t = (char *) (top); \
  while (XN__i--) *XN__t++ = getc(filep); \
}

#define ZERO_STRUCTURE(type,top) \
{ register int XN__i = sizeof(type); \
  register char *XN__t = (char *) (top); \
  while (XN__i--) *XN__t++ = 0; \
}

#define FONTHEIGHT(f) ((f)->max_bounds.ascent+(f)->max_bounds.descent)

extern long int lrand48();
extern double erand48();
extern double log1p();
extern double log10();
extern double log2();
/*extern int rint();*/
extern char *malloc();

STANDALONE_EXTERN short random_seed48[3];
STANDALONE_EXTERN short scrap_seed48[3];
STANDALONE_EXTERN char seed_string_buffer[20];

/* Allocation */

STANDALONE_EXTERN char *XNnewString();
STANDALONE_EXTERN void XNwarning(), XNfatal();
STANDALONE_EXTERN char lowio_character_buffer[1000];

/*
XN_EXTERN_LOWIO void eatnlcopy();
XN_EXTERN_LOWIO char *strip_thru_space();
XN_EXTERN_LOWIO char *today();
XN_EXTERN_LOWIO void XNmessage();
XN_EXTERN_LOWIO char *XNsprintf();
XN_EXTERN_LOWIO char XNeatch();
XN_EXTERN_LOWIO Boolean XNtestEof();
XN_EXTERN_LOWIO char XNpeekc();
XN_EXTERN_LOWIO char XNgetLn();
XN_EXTERN_LOWIO void XNappendToFile();
XN_EXTERN_LOWIO void XNinitFile();
XN_EXTERN_LOWIO Boolean XNtryToInitFile();
XN_EXTERN_LOWIO void XNinitFileIfNeeded();
XN_EXTERN_LOWIO void XNinitFileWithTruncation();
XN_EXTERN_LOWIO XNfileBuffer *XNfindOrCreateFileBuffer();
XN_EXTERN_LOWIO void XNdumpAllFileBuffers();
XN_EXTERN_LOWIO void XNdumpFileBuffer();
XN_EXTERN_LOWIO void XNappendToBufferedFile();
XN_EXTERN_LOWIO Boolean XNattemptToDumpBufferedFile();
*/

STANDALONE_EXTERN void randomize(),init_randomizer();
STANDALONE_EXTERN char *seed_to_string();
STANDALONE_EXTERN int create(), one_in(), with_prob(), create_n();
STANDALONE_EXTERN double bran(), fran(), gaussian();
STANDALONE_EXTERN void millisecond_sleep(), second_sleep();

#ifndef Max
#define Max(x,y) (((x)<(y))?(y):(x))
#endif
#ifndef Min
#define Min(x,y) (((x)<(y))?(x):(y))
#endif
#ifndef Abs
#define Abs(x) Max((x),-(x))
#endif
#ifndef Sgn
#define Sgn(x) (((x)>0)?1:(((x)<0)?-1:0))
#endif

/* MLL specials. */
#ifndef FLOAT
#define FLOAT(x) ((float)(x))
#endif
#ifndef INT
#define INT(x) ((int)(x))
#endif
#ifndef DOUBLE
#define DOUBLE(x) ((double)(x))
#endif
#ifndef LONG
#define LONG(x) ((long)(x))
#endif
#ifndef withprob
#define withprob with_prob
#endif
#ifndef oddsof
#define oddsof(x,y) (create(y)<(x))
#endif

/* epilogue */

#define _libXN_LOADED
#endif				/* #ifndef _libXN_LOADED */


