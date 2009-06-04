/* UTILIB.C, part of LVL library, (C) Libor Spacek */
/****************************************************************/
#include "lvl.h"
#include <time.h>
/*********************************************************************/
/* simple timex function to report cpu time for speed testing 
   timex(); prints to stderr cpu time in rounded miliseconds 
	since program started or since timex(); was last called */
void timex()
{ static int c0 = 0;
  (void)fprintf(stderr,"%dms CPU time\n",
      (int)(0.5+(clock()-c0)/(float)CLOCKS_PER_SEC*1000)); 
  c0 = clock(); }
/*********************************************************************/
float *alloc_1d_float(int n1)
{
    float *d;

    d = (float *) malloc(n1 * sizeof(float));
    if (d == NULL) {
        fprintf(stderr,"alloc_1d_float: mallocation failure!\n");
        exit(1);
}
return d;
}
/*********************************************************************/
float **alloc_2d_float(int n1, int n2)
{
    float **dd, *d;
    int j;

    dd = (float **) malloc(n1 * sizeof(float *));
    if (dd == NULL) {
        fprintf(stderr,"alloc_2d_float: mallocation failure!\n");
        exit(1);
    }
    d = (float *) malloc(n1 * n2 * sizeof(float));
    if (d == NULL) {
        fprintf(stderr,"alloc_2dfloat: row allocation failure!\n");
        exit(1);
    }
    dd[0] = d;
    for (j = 1; j < n1; j++) {
        dd[j] = &d[n2 * j];
    }
    return dd;
}
/*********************************************************************/
void free_2d_float(float **dd)
{
    free(dd[0]);
    free(dd);
}
/*********************************************************************/
IMAGE_TYPE **imarray(PICTURE *pic)
/* creates a column index vector of addresses for easy random row access */
{  IMAGE_TYPE **arr, *bufim = (IMAGE_TYPE *)pic->data;
   int i, samples = pic->samples;
   if (pic->fileid != IMAGE_ID) {
    	(void) fprintf(stderr,"Wrong type of pic passed to imarray\n"); exit(1); }
   arr = (IMAGE_TYPE **) malloc((pic->y)*sizeof(IMAGE_TYPE *));
   if (arr==NULL) {
	   (void) fprintf(stderr,"imarray allocation failure!\n"); exit(1); }
   arr[0]= bufim;
   for (i=1;i<pic->y;i++) arr[i]=  &bufim[(pic->x)*i*samples];
   return arr;
}
/*********************************************************************/
FIMAGE_TYPE **fimarray(PICTURE *pic)
/* creates a 2D array for easier random pixel access only one column
   vector of addresses is allocated */
{  FIMAGE_TYPE **arr, *bufim = (FIMAGE_TYPE *)pic->data;
   int i, samples = pic->samples;
   if (pic->fileid != FIMAGE_ID) {
	   (void) fprintf(stderr,"Wrong type pic passed to fimarray\n"); exit(1); }
   arr = (FIMAGE_TYPE **) malloc((pic->y)*sizeof(FIMAGE_TYPE *));
   if (arr==NULL) {
	   (void) fprintf(stderr,"fimarray allocation failure!\n"); exit(1);	}
   arr[0]= bufim;
   for (i=1;i<pic->y;i++) arr[i]= &bufim[(pic->x)*i*samples];
   return arr;
}
/*********************************************************************/
PICTURE *choppic(PICTURE *picin, float xorig, float yorig, int width, int height)
{
int newx,newy,s,smpls = picin->samples;
PICTURE *picout = 
   makepic(picin->fileid,width,height,xorig,yorig,width*height,smpls,picin->history);
switch (picin->fileid) {
   case IMAGE_ID: {
      IMAGE_TYPE *datain = 
         (IMAGE_TYPE *)(((IMAGE_TYPE *)picin->data)+(int)(smpls*(yorig*(picin->x) + xorig)));
      IMAGE_TYPE *dataout = (IMAGE_TYPE *)picout->data; 
      for(newy=0; newy<height; newy++)
      {
	      for(newx=0; newx<width; newx++)
	      {
		    for (s=0; s<smpls; s++) { *dataout = *datain; dataout++; datain++; }
	       datain += smpls*(picin->x - width);
	      }
	   } }
      break;
   case FIMAGE_ID: {
       FIMAGE_TYPE *datain = 
         (FIMAGE_TYPE *)((FIMAGE_TYPE *)(picin->data)+(int)(smpls*(yorig*(picin->x) + xorig)));
       FIMAGE_TYPE *dataout = (FIMAGE_TYPE *)picout->data; 
      for(newy=0; newy<height; newy++)
      {
	      for(newx=0; newx<width; newx++)
	      {
		    for (s=0; s<smpls; s++) { *dataout = *datain; dataout++; datain++; }
	       datain += smpls*(picin->x - width);
	      }
	   } }
      break;
	default: 
	   (void)fprintf(stderr,"choppic: not yet implemented for this fileid\n");
	   exit(1);
	}
return(picout);
}
/**************************************************************************/
/* insert one of many samples */
int insertsample(PICTURE *pic, PICTURE *picout, int colrno) 
{ int x, outsamples = picout->samples;
  if ((pic->fileid) != (picout->fileid))
   { (void)fprintf(stderr,"insertsample: fileids don't agree\n"); return(-1); }
  if (((outsamples-1) < colrno) || ((pic->samples) > 1))
   { (void)fprintf(stderr,"insertsample: wrong sample numbers\n"); return(-1); }
  switch (pic->fileid) {
   case IMAGE_ID: {
      IMAGE_TYPE *mybufin = (IMAGE_TYPE *)pic->data; 
      IMAGE_TYPE *mybufout = (IMAGE_TYPE *)picout->data; 
      mybufout += colrno;
      for (x = 0; x < pic->items; x++)
	      { *mybufout = *(mybufin++); mybufout+=outsamples; }  }
      break;
   case FIMAGE_ID: {
      FIMAGE_TYPE *mybufin = (FIMAGE_TYPE *)pic->data; 
      FIMAGE_TYPE *mybufout = (FIMAGE_TYPE *)picout->data; 
      mybufout += colrno;
      for (x = 0; x < pic->items; x++)
	      { *mybufout = *(mybufin++); mybufout+=outsamples; }  }
	   break;
	default: 
	   (void)fprintf(stderr,"insertsample: not yet implemented for this fileid\n");
	   return(-1);
	}
return(0);
}
/**********************************************************************/
/* extract one of many samples */
int onesample(PICTURE *pic, PICTURE *picout, int colrno) 
{ int x, insamples = pic->samples;
  if ((pic->fileid) != (picout->fileid))
   { (void)fprintf(stderr,"onesample: fileids don't agree\n"); return(-1); }
  if (((insamples-1) < colrno) || ((picout->samples) > 1))
   { (void)fprintf(stderr,"onesample: wrong sample numbers\n"); return(-1); }
  switch (pic->fileid) {
   case IMAGE_ID: {
      IMAGE_TYPE *mybufin = (IMAGE_TYPE *)pic->data; 
      IMAGE_TYPE *mybufout = (IMAGE_TYPE *)picout->data; 
      mybufin += colrno;
      for (x = 0; x < pic->items; x++)
	      { *(mybufout++) = *mybufin; mybufin+=insamples; }  }
      break;
   case FIMAGE_ID: {
      FIMAGE_TYPE *mybufin = (FIMAGE_TYPE *)pic->data; 
      FIMAGE_TYPE *mybufout = (FIMAGE_TYPE *)picout->data; 
      mybufin += colrno;
      for (x = 0; x < pic->items; x++)
	      { *(mybufout++) = *mybufin; mybufin+=insamples; }  }
	   break;
	default: 
	   (void)fprintf(stderr,"onesample: not yet implemented for this fileid\n");
	   return(-1);
	}
return(0);
}
/**********************************************************************
Input FIMAGE is not normalised but should not much exceed GMAX 
output is simply rounded to image form = quantisation loss */
PICTURE *froundtoim(PICTURE *pic)
{
  PICTURE *picout;
  FIMAGE_TYPE *bfi = (FIMAGE_TYPE *)pic->data, infpixel;
  IMAGE_TYPE *bfo;
  int x, sitems;
  if ((pic->fileid) != FIMAGE_ID)
    { fprintf(stderr,"ftoim: wrong fileid\n");
      return(NULL);
    }
  sitems = (pic->samples)*(pic->items);
  picout = copypic(pic,(pic->samples),IMAGE_ID); // creates new output pic, with empty data
  bfo = (IMAGE_TYPE *)picout->data;
  for (x = 0; x < sitems; x++)
    { infpixel = *(bfi++); // negate for display
     if (infpixel <= 0.0) *bfo = (IMAGE_TYPE) GMAX;
     else { if (infpixel >= GMAX) *bfo = (IMAGE_TYPE) 0; 
            else *bfo = (IMAGE_TYPE) (GMAX - (int)floor(0.5+infpixel)); }
     bfo++; }
 return(picout);
}
/************************************************************************
Input FIMAGE must be normalised [0..1] 
black,white are thresholds (0..1) for linear transformation
if invert = true then invert output into a negative */
PICTURE *ftoim(PICTURE *pic, float black, float white, int invert)
{
  PICTURE *picout;
  FIMAGE_TYPE *bfi = (FIMAGE_TYPE *)pic->data, infpixel;
  IMAGE_TYPE *bfo;
  int x, sitems;
  float range = white-black;
  if ((pic->fileid) != FIMAGE_ID)
    { fprintf(stderr,"ftoim: wrong fileid\n");
      return(NULL);
    }
  sitems = (pic->samples)*(pic->items);
  picout = copypic(pic,(pic->samples),IMAGE_ID); // creates new output pic, with empty data
  bfo = (IMAGE_TYPE *)picout->data;
  if (invert)
   for (x = 0; x < sitems; x++)
     { infpixel = ((*(bfi++))-black)/range;
	if (infpixel <= 0.0) *bfo = (IMAGE_TYPE) GMAX;
	else { if (infpixel >= 1.0) *bfo = (IMAGE_TYPE) 0; 
		else *bfo = (IMAGE_TYPE)(long)(0.5 + GMAX*(1.0-infpixel)); }
       bfo++; }
  else
   for (x = 0; x < sitems; x++)
     { infpixel = ((*(bfi++))-black)/range;
	if (infpixel <= 0.0) *bfo = (IMAGE_TYPE) 0;
	else { if (infpixel >= 1.0) *bfo = (IMAGE_TYPE) GMAX; 
		else *bfo = (IMAGE_TYPE) (long)(0.5+GMAX*infpixel); }
       bfo++; }
 return(picout);
}
/************************************************************************/
PICTURE *imtof(PICTURE *pic)
{
  PICTURE *picout;
  IMAGE_TYPE *bfi = (IMAGE_TYPE *)pic->data;
  float *bfo;
  int x, sitems;
  if ((pic->fileid) != IMAGE_ID)
    { fprintf(stderr,"imtof: wrong fileid\n");
      return(NULL);
    }
  sitems = (pic->samples)*(pic->items); 
  picout = copypic(pic,(pic->samples),FIMAGE_ID); // creates new output pic, with empty data
  bfo = (float *)picout->data;
  for (x = 0; x < sitems; x++)
     {
       *bfo = (float)(*bfi); /* copy data as floats */
       bfo++; bfi++;
     }
  normalise(picout);
  return(picout);
}
/************************************************************************/
/* adds rgb and normalises to form b/w float image */
PICTURE *addsamplesfim(PICTURE *pic)
{
  PICTURE *picout;
  unsigned char *bfi = (unsigned char *)pic->data;
  float *bfo;
  int x, sitems;
  if ((pic->fileid) != IMAGE_ID)
    { fprintf(stderr,"addcomponentsfim: wrong fileid\n");
      return(NULL);
    }
  sitems = (pic->samples)*(pic->items); 
  picout = copypic(pic,1,FIMAGE_ID);
  bfo  = (FIMAGE_TYPE *)picout->data;
  for (x = 0; x < sitems; x++)
     {
       /* *bfo = (77.0*(*bfi++) + 150.0*(*bfi++) + 29.0*(*bfi++)); */
       *bfo =  (float) *bfi++;
       *bfo += (float) *bfi++;
       *bfo += (float) *bfi++;
       bfo++; 
     }
  normalise(picout);
  return(picout);
}
/***********************************************************************
creates a contrasty greyscale from rgb by selecting the minimal component  */
PICTURE *minsamplesim(PICTURE *pic)
{
  PICTURE *picout;
  unsigned char *bfo,*bfi = (unsigned char *)pic->data,min,pix;
  int x, s;

  if ((pic->fileid != IMAGE_ID) || (pic->samples < 2))
    { fprintf(stderr,"minsamplesim: wrong fileid\n");
      return(NULL);
    }
  picout = copypic(pic,1,IMAGE_ID);
  bfo  = (IMAGE_TYPE *)picout->data;
  for (x = 0; x < pic->items; x++)
  { min = *(bfi++);
    for (s = 1; s < pic->samples; s++)
     {
       pix = *(bfi++);
       if (pix < min) min = pix;
     }
   *(bfo++) = min;
  }
  return(picout);
}
/***********************************************************************
	*ftobnd input is one normalised floating scalar per sample. 
	All exceeding threshold are kept - previous coding usage: threshold=1.0
   Output is one sample stream of BND_TYPE records */
 PICTURE *ftobnd(PICTURE *pic, float threshold) 
{
  PICTURE *picout;
  FIMAGE_TYPE *bfi = (FIMAGE_TYPE *)pic->data;
  BND_TYPE *bfo;
  int x, sitems = pic->items * pic->samples, pcount = 0;

  if ((pic->fileid) != FIMAGE_ID)
    { fprintf(stderr,"ftobnd: wrong fileid\n"); return(NULL);}

  for (x = 0; x < sitems; x++)  /* first pass to count items */
     {
       if ( *bfi >= threshold ) pcount++;
       bfi++;
     }
  bfi = (FIMAGE_TYPE *)pic->data; /* reset pointer */

  pic->items = pcount; /* to set up output data size */
  picout = copypic(pic,1,BND_ID);
  /* fudge to preserve samples count without wasting memory allocating space */
  picout->samples = pic->samples; 
  bfo = (BND_TYPE *)picout->data;
  pic->items = pic->x * pic->y;  /* restores input pic, no side-effects */

  for (x = 0; x < sitems; x++)
     {
       if ( *bfi >= threshold )
	 {
	   bfo->index = x;
	   bfo->value = *bfi;
	   bfo++;
	 }
       bfi++;
     }
  return(picout);
}
/************************************************************************/
/* inverse of ftobnd - reconstructs the original fimage */
PICTURE *bndtof(PICTURE *pic) 
{
  PICTURE *picout;
  BND_TYPE *bfi = (BND_TYPE *)pic->data;
  FIMAGE_TYPE *bfo;
  int x,sitems = pic->items * pic->samples;

  if ((pic->fileid) != BND_ID)
    { fprintf(stderr,"bndtof: wrong fileid\n"); return(NULL);}

  picout = makepic(FIMAGE_ID,pic->x,pic->y,
		   pic->xorigin,pic->yorigin,(pic->x)*(pic->y),pic->samples,pic->history);
  bfo = (FIMAGE_TYPE *)picout->data;

  for (x = 0; x < sitems; x++)
     {
       if ( bfi->index == x )
	 {
	   *bfo = bfi->value;
	   bfi++;
	 }
       else *bfo = 0.0;
       bfo++;
     }
  return(picout);
}
/***********************************************************************/
