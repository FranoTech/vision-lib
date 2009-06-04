/* LVL Libor Spacek (C) 1992-2009: Libor's Vision Library Header File
   this file includes <stdlib.h>,<stdio.h> and <math.h>, so do not include them
   in your programs. All 'system' defines and globals (except function names)
   are capitalised to help avoid symbol conflicts */
/*************************************************************************/
#define _CRT_SECURE_NO_DEPRECATE  /* suppress MS warnings against POSIX C*/
/*************************************************************************/
#define MESSAGE "***Libor's Vision Library 2.0, 1st June 2009 (C) Dr L.Spacek***\n"
/*************************************************************************/

#ifndef VIS_HEADER
#define VIS_HEADER
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#ifdef WIN32
#  ifdef LVL_EXPORTS
#    define LVLAPI __declspec(dllexport) extern
#  else
#    define LVLAPI __declspec(dllimport) extern
#  endif //LVL_EXPORTS
#else
#  define LVLAPI extern
#endif //WIN32

#define GMAX 255  /* for ppm & pgm reader check etc */
#define RED 0
#define GREEN 1
#define BLUE 2
/********************************************************************
 These IDs are written to the VISION file header. 
 They determine the various C type  interpretations of the individual
 data items (pixels in the case of image files).
 The data files are normally binary for efficiency and compactness.
**********************************************************************/
#define IMAGE_ID 0
#define POSINT_ID 1
#define INT_ID 2
#define FIMAGE_ID 3
#define FVEC_ID 4
#define IVEC_ID 5
#define BND_ID 6
/********************************************************************
 The associated base types
 rgb files use these types in plain sequence r,g,b,r,g,b,...
 struct triples are not used because they vary in size from machine to
 machine. For instance:
 on MOTOROLA:  sizeof(struct{char r; char g; char b;}) = 4,
 on INTEL it is 3 (the same as written to disk)
 the inline function 'pada' below can be used instead
*********************************************************************/
#define IMAGE_TYPE unsigned char
#define POSINT_TYPE unsigned int
#define INT_TYPE int
#define FIMAGE_TYPE float
typedef struct {float fx; float fy;} FVEC_TYPE;
typedef struct {int fx; int fy;} IVEC_TYPE;
typedef struct {int index; float value;} BND_TYPE;
/***********************************************************************
PICTURE is the core structure describing all images and 
        derived representations (the latter need not be pixel based)
***********************************************************************/
typedef struct {
/*  int magic is read and written separately by readhead and writepic  */
	int	fileid;      /* file type identification number */
	int	x;           /* dimension across in image pixels */
	int	y;           /* dimension down in image pixels */
	float	xorigin;     /* records new x origin - can be used for tiling */
	float	yorigin;     /* records new y origin */
	int	items;       /* count of data items (usually pixels = x*y) */
	int	samples;     /* e.g. colour planes per pixel, usually 1 or 3 */
	char	history [84];/* info string - processing history is appended */
	void	*data;       /* points to the data (when loaded in memory) */
	void	*udata;      /* user index or data */
	void	*adata;		 /* application index or data */
	int 	magic;		 /* internal identification of the input filetype */
} PICTURE;
/* HEADSZ is the size of the PICTURE struct minus the data pointers at the end, 
	  as written to the disk (8 bytes aligned for binary portability) */
#define HEADSZ (sizeof(PICTURE)-3*sizeof(void *)-sizeof(int))
/********************************************************************
 pixel address (pad) inline function to compute 1D address from x,y.
 Use it either wrt (0,0) as in: (pic->data)+pad(pic->xacross,x,y), 
 or as an increment to some current address: pad(pic->xacross,dx,dy)
 To access data in sequence x, use simply: currentaddress+x
********************************************************************/
static inline int pad(int rowl,int dx,int dy) { return(rowl*dy+dx); } 
/********************************************************************
 pada is for any composite items, where samples = s
 Use the simplest one you can for speed 
********************************************************************/
static inline int pada(int s,int rowl,int dx,int dy) {return(s*(rowl*dy+dx));}
/********************************************************************/
static inline int modrgb(unsigned char *ptr) 
	{ int res = *(ptr++);
	  res *= res;
	  res += (*ptr)*(*ptr); ptr++;
	  res += (*ptr)*(*ptr);
	  return (res);
	}
/********************************************************************
scalar product of vectors a and b with 'dimension' components
********************************************************************/
static inline float scprod(float *a, unsigned char *b, int dimension) 
	{ int d; float res = 0.0;
	  for (d=0; d<dimension; d++) res += a[d]*(float)b[d];
	  return res; }
/********************************************************************/
static inline float modfvec(FVEC_TYPE *g) 
	{ float fx = g->fx, fy = g->fy;
	  return (sqrt(fx*fx+fy*fy)); }
/********************************************************************
 Public functions defined in the library sources:
********************************************************************/
extern void
	vmessage(),
	timex(),
	swaphead(PICTURE *),
	freepic(PICTURE *),
	readin(char *, int , int , float *),
	writeout(char *, int, int, float *, int),
	writematrix(char *, int, double **, int),
	writeeigenvs(char *, int, double *, int),
	reduce(PICTURE *, int, int, int),
	dilategrads(PICTURE *,PICTURE *),
	erodegrads(PICTURE *,PICTURE *), 
	free_2d_float(float **);

extern char *PROGNAME,*FILEIN,*FILEOUT;
  
extern float 
   *alloc_1d_float(int),
   **alloc_2d_float(int, int),
   *meanpixel(PICTURE *);

/* creates vector of row addresses. Use as 2D array */
extern IMAGE_TYPE  **imarray(PICTURE *); 
extern FIMAGE_TYPE **fimarray(PICTURE *);

extern int
  sizeofidtype(int),	/* iovis.o */
  sizeofitem(PICTURE *),
  sizeofdata(PICTURE *),
  dofiles(int, char *[]),
  printhead(PICTURE *, int),
  readhead(PICTURE *, FILE *),
  fillhead(PICTURE *, int, int, int, float, float, int, int, char *),
  readdata(PICTURE *, FILE *),
  readascidata(PICTURE *, FILE *),
  newdata(PICTURE *), /* overwrites pic->data with pointer to new memory */
  changepic(PICTURE *, int, int), /* change type, samples, and size of pic */
  write_asci(PICTURE *, int, int, int, int, FILE *), /* headerless output */
  writepic(PICTURE *, char *), /* binary LVL */
  writeascipic(PICTURE *, char *), /* ascii LVL */
  writepmpic(PICTURE *, char *), /* binary portable pixel maps */
  writepmascipic(PICTURE *, char *), /* ascii portable pixel maps */
  writeanypic(PICTURE *, char *, int), /* does all the work */

  diffim(PICTURE *,PICTURE *), /* subtract second from first in place */
  imerr(PICTURE *, PICTURE *), 
  /* sum of absolute values of differences between all pixels */
  onesample(PICTURE *, PICTURE *, int), /* int is sample number, e.g. RED*/
  insertsample(PICTURE *, PICTURE *, int),

  /*  aspect(PICTURE *), */
  contrast(PICTURE *,int,int), /* pixels to be made black and white */
  histogram(PICTURE *,int,int),
  morph(PICTURE *, int),

  smooth(PICTURE *, int), /* second arg is the operator radius */
  fxfy(PICTURE *, int, FVEC_TYPE *, int, float),
  ifxfy(PICTURE *, int, IVEC_TYPE *, int),
  normalise(PICTURE *),/*normalise FIMAGE, find max min*/
  nonmax(PICTURE *, float),
  skeletor(PICTURE *);
  
extern PICTURE
  *newpic(void), /* allocates new pic without data buffer */
  *newvxpic(char *), /* for robots pictures only - fixed size */
  *makepic(int,int,int,float,float,int,int,char *), 
  /* create complete pic with blank memory allocated */
  *choppic(PICTURE *,float,float,int,int), /* extract window out of pic */
  *copypic(PICTURE *, int, int), /* int is fileid, does not copy data! */
  *copyhead(PICTURE *, int, int), /* does not even allocate data memory */
  *readpic(char *),
  *read_asci_pic(int, int, int, int, char *, char *), /* reads headerless ascii file */
  *ftobnd(PICTURE *, float),
  *bndtof(PICTURE *),
  *quarter(PICTURE *),
  *sumimage(PICTURE *), /* 2D (proper) integral image in newedgelib.c */
  *sumimage1d(PICTURE *), /* single running total in newedgelib.c */
  *grad(PICTURE *, int),
  *edgegrad(PICTURE *, int),
  *fastedgegrad(PICTURE *, int),
  *gradmag(PICTURE *),
  *gradmax(PICTURE *),
  *morphgrads(PICTURE *),
  
  *edge(PICTURE *, int, float, float),
  *edge3(PICTURE *, int, float, float),
  *coledge(PICTURE *, int, float, float),
  *fbndry(PICTURE *, int),
  *ibndry(PICTURE *, int, int),
  *icurvature(PICTURE *, int, int),
  *imtof(PICTURE *),
  *ftoim(PICTURE *, float, float, int),
  *froundtoim(PICTURE *),
  *fcontrast(PICTURE *, int, int),
  *minsamplesim(PICTURE *),  /* convert rgb to bw */
  *addsamplesfim(PICTURE *), 
  *tobwf(PICTURE *); /* convert rgb to floating bw */
 // *dct(PICTURE *, int),  /* discrete cosine transform */
 // *idct(PICTURE *), /* inverse discrete cosine transform */
 // *unwarpidct(PICTURE *, int,int,float,float,float), 
 // /* as above plus omnidirectional image unwarping */
 // *unwarprgb(PICTURE *, int,int,float,float,float), /* simple unwarping */
 // *laplace(PICTURE *), /* laplacian */
 // *dx2dy2(PICTURE *), /* idct to give fx^2 + fy^2 instead of just f */
 // *dst(PICTURE *, int, int),  /* discrete sine transform */
 // *idst(PICTURE *); /* inverse discrete sine transform */
/********************************************************************
   The remainder concerns the Principal Components Analysis
   source file: pca.c. This works but is not yet documented in the manual.
   programs: eigen and reduce
*********************************************************************/
// extern float nntest(float **, int, int, int, int);

// extern double* eigen(double **, int);
// /* finds sorted eigenvalues and eigenvectors using Householder's method */
// extern double** makecovar(float **, int, int, int);
// /* makecovar is typically needed before calling eigen for its second argument*/
#endif
