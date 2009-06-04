/* NEWEDGELIB.C Edgefinder for the LVL Library (c) Libor Spacek 2009  
   this code looks a bit complicated but there is a reason for it: 
   it avoids bounds checking whenever possible */
/*************************************************************************/
#include "lvl.h"
/*************************************************************************
	Computes the sum of pixels of an area given by four corner addresses 
   (in summedimage) used by function edgegrad */ 
static inline int area(int *se, int *ne, int *nw, int *sw)
{ return( *se - *sw - *ne + *nw ); }
/***********************************************************************/
static inline float maxabs3(float a1, float a2, float a3)
{ if (fabs(a2) > fabs(a1)) a1 = a2;
  if (fabs(a3) > fabs(a1)) a1 = a3;
  return(a1); }
static inline float minabs3(float a1, float a2, float a3)
{ if (fabs(a2) < fabs(a1)) a1 = a2;
  if (fabs(a3) < fabs(a1)) a1 = a3;
  return(a1); }
/***********************************************************************/ 
PICTURE *sumimage(PICTURE *pic)
/* creates a (2D) integral image for fast summation of pixels
	e.g. for convolution with constant weight operators.
	It is also an example of using int output pixel type */
{
PICTURE *picout;
IMAGE_TYPE *bfi = (IMAGE_TYPE *)pic->data;
INT_TYPE *bfo;
int x,y,s, sampls = pic->samples, rowjump = sampls*(pic->x);
if ((pic->fileid)!= IMAGE_ID)
	{ fprintf(stderr,"Wrong file passed to sumimage\n"); exit(1); }
picout = copypic(pic,sampls,INT_ID);
bfo = (INT_TYPE *)picout->data;
for (s=0; s<sampls; s++) *(bfo++) = (int)(*(bfi++)); /* 1st pixel */
for (x=1; x<(pic->x); x++) /* 1st row */
	for (s=0; s<sampls; s++) { *bfo = *bfi + *(bfo-sampls); bfo++; bfi++; }
for (y=1; y<(pic->y); y++)
	{ 
	for (s=0; s<sampls; s++) /*1st column*/
		{*bfo = *bfi + *(bfo-rowjump); bfo++; bfi++;}
	for (x=1; x<(pic->x); x++) /* all pixels not in first row or column */
		for (s=0; s<sampls; s++) 
		{
			*bfo=*bfi+*(bfo-sampls)+*(bfo-rowjump)-*(bfo-rowjump-sampls);
			bfo++; bfi++; }
		}
	return(picout);
}
/**********************************************************************/
PICTURE *sumimage1d(PICTURE *pic)
/* (1D integral) image for fast summation of pixels within linesegments, 
   e.g. for convolution with circular or irregular shaped 
   constant weight operators.  */
{
  PICTURE *picout;
  IMAGE_TYPE *bfi = (IMAGE_TYPE *) pic->data;
  POSINT_TYPE *bfo;
  int x, sampls = pic->samples, psumr = 0, psumg = 0, psumb = 0;
  picout = copypic(pic,sampls,POSINT_ID); 
  bfo = (POSINT_TYPE *) picout->data;
  for (x = 0; x < pic->items; x++)
  {
	psumr += *bfi++;
	*bfo++ = psumr;
	if ( sampls == 3 )
 	 {
	   psumg += *bfi++;
	   *bfo++ = psumg;
	   psumb += *bfi++;
	   *bfo++ = psumb;
	 }
  }
  return(picout);
}
/**********************************************************************/
/* takes sumimage, opside as input and produces primitive gradients */
PICTURE *edgegrad (PICTURE *pic, int opside)
{
PICTURE *picout;
INT_TYPE *bfi = (INT_TYPE *)pic->data,*rowa,*a,*b,*c,*d,*e,*f,*g,*h;
FVEC_TYPE *bfo;
int x,y,s,sampls=pic->samples,rowjump=sampls*(pic->x),opsmm,opsq,stepx,stepy;
float norm;
if ((pic->fileid)!= INT_ID)
	{ (void)fprintf(stderr,"edgegrad: wrong type of input pic\n"); exit(1); }
if (opside%2) opside++; /* opside should be even number of pixels */
if (opside < 2) opside = 2;  /* minimum workable value */
stepx = sampls*opside;
stepy = rowjump*opside;
opsmm = opside-1;
pic->x -= opsmm;
pic->y -= opsmm;
pic->xorigin = (float)opsmm / (float)2.0;
pic->yorigin = (float)opsmm / (float)2.0;
picout = copypic(pic,sampls,FVEC_ID);
bfo = (FVEC_TYPE *)picout->data;
opsq = opside*opside;
rowa = a = bfi-rowjump-sampls;
d = a + stepy/2 + stepx;
e = a + stepy + stepx; /* initial se corner position */
f = a + stepy + stepx/2;
for (s=0; s<sampls; s++) 
	{  norm = (float)(*e) + opsq; /* add 1 in case all pixels are zero */
		(*bfo).fx = (*e - 2*(*f)) / norm;
		(*bfo).fy = (2*(*d) - *e) / norm;
		bfo++; a++; d++; e++; f++; }
g = a + stepy; /* now non-zero */
h = a + stepy/2;
for (x=1; x<(pic->x); x++) /* 1st row */
  { 
	 for (s=0; s<sampls; s++) 
	 { norm = (float)(*e - *g) + opsq;
		(*bfo).fx = (*e - 2*(*f) + *g) / norm;
		(*bfo).fy = (2*(*d-(*h)) - (*e) + *g)/ norm;
		bfo++; d++; e++; f++; g++; h++; }
  }
for (y=1; y<(pic->y); y++)
  {
  rowa += rowjump; a = rowa;
  b = a + stepx/2;
  c = a + stepx;
  d = a + stepy/2 + stepx;
  e = a + stepy + stepx; /* initial se corner position */
  f = a + stepy + stepx/2; 
	for (s=0; s<sampls; s++) /*1st column*/
	{	norm = (float)(*e - *c) + opsq;
		(*bfo).fx = (*e - 2*(*f) + 2*(*b) -(*c)) / norm;
		(*bfo).fy = (2*(*d) - *e - *c) / norm;
		bfo++; a++; b++; c++; d++; e++; f++; }
   g = a + stepy; /* now non-zero */
   h = a + stepy/2;
	for (x=1; x<(pic->x); x++)
		for (s=0; s<sampls; s++) 
		{	norm = (float)area(e,c,a,g) + opsq;
			(*bfo).fx = (area(e,c,b,f)-area(f,b,a,g)) / norm;
			(*bfo).fy = (area(d,c,a,h)-area(e,d,h,g)) / norm;
			bfo++; a++; b++; c++; d++; e++; f++; g++; h++; }
	}
return(picout);
}
/**********************************************************************
produces fast raw gradients (the best 4x4 operator) */
PICTURE *fastedgegrad (PICTURE *pic, int opside)
{
PICTURE *picout;
IMAGE_TYPE *bfi = (IMAGE_TYPE *)pic->data,*row1,*row2,*row3,*row4;
FVEC_TYPE *bfo;
int x,y,s,sampls=pic->samples,rowjump=sampls*(pic->x),opsmm,ppq,npq,nnq,pnq;
float norm;
if ((pic->fileid)!= IMAGE_ID)
	{ (void)fprintf(stderr,"fastedgegrad: wrong type of input pic\n"); exit(1); }
if (opside%2) opside++; /* opside should be even number of pixels */
if (opside < 2) opside = 2;  /* minimum workable value */
opsmm = opside-1;
pic->x -= opsmm;
pic->y -= opsmm;
pic->xorigin = (float)opsmm / (float)2.0;
pic->yorigin = (float)opsmm / (float)2.0;
picout = copypic(pic,sampls,FVEC_ID);
bfo = (FVEC_TYPE *)picout->data;
row1=bfi; row2=row1+rowjump; row3=row2+rowjump; row4=row3+rowjump;
if (sampls==1) /*b/w is treated as a special case to make it a bit faster */
{
 for (y=0; y<(pic->y); y++)
  {
	for (x=0; x<(pic->x); x++)
		{	
			npq = *(row1)+*(row2)+*(row1+1)+*(row2+1);
			nnq = *(row3)+*(row4)+*(row3+1)+*(row4+1);
			pnq = *(row3+2)+*(row4+2)+*(row3+3)+*(row4+3);
			ppq = *(row1+2)+*(row2+2)+*(row1+3)+*(row2+3);
			norm = (float)(npq + nnq + pnq + ppq + 4);
			(*bfo).fx = (ppq+pnq-npq-nnq) / norm;
			(*bfo).fy = (ppq+npq-pnq-nnq) / norm;
			bfo++; row1++; row2++; row3++; row4++;
		}
  row1+=opsmm; row2+=opsmm; row3+=opsmm; row4+=opsmm; /* end of the row jump */
  }
} else /* rgb etc, multiply the input index by sampls to "jump" one pixel */
  for (y=0; y<(pic->y); y++)
  {
	for (x=0; x<(pic->x); x++)
		for (s=0; s<sampls; s++) 
		{  
			npq = *row1+*row2+*(row1+sampls)+*(row2+sampls);
			nnq = *row3+*row4+*(row3+sampls)+*(row4+sampls);
			ppq = *(row1+2*sampls)+*(row2+2*sampls)+*(row1+3*sampls)+*(row2+3*sampls);
			pnq = *(row3+2*sampls)+*(row4+2*sampls)+*(row3+3*sampls)+*(row4+3*sampls);
			norm = (float)(npq + nnq + pnq + ppq + 4);
			(*bfo).fx = (ppq+pnq-npq-nnq) / norm;
			(*bfo).fy = (ppq+npq-pnq-nnq) / norm;
			bfo++; row1++; row2++; row3++; row4++;
		}
  row1+=opsmm*sampls;row2+=opsmm*sampls;row3+=opsmm*sampls;row4+=opsmm*sampls;
  /* jump to the end of the row */
  }
return(picout);
}
/**********************************************************************/
PICTURE *grad(PICTURE *pic,int opside)
{ /* uses integral image (sumimage; edgegrad) for bigger operators and simple
     fastedgegrad for ops up to side 4  - and frees memory */ 
PICTURE *summedpic, *edgepic;
if (opside>4)
  { summedpic = sumimage(pic); freepic(pic);
    edgepic = edgegrad(summedpic,opside); freepic(summedpic); }
else 
  { edgepic = fastedgegrad(pic,opside); freepic(pic); }
return(edgepic); 
}
/**********************************************************************/
/* Produce and normalise gradient vector magnitudes for display purposes */
/* uses simple mahabolis grad magnitude (absolute values) and frees input pic */
PICTURE *gradmag(PICTURE *pic)
{
PICTURE *picout;
FVEC_TYPE *bfi = (FVEC_TYPE *)pic->data;
FIMAGE_TYPE *bfo,vfx,vfy;
int x, sitems;
if ((pic->fileid)!= FVEC_ID)
	{ (void)fprintf(stderr,"gradmag: wrong type of input pic\n"); exit(1); }
sitems = (pic->samples)*(pic->items);
picout = copypic(pic,(pic->samples),FIMAGE_ID);
bfo = (float*)picout->data;
for (x=0; x<sitems; x++) 
  { 
  vfx = (*bfi).fx;
  vfy = (*bfi).fy;
  *(bfo++) = ((vfx < 0.0)?(-vfx):(vfx))+((vfy < 0.0)?(-vfy):(vfy));
	bfi++; 
  }
freepic(pic);
return(picout);
}
/**********************************************************************/
/* Select maximum gradient vector components and free input pic */
PICTURE *gradmax(PICTURE *pic)
{
PICTURE *picout;
FVEC_TYPE *bfi = (FVEC_TYPE *)pic->data;
FVEC_TYPE *bfo;
int x, s, items, samps;
float maxx, maxy, fx, fy;
if (((pic->fileid)!= FVEC_ID) || ((pic->samples) < 2))
	{ (void)fprintf(stderr,"gradmax: wrong type of input pic\n"); exit(1); }
samps = pic->samples;
items = pic->items;
picout = copypic(pic,1,FVEC_ID); /* only one sample per pixel output */
bfo = (FVEC_TYPE *)picout->data;
for (x=0; x<items; x++) 
 { maxx = (*bfi).fx;
   maxy = (*(bfi++)).fy;
   for (s=1; s<samps; s++)	/* global maximum over all the samples */
   {  fx = (*bfi).fx;
      fy = (*(bfi++)).fy;
      if (fabs(fx) > fabs(maxx)) maxx = fx;
      if (fabs(fy) > fabs(maxy)) maxy = fy; }
  (*bfo).fx = maxx; (*(bfo++)).fy = maxy; 
 }
freepic(pic);
return(picout);
}
/**********************************************************************/
/* Vector Morphology */
/**********************************************************************/
PICTURE *morphgrads(PICTURE *pic)
{
PICTURE *picout;
FVEC_TYPE *bfi = (FVEC_TYPE *)pic->data;

if ((pic->fileid)!= FVEC_ID)
	{ (void)fprintf(stderr,"morphgrads: wrong type of input pic\n"); exit(1); }
picout = copypic(pic,pic->samples,FVEC_ID);
erodegrads(pic,picout);
dilategrads(picout,pic);
freepic(picout);
return(pic);
}
/**********************************************************************/
void erodegrads(PICTURE *pic, PICTURE *picout)
{
FVEC_TYPE *bfi = (FVEC_TYPE *)pic->data;
FVEC_TYPE *bfo = (FVEC_TYPE *)picout->data, zerog;
int x,y,s,sampls=pic->samples,rowjump;
float maxg;
if (((pic->fileid)!= FVEC_ID)||((picout->fileid)!= FVEC_ID))
	{ (void)fprintf(stderr,"dilategrads: wrong type of input pic\n"); exit(1); }
rowjump = sampls*(pic->x);
zerog.fx = 0.0; zerog.fy = 0.0;
for (s=0; s<sampls; s++)    /* 1st pixel */
   { maxg  = modfvec(bfi); *bfo = zerog;
     if (maxg < fabs((bfi+sampls)->fx))  { bfo++; bfi++; continue; }
     if (maxg < fabs((bfi+rowjump)->fy)) { bfo++; bfi++; continue; }
     *bfo = *bfi; bfo++; bfi++; }
for (x=2; x<(pic->x); x++)  /* 1st row */ 
  for (s=0; s<sampls; s++) 
   { maxg  = modfvec(bfi); *bfo = zerog;
     if (maxg < fabs((bfi+sampls)->fx))  { bfo++; bfi++; continue; }
     if (maxg < fabs((bfi+rowjump)->fy)) { bfo++; bfi++; continue; } 
     if (maxg < fabs((bfi-sampls)->fx))  { bfo++; bfi++; continue; } 
	  *bfo = *bfi; bfo++; bfi++; }  
for (s=0; s<sampls; s++) /* last pixel in 1st row */
   { maxg  = modfvec(bfi); *bfo = zerog;
     if (maxg < fabs((bfi+rowjump)->fy)) { bfo++; bfi++; continue; }
	  if (maxg < fabs((bfi-sampls)->fx))  { bfo++; bfi++; continue; }
 	  *bfo = *bfi; bfo++; bfi++; }
for (y=2; y<(pic->y); y++) /* all inner rows */
	{ 
	for (s=0; s<sampls; s++) /* 1st column */
	{ maxg  = modfvec(bfi); *bfo = zerog;
     if (maxg < fabs((bfi+sampls)->fx))  { bfo++; bfi++; continue; }
	  if (maxg < fabs((bfi+rowjump)->fy)) { bfo++; bfi++; continue; }
	  if (maxg < fabs((bfi-rowjump)->fy)) { bfo++; bfi++; continue; } 
  	  *bfo = *bfi; bfo++; bfi++; }
	for (x=2; x<(pic->x); x++) /* all inner pixels */
	  for (s=0; s<sampls; s++) 
		{ maxg  = modfvec(bfi); *bfo = zerog;
        if (maxg < fabs((bfi+sampls)->fx))  { bfo++; bfi++; continue; } 
        if (maxg < fabs((bfi-sampls)->fx))  { bfo++; bfi++; continue; }   
        if (maxg < fabs((bfi+rowjump)->fy)) { bfo++; bfi++; continue; }   
        if (maxg < fabs((bfi-rowjump)->fy)) { bfo++; bfi++; continue; } 
   	  *bfo = *bfi; bfo++; bfi++; }
	for (s=0; s<sampls; s++) /* last column */
	{ maxg  = modfvec(bfi); *bfo = zerog;
     if (maxg < fabs((bfi-sampls)->fx)) { bfo++; bfi++; continue; } 
     if (maxg < fabs((bfi+rowjump)->fy)) { bfo++; bfi++; continue; } 
     if (maxg < fabs((bfi-rowjump)->fy)) { bfo++; bfi++; continue; } 
	  *bfo = *bfi; bfo++; bfi++; } 
	}
for (s=0; s<sampls; s++) /* first pixel in the last row */
   { maxg  = modfvec(bfi); *bfo = zerog;
     if (maxg < fabs((bfi-rowjump)->fy)) { bfo++; bfi++; continue; }
     if (maxg < fabs((bfi+sampls)->fx))  { bfo++; bfi++; continue; }
	  *bfo = *bfi; bfo++; bfi++; }

for (x=2; x<(pic->x); x++)  /* last row */ 
  for (s=0; s<sampls; s++) 
   { maxg  = modfvec(bfi); *bfo = zerog;
     if (maxg < fabs((bfi+sampls)->fx)) { bfo++; bfi++; continue; }
	  if (maxg < fabs((bfi-rowjump)->fy)) { bfo++; bfi++; continue; }
     if (maxg < fabs((bfi-sampls)->fx)) { bfo++; bfi++; continue; } 
     *bfo = *bfi; bfo++; bfi++; }
for (s=0; s<sampls; s++) /* last pixel in last row */
   { maxg  = modfvec(bfi); *bfo = zerog;
     if (maxg < fabs((bfi-rowjump)->fy)) { bfo++; bfi++; continue; } 
     if (maxg < fabs((bfi-sampls)->fx))  { bfo++; bfi++; continue; }
     *bfo = *bfi; bfo++; bfi++; }
}
/**********************************************************************/
void dilategrads(PICTURE *pic, PICTURE *picout)
{
FVEC_TYPE *bfi = (FVEC_TYPE *)pic->data;
FVEC_TYPE *bfo = (FVEC_TYPE *)picout->data;
int x,y,s,sampls=pic->samples,rowjump;

if (((pic->fileid)!= FVEC_ID)||((picout->fileid)!= FVEC_ID))
	{ (void)fprintf(stderr,"dilategrads: wrong type of input pic\n"); exit(1); }
rowjump = sampls*(pic->x);
for (s=0; s<sampls; s++)    /* 1st pixel */
   { *bfo = *bfi;
     	if (fabs(bfo->fy) < fabs((bfi+sampls)->fy)) (bfo->fy) = (bfi+sampls)->fy;
     	if (fabs(bfo->fx) < fabs((bfi+rowjump)->fx))(bfo->fx) = (bfi+rowjump)->fx;
      bfo++; bfi++; }
for (x=2; x<(pic->x); x++)  /* 1st row */ 
  for (s=0; s<sampls; s++) 
   {  *bfo = *bfi;
     	if (fabs(bfo->fy) < fabs((bfi+sampls)->fy)) (bfo->fy) = (bfi+sampls)->fy;
     	if (fabs(bfo->fx) < fabs((bfi+rowjump)->fx))(bfo->fx) = (bfi+rowjump)->fx;
		if (fabs(bfo->fy) < fabs((bfi-sampls)->fy)) (bfo->fy) = (bfi-sampls)->fy;
      bfo++; bfi++; }
for (s=0; s<sampls; s++) /* last pixel in 1st row */
   {  *bfo = *bfi;
     	if (fabs(bfo->fx) < fabs((bfi+rowjump)->fx))(bfo->fx) = (bfi+rowjump)->fx;
		if (fabs(bfo->fy) < fabs((bfi-sampls)->fy)) (bfo->fy) = (bfi-sampls)->fy;
      bfo++; bfi++; }
for (y=2; y<(pic->y); y++) /* all inner rows */
	{ 
	for (s=0; s<sampls; s++) /* 1st column */
   {  *bfo = *bfi;
     	if (fabs(bfo->fy) < fabs((bfi+sampls)->fy)) (bfo->fy) = (bfi+sampls)->fy;
     	if (fabs(bfo->fx) < fabs((bfi+rowjump)->fx))(bfo->fx) = (bfi+rowjump)->fx;
		if (fabs(bfo->fx) < fabs((bfi-rowjump)->fx)) (bfo->fx) = (bfi-rowjump)->fx;
      bfo++; bfi++; }
	for (x=2; x<(pic->x); x++) /* all inner pixels */
	  for (s=0; s<sampls; s++) 
   {  *bfo = *bfi;
     	if (fabs(bfo->fy) < fabs((bfi+sampls)->fy)) (bfo->fy) = (bfi+sampls)->fy;
     	if (fabs(bfo->fx) < fabs((bfi+rowjump)->fx))(bfo->fx) = (bfi+rowjump)->fx;
		if (fabs(bfo->fx) < fabs((bfi-rowjump)->fx)) (bfo->fx) = (bfi-rowjump)->fx;
		if (fabs(bfo->fy) < fabs((bfi-sampls)->fy)) (bfo->fy) = (bfi-sampls)->fy;
      bfo++; bfi++; }
	for (s=0; s<sampls; s++) /* last column */
   {  *bfo = *bfi;
     	if (fabs(bfo->fx) < fabs((bfi+rowjump)->fx))(bfo->fx) = (bfi+rowjump)->fx;
		if (fabs(bfo->fx) < fabs((bfi-rowjump)->fx)) (bfo->fx) = (bfi-rowjump)->fx;
		if (fabs(bfo->fy) < fabs((bfi-sampls)->fy)) (bfo->fy) = (bfi-sampls)->fy;
      bfo++; bfi++; }	
	}	
for (s=0; s<sampls; s++) /* first pixel in the last row */
    {  *bfo = *bfi;
     	if (fabs(bfo->fy) < fabs((bfi+sampls)->fy)) (bfo->fy) = (bfi+sampls)->fy;
		if (fabs(bfo->fx) < fabs((bfi-rowjump)->fx)) (bfo->fx) = (bfi-rowjump)->fx;
      bfo++; bfi++; }
for (x=2; x<(pic->x); x++)  /* last row */ 
  for (s=0; s<sampls; s++) 
   {  *bfo = *bfi;
     	if (fabs(bfo->fy) < fabs((bfi+sampls)->fy)) (bfo->fy) = (bfi+sampls)->fy;
		if (fabs(bfo->fx) < fabs((bfi-rowjump)->fx)) (bfo->fx) = (bfi-rowjump)->fx;
		if (fabs(bfo->fy) < fabs((bfi-sampls)->fy)) (bfo->fy) = (bfi-sampls)->fy;
      bfo++; bfi++; }
for (s=0; s<sampls; s++) /* last pixel in last row */
   {  *bfo = *bfi;
		if (fabs(bfo->fx) < fabs((bfi-rowjump)->fx)) (bfo->fx) = (bfi-rowjump)->fx;
		if (fabs(bfo->fy) < fabs((bfi-sampls)->fy)) (bfo->fy) = (bfi-sampls)->fy;
      bfo++; bfi++; }
}
/**********************************************************************/
