/* Libor Spacek (c) 2009 edge-finding  */
#include "lvl.h"
int main(int argc, char *argv[])
{
  PICTURE *pic,*pic2;
  int opdim = 4;
  float lowthresh = 0.05f, highthresh = 0.9f;

  switch (argc) {
	case 6: highthresh = (float)atof(argv[5]); argc--;
	case 5: lowthresh = (float)atof(argv[4]); argc--;
	case 4: opdim = atoi(argv[3]); argc--;
	case 3: case 2: case 1: dofiles(argc,argv); 
		fprintf(stderr,"%s: operator side = %d\n", PROGNAME,opdim); 
		break;
	default: fprintf(stderr,
	"usage: %s [infile] [outfile] [side] [lowthresh] [highthresh]\n",
				PROGNAME);
		exit(1); }
  if ((pic = readpic(FILEIN)) == NULL) 
		{ fprintf(stderr,"%s: failed to read input file\n",PROGNAME); 
			exit(1); }
  if ((pic->fileid) != IMAGE_ID)
		{ fprintf(stderr,"%s: wrong type of input file\n",PROGNAME); 
			exit(1); }
  timex(); pic = grad(pic,opdim); timex();
  // pic = morphgrads(pic); /* optional thinning */
  pic = gradmag(pic);
  pic2  = ftoim(pic,lowthresh,highthresh,1); 
  freepic(pic); /* ftoim does not free pic */
  writepmpic(pic2,FILEOUT);
  exit(0);
}
