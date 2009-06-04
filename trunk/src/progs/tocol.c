/* Libor Spacek (C) 1993 tocol.c */
#include <lvl.h>
#include <string.h>
/********************************************************************/
int main(int argc, char *argv[])
{
  PICTURE *pic, *pic1;
  char fout[20];
  if (argc != 3)
    {
      fprintf(stderr,"usage: %s: infile outfile\n",argv[0]);
      exit(1);
    }
  dofiles(argc,argv);
  if ((pic = readpic(FILEIN)) == NULL) exit(1);
  if ((pic->samples != 3)||(pic->fileid != IMAGE_ID))
    {
      fprintf(stderr,"%s: wrong type of input file\n",PROGNAME);
      exit(1);
    }
  pic1 = copypic(pic,1,IMAGE_ID);
  onesample(pic,pic1,RED);
  strcpy(fout,FILEOUT);
  strcat(fout,"-r.pgm");
  writepmpic(pic1,fout);
  onesample(pic,pic1,GREEN);
  strcpy(fout,FILEOUT);
  strcat(fout,"-g.pgm");
  writepmpic(pic1,fout);
  onesample(pic,pic1,BLUE);
  strcpy(fout,FILEOUT);
  strcat(fout,"-b.pgm");
  writepmpic(pic1,fout);
  exit(0);
}
/*********************************************************************/
