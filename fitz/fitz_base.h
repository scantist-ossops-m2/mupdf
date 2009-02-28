/*
 * Include the basic standard libc headers.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include <limits.h>	/* INT_MIN, MAX ... */
#include <float.h>	/* DBL_EPSILON */
#include <math.h>

#include <errno.h>
#include <fcntl.h>	/* O_RDONLY & co */

/* Stupid macros that don't exist everywhere */

#ifndef O_BINARY
#define O_BINARY 0
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_SQRT2
#define M_SQRT2 1.41421356237309504880
#endif

/* Some useful semi-standard functions */

#ifdef NEED_STRLCPY
extern int strlcpy(char *dst, const char *src, int n);
extern int strlcat(char *dst, const char *src, int n);
#endif

#ifdef NEED_STRSEP
extern char *strsep(char **stringp, const char *delim);
#endif

#ifdef NEED_GETOPT
extern int getopt(int nargc, char * const * nargv, const char *ostr);
extern int opterr, optind, optopt;
extern char *optarg;
#endif

#ifdef MSVC /* stupid stone-age compiler */

#include <io.h>

extern int gettimeofday(struct timeval *tv, struct timezone *tz);

#define FZ_FLEX 1
#define restrict

#ifdef _MSC_VER
#define inline __inline
#else
#define inline __inline__
#endif

#define __func__ __FUNCTION__

#if _MSC_VER < 1500
#define vsnprintf _vsnprintf
#endif

#ifndef isnan
#define isnan _isnan
#endif

#ifndef va_copy
#define va_copy(a,b) (a) = (b)
#endif

#ifndef R_OK
#define R_OK 4
#endif

#else /* C99 or close enough */ 

#include <unistd.h>
#define FZ_FLEX

#endif

/*
 * CPU detection and flags
 */

#if defined(ARCH_X86) || defined(ARCH_X86_64)
#  define HAVE_CPUDEP
#  define HAVE_MMX        (1<<0)
#  define HAVE_MMXEXT     (1<<1)
#  define HAVE_SSE        (1<<2)
#  define HAVE_SSE2       (1<<3)
#  define HAVE_SSE3       (1<<4)
#  define HAVE_3DNOW      (1<<5)
#  define HAVE_AMD64      (1<<6)

#elif defined (ARCH_PPC)
#  define HAVE_CPUDEP
#  define HAVE_ALTIVEC    (1<<7)

#elif defined (ARCH_SPARC)
#  define HAVE_CPUDEP
#  define HAVE_VIS        (1<<8)

#endif

/* call this before using fitz */
extern void fz_cpudetect();

/* treat as constant! */
extern unsigned fz_cpuflags;

/*
 * Base Fitz runtime.
 */

#ifndef __printflike
#if __GNUC__ > 2 || __GNUC__ == 2 && __GNUC_MINOR__ >= 7
#define __printflike(fmtarg, firstvararg) \
        __attribute__((__format__ (__printf__, fmtarg, firstvararg)))
#else
#define __printflike(fmtarg, firstvararg)
#endif
#endif

#ifndef nil
#define nil ((void*)0)
#endif

#ifndef offsetof
#define offsetof(s, m) (unsigned long)(&(((s*)0)->m))
#endif

#ifndef nelem
#define nelem(x) (sizeof(x)/sizeof((x)[0]))
#endif

#ifndef ABS
#define ABS(x) ( (x) < 0 ? -(x) : (x) )
#endif

#ifndef MAX
#define MAX(a,b) ( (a) > (b) ? (a) : (b) )
#endif

#ifndef MIN
#define MIN(a,b) ( (a) < (b) ? (a) : (b) )
#endif

#ifndef CLAMP
#define CLAMP(x,a,b) ( (x) > (b) ? (b) : ( (x) < (a) ? (a) : (x) ) )
#endif

#define MAX4(a,b,c,d) MAX(MAX(a,b), MAX(c,d))
#define MIN4(a,b,c,d) MIN(MIN(a,b), MIN(c,d))

#define STRIDE(n, bcp) (((bpc) * (n) + 7) / 8)

/* plan9 stuff for utf-8 and path munging */
int chartorune(int *rune, char *str);
int runetochar(char *str, int *rune);
int runelen(long c);
int runenlen(int *r, int nrune);
int fullrune(char *str, int n);
char *cleanname(char *name);

typedef struct fz_error_s fz_error;

struct fz_error_s
{
	char msg[184];
	char file[32];
	char func[32];
	int line;
	fz_error *cause;
};

#define fz_outofmem (&fz_koutofmem)
extern fz_error fz_koutofmem;

void fz_printerror(fz_error *eo);
void fz_droperror(fz_error *eo);
void fz_warn(char *fmt, ...) __printflike(1,2);

#define fz_throw(...) fz_throwimp(nil, __func__, __FILE__, __LINE__, __VA_ARGS__)
#define fz_rethrow(cause, ...) fz_throwimp(cause, __func__, __FILE__, __LINE__, __VA_ARGS__)
#define fz_okay ((fz_error*)0)

fz_error *fz_throwimp(fz_error *cause, const char *func, const char *file, int line, char *fmt, ...) __printflike(5, 6);

typedef struct fz_memorycontext_s fz_memorycontext;

struct fz_memorycontext_s
{
	void * (*malloc)(fz_memorycontext *, int);
	void * (*realloc)(fz_memorycontext *, void *, int);
	void (*free)(fz_memorycontext *, void *);
};

fz_memorycontext *fz_currentmemorycontext(void);
void fz_setmemorycontext(fz_memorycontext *memorycontext);

void *fz_malloc(int n);
void *fz_realloc(void *p, int n);
void fz_free(void *p);

char *fz_strdup(char *s);

/*
 * Generic hash-table with fixed-length keys.
 */

typedef struct fz_hashtable_s fz_hashtable;

fz_error *fz_newhash(fz_hashtable **tablep, int initialsize, int keylen);
fz_error *fz_resizehash(fz_hashtable *table, int newsize);
void fz_debughash(fz_hashtable *table);
void fz_emptyhash(fz_hashtable *table);
void fz_drophash(fz_hashtable *table);

void *fz_hashfind(fz_hashtable *table, void *key);
fz_error *fz_hashinsert(fz_hashtable *table, void *key, void *val);
fz_error *fz_hashremove(fz_hashtable *table, void *key);

int fz_hashlen(fz_hashtable *table);
void *fz_hashgetkey(fz_hashtable *table, int idx);
void *fz_hashgetval(fz_hashtable *table, int idx);

/* multiply 8-bit fixpoint (0..1) so that 0*0==0 and 255*255==255 */
#define fz_mul255(a,b) (((a) * ((b) + 1)) >> 8)
#define fz_floor(x) floor(x)
#define fz_ceil(x) ceil(x)

/* divide and floor towards -inf */
static inline int fz_idiv(int a, int b)
{
	return a < 0 ? (a - b + 1) / b : a / b;
}

/* from python */
static inline void fz_idivmod(int x, int y, int *d, int *m)
{
	int xdivy = x / y;
	int xmody = x - xdivy * y;
	/* If the signs of x and y differ, and the remainder is non-0,
	 * C89 doesn't define whether xdivy is now the floor or the
	 * ceiling of the infinitely precise quotient.  We want the floor,
	 * and we have it iff the remainder's sign matches y's.
	 */
	if (xmody && ((y ^ xmody) < 0)) {
		xmody += y;
		xdivy --;
	}
	*d = xdivy;
	*m = xmody;
}

typedef struct fz_matrix_s fz_matrix;
typedef struct fz_point_s fz_point;
typedef struct fz_rect_s fz_rect;
typedef struct fz_ipoint_s fz_ipoint;
typedef struct fz_irect_s fz_irect;

extern fz_rect fz_emptyrect;
extern fz_rect fz_infiniterect;

#define fz_isemptyrect(r) ((r).x0 == (r).x1)
#define fz_isinfiniterect(r) ((r).x0 > (r).x1)

/*
	/ a b 0 \
	| c d 0 |
	\ e f 1 /
*/

struct fz_matrix_s
{
	float a, b, c, d, e, f;
};

struct fz_point_s
{
	float x, y;
};

struct fz_rect_s
{
	float x0, y0;
	float x1, y1;
};

struct fz_ipoint_s
{
	int x, y;
};

struct fz_irect_s
{
	int x0, y0;
	int x1, y1;
};

void fz_invert3x3(float *dst, float *m);

fz_matrix fz_concat(fz_matrix one, fz_matrix two);
fz_matrix fz_identity(void);
fz_matrix fz_scale(float sx, float sy);
fz_matrix fz_rotate(float theta);
fz_matrix fz_translate(float tx, float ty);
fz_matrix fz_invertmatrix(fz_matrix m);
int fz_isrectilinear(fz_matrix m);
float fz_matrixexpansion(fz_matrix m);

fz_rect fz_intersectrects(fz_rect a, fz_rect b);
fz_rect fz_mergerects(fz_rect a, fz_rect b);

fz_irect fz_roundrect(fz_rect r);
fz_irect fz_intersectirects(fz_irect a, fz_irect b);
fz_irect fz_mergeirects(fz_irect a, fz_irect b);

fz_point fz_transformpoint(fz_matrix m, fz_point p);
fz_rect fz_transformaabb(fz_matrix m, fz_rect r);

/*
 *TODO: move this into draw module
 */

/*
pixmaps have n components per pixel. the first is always alpha.
premultiplied alpha when rendering, but non-premultiplied for colorspace
conversions and rescaling.
*/

typedef struct fz_pixmap_s fz_pixmap;
typedef unsigned char fz_sample;

struct fz_pixmap_s
{
	int x, y, w, h, n;
	fz_sample *samples;
};

fz_error *fz_newpixmapwithrect(fz_pixmap **mapp, fz_irect bbox, int n);
fz_error *fz_newpixmap(fz_pixmap **mapp, int x, int y, int w, int h, int n);
fz_error *fz_newpixmapcopy(fz_pixmap **pixp, fz_pixmap *old);

void fz_debugpixmap(fz_pixmap *map, char *prefix);
void fz_clearpixmap(fz_pixmap *map);
void fz_droppixmap(fz_pixmap *map);

fz_error *fz_scalepixmap(fz_pixmap **dstp, fz_pixmap *src, int xdenom, int ydenom);

/* needed for tiled rendering */
fz_error *fz_newscaledpixmap(fz_pixmap **dstp, int w, int h, int n, int xdenom, int ydenom);
fz_error *fz_scalepixmaptile(fz_pixmap *dstp, int xoffs, int yoffs,
			     fz_pixmap *tile, int xdenom, int ydenom);

