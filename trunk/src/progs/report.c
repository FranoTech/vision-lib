/* Libor Spacek (C) 1991 report.c for report */
#include <lvl.h>
int main(int argc,char *argv[])
{
  PICTURE *pic = newpic();
  FILE *fp;

  dofiles(argc,argv);
  if (FILEIN == NULL) 
    fp = stdin;
  else    
    if ((fp = fopen(FILEIN,"rb")) == NULL )
      { fprintf(stderr,"%s: wrong input file %s\n",PROGNAME,FILEIN);
			exit(1); }
  readhead(pic,fp);
  fclose(fp);
  exit(0);
}
