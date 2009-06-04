/* Libor Spacek (C) 2009 tobw.c */
#include <lvl.h>
/********************************************************************/
int main(int argc, char *argv[])
{
	PICTURE *pic,*picout;
	dofiles(argc,argv);
	if ((pic = readpic(FILEIN)) == NULL) 
	{ (void)fprintf(stderr,"%s: failed to read input file\n",PROGNAME);
	  exit(1); }
	if ((pic->fileid != IMAGE_ID) || (pic->samples < 2))
  	{ (void)fprintf(stderr,"%s: wrong type of input file\n",PROGNAME); 
	  exit(1); }
	picout = tobwf(pic); /* does the work */
	freepic(pic);
	pic = ftoim(picout,0.1f,0.9f,0); /* normalised & quantised */
	writepmpic(pic,FILEOUT); /* could write other formats */
	exit(0);
}
/********************************************************************/
