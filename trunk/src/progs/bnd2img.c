/* Libor Spacek (C) 2009 bnd2img.c  */
#include "lvl.h"

int main(int argc, char *argv[])
{ PICTURE *fpic, *pic;
  dofiles(argc,argv); 
  if ((pic = readpic(FILEIN)) == NULL) exit(1);
  if (pic->fileid != BND_ID)
    { fprintf(stderr,"%s: wrong type of file\n",PROGNAME); exit(1); }
  fpic = bndtof(pic);
  freepic(pic);
  // nonmax(fpic,0.01);
  pic = ftoim(fpic,0.1f,0.9f,1); /* linearly transform and invert */
  writepmpic(pic,FILEOUT);
  exit(0);
}
/*********************************************************************/




