/* IMPROCLIB.C, part of LVL library, (C) Libor Spacek */
/****************************************************************/
#include "lvl.h"
#define ROUND(x) ((int) floor((x)+0.5))
#ifndef MAXFLOAT
#define MAXFLOAT 1.0E20
#endif
/****************************************************************************/
/* does global normalisation of all samples. Overwrites pic in-place */
int normalise(PICTURE *pic)
{ 
  FIMAGE_TYPE *bfi = (FIMAGE_TYPE *)pic->data;
  float range,max = (float)-MAXFLOAT,min = (float)MAXFLOAT;
  int x,sitems;
  if ((pic->fileid) != FIMAGE_ID) 
    { fprintf(stderr,"normalise: wrong fileid\n"); return(-1); }
  sitems = (pic->samples)*(pic->items);
  for (x=0;x<sitems;x++)
      { 
		if (*bfi > max) max = *bfi;
		if (*bfi < min) min = *bfi;
		bfi++;
      }
  bfi = (FIMAGE_TYPE *)pic->data; /* reset to the start */
  range = max-min;
  for (x = 0; x < sitems; x++)
	{
	  if (*bfi <= min) { *(bfi++) = 0.0; continue; }
	  if (*bfi >= max) { *(bfi++) = 1.0; continue; }
	  *bfi = (*(bfi) - min)/range;  /* normalised */
	  bfi++;
	}
  return(0);
}
/**********************************************************************/
float *meanpixel(PICTURE *pic)
{
  IMAGE_TYPE *bfi = (IMAGE_TYPE *)pic->data; 
  float *res;
  double rgbmod = 0.0;
  int x, s, sampls = pic->samples;
  res = (float *)malloc(sampls*sizeof(float));
  if (res == NULL) {
	(void)fprintf(stderr,"meanpixel: failed to allocate memory for the result\n");
	exit(1); }
  for (s=0; s < sampls; s++) *(res+s) = 0.0; /* initialise sums to zero */
  
  for (x = 0; x < pic->items; x++)
     for (s=0; s < sampls; s++) *(res+s) += (float) *(bfi++); /*add all pixels*/
  for (s=0; s < sampls; s++) 
     rgbmod += res[s] * res[s]; /* euclidian norm */
  rgbmod = sqrt(rgbmod);
  for (s=0; s < sampls; s++) res[s] /= (float)rgbmod; /* res is now a unit vector */
  return(res);
}
/**********************************************************************/
/* projects component pixels on unit mean and normalises -> b/w float image */
PICTURE *tobwf(PICTURE *pic)
{
  PICTURE *picout;
  IMAGE_TYPE *bfi = (IMAGE_TYPE *)pic->data;
  float *bfo, *meanvec;
  int x, smpls = pic->samples;
  if ((pic->fileid) != IMAGE_ID) {
   (void)fprintf(stderr,"tobwf: wrong input fileid\n"); return(NULL); }
  meanvec = meanpixel(pic); /* finds the mean of rgb pixels */
  picout = copypic(pic,1,FIMAGE_ID);
  bfo  = (FIMAGE_TYPE *)picout->data;
  for (x = 0; x < pic->items; x++)
     {
       *bfo =  scprod(meanvec,bfi,smpls);
       bfo++; bfi+=smpls;
     }
  free(meanvec);
  normalise(picout); 
  return(picout);
}
/************************************************************************
changes pic in place by subtracting picneg and normalising */
int diffim(PICTURE *pic, PICTURE *negp)
{
  FIMAGE_TYPE *buf = (FIMAGE_TYPE *)pic->data,
  *bufneg = (FIMAGE_TYPE *)negp->data;
  int x,sitems;
  if (((pic->fileid) != FIMAGE_ID) || ((negp->fileid) != FIMAGE_ID))
    { fprintf(stderr,"diffim: wrong fileid\n"); return(-1);}
  sitems = (pic->samples)*(pic->items); 
  for (x=0; x < sitems; x++)
    {
      *buf -= *bufneg;
      buf++; bufneg++;
    }
  normalise(pic);
  return(0);
}
/************************************************************************/
int imerr(PICTURE *pic, PICTURE *picneg)
{ /* unsigned char pixel difference-measures global quality of reconstruction*/
  IMAGE_TYPE *buf = (IMAGE_TYPE *)pic->data,
  *bufneg = (IMAGE_TYPE *)picneg->data;
  int x,sitems,err=0;
  if (((pic->fileid) != IMAGE_ID) || ((picneg->fileid) != IMAGE_ID))
    { fprintf(stderr,"imerr: wrong fileid\n"); return(-1);}
  sitems = (pic->samples)*(pic->items);
  for (x=0; x < sitems; x++)
    {
      err += abs(*buf - *bufneg);
      buf++; bufneg++;
    }
  return(err);
}
/***********************************************************************/
PICTURE *quarter(PICTURE *pic)
/* reduces any image to quarter of its original size, freeing the original data
   sub-sampling with weighted 3x3 averaging */
{  
   PICTURE *picout = copyhead(pic,pic->samples,pic->fileid);  
   IMAGE_TYPE *bufim, *bufout;
   int x,y,s,samp = pic->samples,roww =(pic->x)*samp;
   if ((pic->fileid) != IMAGE_ID) 
      {  fprintf(stderr,"quarter: wrong filetype\n"); return(NULL); }
      
	picout->y = ((pic->y)-1)/2;
   picout->x = ((pic->x)-1)/2;
	picout->items = (picout->x)*(picout->y);
   newdata(picout); /* fresh memory */
   bufout = (IMAGE_TYPE *)picout->data;
	if (bufout == NULL)
    { fprintf(stderr,"quarter: failed to allocate memory for the new data\n");
      return(NULL); }
   for (y=0; y<(picout->y); y++)
	{
		bufim = (IMAGE_TYPE *)(pic->data)+2*y*roww; 
		for (x=0; x<(picout->x); x++)
		{
			for (s=0; s<samp; s++)
			{
			*(bufout++) = (IMAGE_TYPE) ROUND(( 12*(*(bufim+roww+samp)) + 
			2*(*(bufim+samp)+*(bufim+roww)+*(bufim+roww+2*samp)+*(bufim+2*roww+samp))
			+ *(bufim)+*(bufim+2*samp)+*(bufim+2*roww)+*(bufim+2*roww+2*samp))/24.f);
			bufim++;
			}
			bufim += samp;
		} 
	}
 return(picout);
}
/******************************************************************************/
float opvalue(float r) 
   {  float res = (float)(1.0-r);
   	return (float)( pow(res,(float)5.0) * ( 1.0 + 5.0*r ) ); /* polynomial operator */
	}
/******************************************************************************/
int smooth(PICTURE *pic,int oprad)   /* destructive smoothing */
{
  /* oprad >= 1 */

  IMAGE_TYPE *sub,*mybufin = (IMAGE_TYPE *)pic->data;
  FIMAGE_TYPE *subout,*mybufout;
  int x,y,width,height, opsize, opsizesq, radiussq;
  float *weights, weightsum = (float)0.0, min = (float)MAXFLOAT, max = (float)0.0, 
  resol = (float)GMAX, range, val;

  if (pic->fileid != IMAGE_ID)
  {
    fprintf(stderr,"smooth: wrong pic fileid\n");
    return(-1);
  }

  mybufout = (FIMAGE_TYPE *) malloc(pic->items*sizeof(FIMAGE_TYPE));

  if ((oprad < 1) || (oprad > 20))
    { fprintf(stderr,"Attempt to call smooth with non-sensible oprad %d",oprad);
      return(-1);
    }
   opsize = oprad-1;
   opsizesq = oprad*oprad-2*oprad+1;
   width = pic->x - 2*opsize;
   height = pic->y - 2*opsize;
   weights = (float *) malloc((opsizesq+1)*sizeof(float));

    for (x = 0; x < oprad; x++)  /* fill in the weights */
      for (y = 0; y <= x; y++)
	{  radiussq = x*x + y*y;
	   if (radiussq > opsizesq) break;
	  /* argument to opvalue in range 0.0  - 1.0 */
	   weights[radiussq] = opvalue(sqrt((float)radiussq)/oprad);

	 /* normalise, the weights, if you wish */
	    if ((y == 0) || (y == x)) weightsum += (float)4.0*weights[radiussq];
	    else weightsum += (float)8.0*weights[radiussq];
	}

    /* normalise the weights - not necessary for smoothing with linear trnsf. */
    for (x=0;x <= opsizesq; x++)
     {
      weights[x] /= weightsum;
#ifdef DEBUG
	printf("weight[%d] = %f, ",x,weights[x]);
#endif
     }
#ifdef DEBUG
   printf("sum: %f\n",weightsum);
#endif

  /* do the convolution */

  sub = mybufin+opsize*(pic->x)+opsize;
  subout = mybufout+opsize*(pic->x)+opsize;
  for (y = 0; y < height; y++)
  {
    for (x = 0; x < width; x++)
      {
       {
	      int xl, yl, hx, hy;
	      val = weights[0] * (unsigned int)(*sub);   /* x=0,y=0 */
	      for (xl = 1; xl < oprad; xl++)
	       {
	         hx = xl*(pic->x);
	         val += weights[xl*xl]*((unsigned int)(*(sub+xl))+(*(sub-xl))+
                  (*(sub+hx))+(*(sub-hx))); /*y=0*/
  		       for (yl = 1; yl <= xl; yl++)
 		       {  radiussq = xl*xl + yl*yl;
  	             if (radiussq > opsizesq) break;
  	             hy = yl*pic->x;
 	              if (yl < xl) 
 	              val += weights[radiussq]*((unsigned int)(*(sub+xl+hy))+
	    	         (*(sub+xl-hy))+(*(sub-xl+hy))+(*(sub-xl-hy)) +
		         (*(sub+yl+hx))+(*(sub+yl-hx))+(*(sub-yl+hx))+(*(sub-yl-hx)));
   	          else
                val += weights[radiussq]*((unsigned int)(*(sub+xl+hy))+
                (*(sub+xl-hy))+(*(sub-xl+hy))+(*(sub-xl-hy)));
  			   }
		     }
	    }
      if (val < min) min = val;
      if (val > max) max = val;
      *subout = val;
      subout++; sub++;
      }
    sub += pic->x - width;
    subout += pic->x - width;
  }

  sub = mybufin;
  subout = mybufout;
  range = max-min;
#ifdef DEBUG
  printf("max = %f, min = %f\n",max,min);
#endif
  for (y = 0; y < opsize; y++)        /* copy margins */
     for (x = 0; x < pic->x; x++)
	{ *subout = (*sub) / resol; sub++; subout++; }

  for (y = opsize; y+opsize < pic->y; y++)
    {
     for (x = 0; x < opsize; x++)
	{ *subout = *sub / resol; sub++; subout++; }
     for (x = 0; x < width; x++)
	{
	  *subout = (*subout - min)/range; /* normalise */
	  sub++; subout++;
	}
     for (x = 0; x < opsize; x++)
	{ *subout = *sub / resol; sub++; subout++; }
    }

  for (y = 0; y < opsize; y++)
     for (x = 0; x < pic->x; x++)
	{ *subout = *sub / resol; sub++; subout++; }

  free(pic->data);
  pic->data = mybufout;
  pic->fileid = FIMAGE_ID;
  return(0);
}
/****************************************************************************/
int bwcontrast(PICTURE *pic, int blacklimit, int whitelimit)
{
   IMAGE_TYPE *buff = (IMAGE_TYPE *)pic->data;
   float scale;
   int loc,countout,total = 0;
   int imin=0,imax=GMAX,profile[GMAX+1] = { 0 };
/* compute the histogram */
   for (countout = 0; countout < pic->items; countout++)
     profile[buff[countout]]++;
   while (profile[imin] == 0) imin++; /* find the smallest grey-level */
   while ((profile[imax] == 0) && (imax > imin+1)) imax--; /* and biggest */
/* compute the cumulative histogram */
   for (countout = imin; countout <= imax; countout++)
      {
       profile[countout] += total;      /* cumulative histogram */
       total = profile[countout];
      }
   if (blacklimit)
     while (profile[imin] < blacklimit) imin++;
   if (whitelimit)
     while ((profile[imax-1] > ((pic->items)-whitelimit)) && (imax > imin+1))
       imax--;
   scale = (float)GMAX / (float)(imax-imin);
/* construct the table */
  for (countout = imin+1; countout <= imax; countout++)
      profile[countout] = ROUND(scale*(countout-imin));
/* transform the image */
   for (countout = 0; countout < pic->items; countout++)
     {
       loc =  (int)buff[countout];
       if (loc >= imax) buff[countout] = GMAX;
       else { if (loc <= imin ) buff[countout] = 0;
             else buff[countout] = (IMAGE_TYPE)profile[loc]; }
     }
   return(0);
 }
/*********************************************************************/
int contrast(PICTURE *pic, int blim, int wlim)
{
  if (((pic->fileid) != IMAGE_ID) || ((pic->samples > 3)))
   { fprintf(stderr,"contrast: wrong filetype\n"); return(-1); }
  if ((pic->samples) == 1) { bwcontrast(pic,blim,wlim); return(0);}
	PICTURE *bwpic;
	bwpic = copypic(pic,1,IMAGE_ID);
	onesample(pic,bwpic,RED);
	bwcontrast(bwpic,blim,wlim);
	insertsample(bwpic,pic,RED);
	onesample(pic,bwpic,GREEN);
	bwcontrast(bwpic,blim,wlim);
	insertsample(bwpic,pic,GREEN);
	onesample(pic,bwpic,BLUE);
	bwcontrast(bwpic,blim,wlim);
	insertsample(bwpic,pic,BLUE);
	freepic(bwpic);
  return(0);
}
/************************************************************************/
int addcontrast(
		 PICTURE *pic,
		 PICTURE *picout,
		 int blacklimit,
		 int whitelimit,
		 int col)
{
   IMAGE_TYPE *buff = (IMAGE_TYPE *)(pic->data);
   FIMAGE_TYPE *buffout = (FIMAGE_TYPE *)picout->data;
   float scale,ftable[GMAX+1] = { 0.0 };
   int countout,total = 0, smps=pic->samples, sitems=smps*(pic->items);
   int loc,imin=0,imax=GMAX,profile[GMAX+1] = { 0 };
   
   if ((pic->fileid != IMAGE_ID))
		{ (void)fprintf(stderr,"addcontrast: wrong input fileid\n"); return(-1); }
   if (picout->fileid != FIMAGE_ID)
     { (void)fprintf(stderr,"addcontrast: wrong output fileid\n"); return(-1); }
/* compute the histogram  */
   if (smps == 3) buff += col;
   for (countout = 0; countout < sitems; countout+=smps)
       profile[buff[countout]]++;
   while (profile[imin] == 0) imin++; /* find the smallest grey-level */
   while ((profile[imax] == 0) && (imax > imin+1)) imax--; /* and biggest */
/* compute the cumulative histogram */
   for (countout = imin; countout <= imax; countout++)
      {
       profile[countout] += total;      /* cumulative histogram */
       total = profile[countout];
      }
   if (blacklimit)
     while (profile[imin] < blacklimit) imin++;
   if (whitelimit)
     while ((profile[imax-1] > (pic->items-whitelimit)) && (imax > imin+1))
       imax--;
   scale = (float)(imax-imin);
/* construct the table */
  for (countout = imin+1; countout <= imax; countout++)
      ftable[countout] = (countout-imin)/scale;
/* transform the image */
   switch (smps)
     {
     case 1:
      for (countout = 0; countout < sitems; countout++)
	 	{
	   loc = buff[countout];
	   if (loc >= imax) *buffout = 1.0;
	   else if (loc > imin) *buffout = ftable[loc];
	   else *buffout = 0.0;
	   buffout++;
	 	}
      break;
     case 3:
       for (countout = 0; countout < sitems; countout+=3)
	 	{
	   loc = buff[countout];
	   if (loc >= imax) *buffout += 1.0;
	   else if (loc > imin) *buffout += ftable[loc];
	   buffout++;
	 	}
     }
   return(0);
 }
/*********************************************************************/
PICTURE *fcontrast(PICTURE *pic, int blim, int wlim)
{
  PICTURE *picout;
  float *subout;
  int x;
  if (pic->fileid != IMAGE_ID)
  	 {  (void)fprintf(stderr,"fcontrast: wrong input fileid\n"); return(NULL); }
  /* fprintf(stderr,"fcontrast: optimising contrasts - please wait\n"); */
  picout = copypic(pic,1,FIMAGE_ID); /* the same samples  as input */
  subout = (float*)picout->data;
  switch (pic->samples)
    {
    case 1:
      addcontrast(pic,picout,blim,wlim,0);
      break;
    case 3:
      for (x=0; x < picout->items; x++) subout[x] = 0.0;
      addcontrast(pic,picout,blim,wlim,RED);
      addcontrast(pic,picout,blim,wlim,GREEN);
      addcontrast(pic,picout,blim,wlim,BLUE);
      normalise(picout);
      break;
    default:
    	(void)fprintf(stderr,"fcontrast: unexpected samples number\n"); 
	return(NULL); 
    }
  return(picout);
}
/************************************************************************/
int bwhistogram( PICTURE *pic, int blacklimit, int whitelimit)
{
   IMAGE_TYPE *buff = (IMAGE_TYPE *)pic->data;
   float scale;
   int loc,countout,total = 0;
   int imin=0,imax=GMAX,profile[GMAX+1] = { 0 };
	if (((pic->fileid) != IMAGE_ID) || ((pic->samples > 1)))
		{ fprintf(stderr,"bwhistogram: wrong filetype\n"); return(-1); }
/* compute the histogram */
   for (countout = 0; countout < pic->items; countout++)
     profile[buff[countout]]++;
   while (profile[imin] == 0) imin++; /* find the smallest grey-level */
   while ((profile[imax] == 0) && (imax > imin+1)) imax--; /* and biggest */
/* compute the cumulative histogram */
   for (countout = imin; countout <= imax; countout++)
      {
       profile[countout] += total;      /* cumulative histogram */
       total = profile[countout];
      }
   if (blacklimit)
     while (profile[imin] < blacklimit) imin++;
   if (whitelimit)
     while ((profile[imax-1] > (pic->items-whitelimit)) && (imax > imin+1))
       imax--;
   total = profile[imin]; /* number of pixels at the bottom tail-end */
   /* resolution divided by new number of pixels */
   scale = (float) GMAX / (float)(profile[imax]-total);
   /* construct the table */
  for (countout = imin+1; countout <= imax; countout++)
      profile[countout] = ROUND(scale*(profile[countout]-total));
   /* transform the image */
   for (countout = 0; countout < pic->items; countout++)
     {
       loc =  buff[countout];
       if (loc >= imax) buff[countout] = GMAX;
       else if (loc <= imin) buff[countout] = 0;
       else buff[countout] = (IMAGE_TYPE) profile[loc];
     }
#ifdef DEBUG
   {
   for (countout = 0; countout < GMAX; countout++) profile[countout] = 0;
   for (countout = 0; countout < pic->items; countout++)
       profile[buff[countout]]++;
   fprintf(stderr,"\nThe new histogram: ");
   for (countout = 0; countout < GMAX+1; countout++)
       fprintf(stderr,"%d:%d ",countout,profile[countout]);
   fprintf(stderr,"\n");
   }
#endif
   return(0);
 }
/*********************************************************************/
int rgbhistogram( PICTURE *pic, int blacklimit, int whitelimit)
{
   IMAGE_TYPE *buff = (IMAGE_TYPE *)pic->data;
   float scale;
   int loc,countout,total = 0;
   int imin=0,imax=3*GMAX,sitems,profile[3*GMAX+1] = { 0 };
   if (((pic->fileid) != IMAGE_ID) || ((pic->samples != 3)))
		{ fprintf(stderr,"rgbhistogram: wrong filetype\n"); return(-1); }
	sitems = 3*(pic->items);
/* compute the intensity histogram */
   for (countout = 0; countout < sitems; countout += 3)
     (*(profile+*(buff+countout)+*(buff+countout+1)+*(buff+countout+2)))++;
   while (profile[imin] == 0) imin++; /* find the smallest grey-level */
   while ((profile[imax] == 0) && (imax > imin+1)) imax--; /* and biggest */
/* compute the cumulative histogram */
   for (countout = imin; countout <= imax; countout++)
      {
       profile[countout] += total;      /* cumulative histogram */
       total = profile[countout];
      }
   if (blacklimit)
     while (profile[imin] < blacklimit) imin++;
   if (whitelimit)
     while ((profile[imax-1] > (pic->items-whitelimit)) && (imax > imin+1))
       imax--;
   total = profile[imin]; /* number of pixels at the bottom tail-end */
   /* output resolution divided by number of pixels within limits */
   scale = (float) GMAX / (float)(profile[imax]-total);
   /* construct the table */
  for (countout = imin+1; countout <= imax; countout++)
      profile[countout] = ROUND(scale*(profile[countout]-total));
   /* transform the image */
   for (countout = 0; countout < pic->items; countout++)
     {
       loc =  3*buff[countout];
       if (loc >= imax) buff[countout] = GMAX;
       else if (loc <= imin) buff[countout] = 0;
       else buff[countout] = (IMAGE_TYPE) profile[loc];
     }
#ifdef DEBUG
   {
   for (countout = 0; countout < GMAX; countout++) profile[countout] = 0;
   for (countout = 0; countout < pic->items; countout++)
       profile[buff[countout]]++;
   fprintf(stderr,"\nThe new histogram: ");
   for (countout = 0; countout < GMAX+1; countout++)
       fprintf(stderr,"%d:%d ",countout,profile[countout]);
   fprintf(stderr,"\n");
   }
#endif
   return(0);
 }
/*********************************************************************/
int histogram(PICTURE *pic, int blim, int wlim)
{
  if (((pic->fileid) != IMAGE_ID) || ((pic->samples > 3)))
   { fprintf(stderr,"histogram: wrong filetype\n"); return(-1); }
  switch (pic->samples)
  {
    case 1: bwhistogram(pic,blim,wlim); break;
    case 3: rgbhistogram(pic,blim,wlim); break;
    default: fprintf(stderr,"histogram: wrong samples\n"); return(-1);
  }
  return(0);
}
/************************************************************************/
/* Libor Spacek (c) 1989  morphvis.c for the vision library ANSI C
   Morphology Region Growing */

#define min2(x,y) ((x<y)&&((x+mrthr)>y))? x:y
#define max2(x,y) ((x>y)&&(x<(y+mrthr)))? x:y
#define min3(x,y,z) ((min2(x,z)) < (min2(y,z)))? x:(min2(y,z))
#define max3(x,y,z) ((max2(x,z)) > (max2(y,z)))? x:(max2(y,z))
#define minass(x,y) if ((y<x)&&((y+mrthr)>x)) x = y
#define maxass(x,y) if ((y>x)&&(y<(x+mrthr))) x = y
/********************************************************************/
PICTURE *glopic;
IMAGE_TYPE *morbufin, *morbufout;
int mrthr;

int bwmorph(PICTURE *pic,int thr)  /* performs morphology smoothing  */
{
  void swap(void), erode(void), dilate(void), erode8(void), dilate8(void);

  morbufin = (IMAGE_TYPE *)pic->data;
  glopic = pic;
  morbufout = (IMAGE_TYPE *) malloc (glopic->items);

  mrthr = thr;        /* assign static mrthr */
  erode8(); swap();
  erode(); swap();
  erode(); swap();
  dilate8(); swap();
  dilate(); swap();
  dilate(); swap();
  free(morbufout);
  fprintf(stderr,"Morphology operations done\n");
  return(0);
}
/*********************************************************************/
int morph(PICTURE *pic, int thresh)
{
  switch (pic->samples)
  {
    case 1: bwmorph(pic,thresh); break;
    case 3: 
      {
		PICTURE *bwpic;
		bwpic = copypic(pic,1,IMAGE_ID);
		onesample(pic,bwpic,RED);
		bwmorph(bwpic,thresh);
		insertsample(bwpic,pic,RED);
		onesample(pic,bwpic,GREEN);
		bwmorph(bwpic,thresh);
		insertsample(bwpic,pic,GREEN);
		onesample(pic,bwpic,BLUE);
		bwmorph(bwpic,thresh);
		insertsample(bwpic,pic,BLUE);
		freepic(bwpic);
      }
      break;
    default: fprintf(stderr,"morph: wrong filetype\n"); return(-1);
  }
  return(0);
}
/************************************************************************/
void swap(void)

{
  IMAGE_TYPE *swapper;

  swapper = morbufin;
  morbufin = morbufout;
  morbufout = swapper;
}
/***********************************************************************/
void dilate(void)

{
   int sub,row,column;

   morbufout[0] = morbufin[0];
   for (sub=1;sub<glopic->x;sub++)
     {
       morbufout[sub] = max2(morbufin[sub-1],morbufin[sub]);
       maxass(morbufout[sub-1],morbufin[sub]);
     }
   for (row=1;row<glopic->y;row++)
     {
       morbufout[sub] = max2(morbufin[sub-glopic->x],morbufin[sub]);
       maxass(morbufout[sub-glopic->x],morbufin[sub]);
       sub++;
       for (column=1;column<glopic->x;column++)	 {
			morbufout[sub] = 
				max3(morbufin[sub-1],morbufin[sub-glopic->x],morbufin[sub]);
			maxass(morbufout[sub-1],morbufin[sub]);
			maxass(morbufout[sub-glopic->x],morbufin[sub]);
			sub++;
			}
     }
 }
/****************************************************************************/
void erode(void)

{
   int sub,row,column;

   morbufout[0] = morbufin[0];
   for (sub=1;sub<glopic->x;sub++)
     {
       morbufout[sub] = min2(morbufin[sub-1],morbufin[sub]);
       minass(morbufout[sub-1],morbufin[sub]);
     }
   for (row=1;row<glopic->y;row++)
     {
       morbufout[sub] = min2(morbufin[sub-glopic->x],morbufin[sub]);
       minass(morbufout[sub-glopic->x],morbufin[sub]);
       sub++;
       for (column=1;column<glopic->x;column++)
	 {
	   morbufout[sub] = min3(morbufin[sub-1],morbufin[sub-glopic->x],morbufin[sub]);
	   minass(morbufout[sub-1],morbufin[sub]);
	   minass(morbufout[sub-glopic->x],morbufin[sub]);
	   sub++;
	 }
     }

 }
/****************************************************************************/
void dilate8(void)
{
   int sub,row,column;

   morbufout[0] = morbufin[0];
   for (sub=1;sub<glopic->x;sub++)
     {
       morbufout[sub] = max2(morbufin[sub-1],morbufin[sub]);
       maxass(morbufout[sub-1],morbufin[sub]);
     }
   for (row=1;row<glopic->y;row++)
     {
       morbufout[sub] = max3(morbufin[sub-glopic->x],morbufin[sub-glopic->x+1],morbufin[sub]);
       maxass(morbufout[sub-glopic->x],morbufin[sub]);
       maxass(morbufout[sub-glopic->x+1],morbufin[sub]);
       sub++;
       for (column=1;column<glopic->x;column++)
	 {
	   morbufout[sub] = max3(morbufin[sub-1],morbufin[sub-glopic->x],morbufin[sub]);
	   if ((column+1) == glopic->x)
	     morbufout[sub] = max2(morbufout[sub],morbufin[sub-glopic->x-1]);
	   else
	   {
	     morbufout[sub] = 
	     max3(morbufout[sub],morbufin[sub-glopic->x-1],morbufin[sub-glopic->x+1]);
	     maxass(morbufout[sub-glopic->x+1],morbufin[sub]);
	   }
	   maxass(morbufout[sub-1],morbufin[sub]);
	   maxass(morbufout[sub-glopic->x],morbufin[sub]);
	   maxass(morbufout[sub-glopic->x-1],morbufin[sub]);
	   sub++;
	 }
     }

 }
/****************************************************************************/
void erode8(void)
{
   int sub,row,column;

   morbufout[0] = morbufin[0];
   for (sub=1;sub<glopic->x;sub++)
     {
       morbufout[sub] = min2(morbufin[sub-1],morbufin[sub]);
       minass(morbufout[sub-1],morbufin[sub]);
     }
   for (row=1;row<glopic->y;row++)
     {
       morbufout[sub] = min3(morbufin[sub-glopic->x],morbufin[sub-glopic->x+1],morbufin[sub]);
       minass(morbufout[sub-glopic->x],morbufin[sub]);
       minass(morbufout[sub-glopic->x+1],morbufin[sub]);
       sub++;
       for (column=1;column<glopic->x;column++)
	 {
	   morbufout[sub] = min3(morbufin[sub-1],morbufin[sub-glopic->x],morbufin[sub]);
	   if ((column+1) == glopic->x)
	     morbufout[sub] = min2(morbufout[sub],morbufin[sub-glopic->x-1]);
	   else
	   {
	     morbufout[sub] = 
	     min3(morbufout[sub],morbufin[sub-glopic->x-1],morbufin[sub-glopic->x+1]);
	     minass(morbufout[sub-glopic->x+1],morbufin[sub]);
	   }
	   minass(morbufout[sub-1],morbufin[sub]);
	   minass(morbufout[sub-glopic->x],morbufin[sub]);
	   minass(morbufout[sub-glopic->x-1],morbufin[sub]);
	   sub++;
	 }
     }
 }
/****************************************************************************/
