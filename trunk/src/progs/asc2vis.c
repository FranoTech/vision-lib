/* Libor Spacek (C) 1994 asc2vis.c 
   conversion from asci floats to vision binary file */
#include "lvl.h"
/********************************************************************/
int main(int argc, char *argv[])
{
  PICTURE *pic;
  int id=IMAGE_ID,x=100,y=100,s=1;

  dofiles(3,argv);
	if (argc<4) 
		 if (  (pic = readpic(FILEIN)) != NULL) { writepic(pic,FILEOUT); exit(0); }
  (void)fprintf(stderr,"%s trying to read headerless ascii file\n",argv[0]);
  switch (argc)
    {
	 case 8:
    case 7: s = atoi(argv[6]);
	 case 6: y = atoi(argv[5]); 
    case 5:	x = atoi(argv[4]);
	 case 4: id = atoi(argv[3]);
     	fprintf(stderr,"%s: id = %d, xacross = %d, ydown = %d, samples = %d, history = %s\n",
	      PROGNAME,id,x,y,s,argv[7]);
    	break;
    default:
	(void)fprintf(stderr,"usage: %s infile outfile id x y samples history\n",argv[0]);
	exit(1);
    }
  pic = read_asci_pic(id,x,y,s,argv[7],FILEIN);
  writepic(pic,FILEOUT);
  exit(0);
}
/*********************************************************************/
