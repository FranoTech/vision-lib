/* Libor Spacek (c) 1997 contrast.c */
#include <lvl.h>
#include <stdlib.h>
/********************************************************************/
int main(int argc, char *argv[])
{
  PICTURE *pic;
  int bperc = 5, wperc = 5, sz;
  vmessage();
  switch (argc) {
    case 5: wperc = atoi(argv[4]);
    case 4: bperc = atoi(argv[3]);
    case 3: case 2: dofiles(argc,argv);
 /*  fprintf(stderr,"%s %d %d %s %s\n",PROGNAME,bperc,wperc,FILEIN,FILEOUT); */
		break;
    default: (void)fprintf(stderr,
	     "usage: contrast infile outfile [blackpercent][whitepercent]\n");
      exit(1);  }
  if ((pic = readpic(FILEIN)) == NULL) exit(1);
  if ((pic->fileid) != IMAGE_ID)
	{ (void)fprintf(stderr,"%s: wrong type of input file\n",PROGNAME);
      exit(1);
   }
	sz = (pic->items)*(pic->samples);
  contrast(pic,bperc*sz/100,wperc*sz/100);
  writepmpic(pic,FILEOUT);
  exit(0);
}
/*********************************************************************/




