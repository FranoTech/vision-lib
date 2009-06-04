/* libor Spacek (C) 1991 reflect.c for reflect */
#include <lvl.h>
#define VIS_MAGIC 0x5649533a   /* VIS: lvl (VISION) magic int in hex */
unsigned char *chbufin; /* byte buffer, but can handle all types */
char answer,reverse,wout;
unsigned short swapping;
int count,county,sz,magic;
PICTURE *pic;
FILE *fp;
/********************************************************************/
int main(int argc,char *argv[])
{
  if (argc != 3)
    {
      fprintf(stderr,"usage: %s filein fileout\n",argv[0]);
      exit(1);
    }
  dofiles(argc,argv);
  if ((fp = fopen(FILEIN,"rb")) == NULL )
      {
        fprintf(stderr,"%s: wrong input file %s\n",PROGNAME,FILEIN);
        exit(1);
      }
  pic = newpic();
  
while (1) 
 {
  printf("k keeps the existing header\n");
  printf("d deletes the header (translates into raw data format)\n");
  printf("p prepends new header (translates from raw data format)\n");
  printf("c changes to new header (d+p)\t>");
  answer = getchar(); getchar();
  printf("%c it is!\n\n",answer); 
  if ((answer == 'd') || (answer == 'k') || (answer == 'c'))
		{ if (readhead(pic,fp) == -1) exit(1); 
		  break; }
  if (answer == 'p') 
	{  
    /* request new header - no header to be read anymore */
	 printf("Fileid int, as defined in vis.h (0 for unsigned char image): ");
    scanf("%d",&(pic->fileid));
	 printf("New image width (x pixels): ");
    scanf("%d",&(pic->x));
    printf("New image height (y pixels): ");
    scanf("%d",&(pic->y));
    printf("Window x origin (float): ");
    scanf("%f",&(pic->xorigin));
    printf("Window y origin (float): ");
    scanf("%f",&(pic->yorigin));
    printf("Number of items (0 for x*y): ");
    scanf("%d",&(pic->items)); 
    if (pic->items == 0) pic->items = (pic->x)*(pic->y);
    printf("Number of samples for each item: ");
    scanf("%d",&(pic->samples));
	 printf("New image history: ");
    scanf("%[^\n]",pic->history);
    getchar();
    pic->magic = VIS_MAGIC;
    break;
	}
  printf("Unexpected response, please try again\n"); 
 }
 
 while (1)
 {
  printf("d for diagonal reflection (rows into columns)\n");
  printf("r rotate 90 deg anti-clockwise (diagonal & horizontal reflections)\n");
  printf("n for no reflection or endian byte swapping: ");
  reverse = getchar(); getchar();
  printf("%c it is!\n\n",reverse);
  if ((reverse=='d') || (reverse=='r') || (reverse=='n')) break;
  printf("Unexpected response - please try again or control-C\n");
 }
 
 while (1)
 {
  printf("v for Libor Spacek's LVL format output\n");
  printf("p for Jef Poskanzer's PGM or PPM output \t>");
  wout = getchar(); getchar();
  printf("%c it is!\n",wout);
  if (wout == 'v') break;
  if (wout == 'p') break;
  (void) printf("Unexpected response - please try again or control-C\n");
 }
  /* no more user input 
     the desired header must be known by now and be in *pic */
  if ((reverse == 'd')||(reverse == 'r'))   /* swap rows and columns */
    { 
      newdata(pic);
      chbufin = (unsigned char *)pic->data;
      swapping = pic->y;
      pic->y = pic->x;
      pic->x = swapping;
		sz = (pic->samples)*sizeofidtype(pic->fileid);
      for (count=0;count<pic->x;count++)
			if (reverse == 'r')
	  			for (county=(pic->y)-1;county>=0;county--)
	    			fread(&(chbufin[sz*(county*(pic->x)+count)]),sz,1,fp);
			else
	  			for (county=0;county<pic->y;county++)
	   			 fread(&(chbufin[sz*(county*pic->x+count)]),sz,1,fp);
    }
  else readdata(pic,fp);
  fclose(fp);

  if (answer != 'd') 
    switch (wout)
     {
    	case 'v': writepic(pic,FILEOUT); break;
		case 'p': writepmpic(pic,FILEOUT); break;
    	default: exit(1);
     }
  else
    {
      if ((fp = fopen(FILEOUT,"wb")) == NULL )
			{
	  		fprintf(stderr,"%s: wrong output file %s\n",PROGNAME,FILEOUT);
	  		exit(1);
			}
      if (fwrite(pic->data,sizeofdata(pic),1,fp) == 0)
			{
	  		fprintf(stderr,"%s: failed to write data\n",PROGNAME);
	  		exit(1);
			}
    }
  exit(0);
}
/*************************************************************************/




