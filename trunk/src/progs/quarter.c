/* Libor Spacek (C) 1992 quarter.c */
#include <lvl.h>

int main(int argc, char *argv[])
{
  PICTURE *pic, *picout;
  dofiles(argc,argv);
  if ((pic = readpic(FILEIN)) == NULL) exit(1);
  if ((pic->fileid)!= IMAGE_ID)
	{
  	(void)fprintf(stderr,"%s: wrong type of input file\n", argv[0]);
  	exit(1);
  	}
  picout = quarter(pic);
  writepmpic(picout,FILEOUT);
  return 0;
}
/*********************************************************************/




