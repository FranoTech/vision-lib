/* Libor Spacek, (C) 2009
   PAINT.C CREATES AN ARTIFICIAL IMAGE with added zero mean normal noise */

#include <lvl.h>
#ifndef M_PI
#define M_PI 3.141592653589
#endif
#define SIGMA 128.0f
#define GAUSSCONST ((float)(SIGMA*sqrt((2.f*M_PI))))
#define GAUSSDENOM ((float)(2.f*SIGMA*SIGMA)) /* for faster Gauss */
#define RANDRANGE 1000 /* rectangular pdf random numbers resolution */
#define PDFPTS 1000 /* normal probability density function resolution */
#define BRIGHT 191
#define DARK 64
#define SQUARESZ 50
#define SQUARENO 8
#define IMSIZE SQUARENO*SQUARESZ
#define ROUND(x) ((int)(floor ((x) + 0.5f)))

/* #define DEBUG */

static int invcdf[RANDRANGE], noisy = 0;

/****************************************************************************/
float gauss (int x)  /*  the Gaussian - standardised normal distribution */
{ return (float)( exp(-x*x/GAUSSDENOM) / GAUSSCONST ); }
/****************************************************************************/
/* Systematically inverts any quantised monotonic function,
	in this case the cumulative (probability density) function cdfvec of the
	normal distribution. Other distributions could be used just by replacing
	the function gauss above */ 

void invertcdf() 
{
  float cdfvec[PDFPTS];
  register int count, u=0;

   /* compute half of the cumulative normal distribution */
	cdfvec[0] = 1.f/GAUSSCONST;
 	for (count=1; count < PDFPTS; count++)
		cdfvec[count] = cdfvec[count-1]+2.f*gauss(count);
#ifdef DEBUG 
	(void)fprintf(stderr,"%e\n",cdfvec[PDFPTS-1]);
#endif   
	cdfvec[PDFPTS-1] = 1.f; /* prevent inf loop u++ below */
    
  /* compute the inverse cumulative normal distribution */
  invcdf[0] = 0;	
  for (count=1; count < RANDRANGE; count++)
	{
	while ( cdfvec[u] < (float)count/(float)RANDRANGE ) u++;
	invcdf[count] = ROUND((float)GMAX*u/PDFPTS);	
#ifdef DEBUG 
		(void)fprintf(stderr,"%d ",invcdf[count]);
#endif  
	}     
}
/****************************************************************************/
int pixelvalue (int ind)  /* chessboard squares */
{ if ( ind%2 > 0 ) return(DARK); else return(BRIGHT); }
/**************************************************************************/

int main (int argc,char *argv[])

{ PICTURE *pic;
  IMAGE_TYPE *buf;
  register int count;
  int x,y,r,value,noisy=0;

  switch (argc) {
    case 3: noisy = atoi(argv[2]); argc--; 
    case 2: FILEOUT = argv[1]; argc--;
    case 1: PROGNAME = argv[0]; break;
    default:
      fprintf (stderr,"usage: %s outputfile [noisy]\n", argv[0]);
      exit(1);  }

	if (noisy < 2)
	{
  	pic = makepic(IMAGE_ID,IMSIZE,IMSIZE,0.0f,0.0f,IMSIZE*IMSIZE,1,(char *)"Artificial squares");
  	buf = (IMAGE_TYPE *)pic->data;
	}
	else
	{
  	pic = makepic(IMAGE_ID,IMSIZE,IMSIZE,0.0f,0.0f,IMSIZE*IMSIZE,3,(char *)"Artificial rgb squares");
  	buf = (IMAGE_TYPE *)pic->data;
	}

  /* precompute the inverse cumulative normal distribution */
  invertcdf();

  /* fill the image */
  	count = 0;
	switch (noisy) {
		case 0: /* plain bw */
		  for (y = 0;  y < pic->y;  y++)
		  	for (x = 0;  x < pic->x; x++)
  				buf[count++] = (unsigned char)pixelvalue(x/SQUARESZ+y/SQUARESZ);
			break;
		case 1: /* noisy bw */			
		  for (y = 0;  y < pic->y;  y++)
		   for (x = 0;  x < pic->x; x++)
		   {
				value = pixelvalue(x/SQUARESZ+y/SQUARESZ);
				r = invcdf[rand()%RANDRANGE];
				if (rand()%2) value += r; else value -= r;
				if (value > GMAX) value = GMAX; else if (value < 0) value = 0;
				buf[count++] = (unsigned char) value;
   	   }
			break;
		case 2: /* plain rgb */
		  for (y = 0;  y < pic->y;  y++)
		  	for (x = 0;  x < pic->x; x++)
				{
  				buf[count++] = (unsigned char)pixelvalue(x/SQUARESZ);
				buf[count++] = (unsigned char)pixelvalue(y/SQUARESZ);
				buf[count++] = (unsigned char)pixelvalue((x+y)/SQUARESZ);
				}
			break;
		case 3: /* noisy rgb */			
		  for (y = 0;  y < pic->y;  y++)
		    for (x = 0;  x < pic->x; x++)
		    	{
				value = pixelvalue(x/SQUARESZ);
				r = invcdf[rand()%RANDRANGE];
				if (rand()%2) value += r; else value -= r;
				if (value > GMAX) value = GMAX;
				else if (value < 0) value = 0;
				buf[count++] = (unsigned char)value;
				value = pixelvalue(y/SQUARESZ);
				r = invcdf[rand()%RANDRANGE];
				if (rand()%2) value += r; else value -= r;
				if (value > GMAX) value = GMAX;
				else if (value < 0) value = 0;
				buf[count++] = (unsigned char)value;
				value = pixelvalue((x+y)/SQUARESZ);
				r = invcdf[rand()%RANDRANGE];
				if (rand()%2) value += r; else value -= r;
				if (value > GMAX) value = GMAX;
				else if (value < 0) value = 0;
				buf[count++] = (unsigned char)value;
   	  		}
			break;
		default: 
			fprintf (stderr," %s: third program arg %d out of range!\n", argv[0], noisy);
			exit(1);  }

  writepmpic(pic,FILEOUT);
  printf ("%s finished, %s written successfully\n",argv[0],argv[1]);
  return 0;
}
