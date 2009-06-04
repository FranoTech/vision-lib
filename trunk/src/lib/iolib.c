/* IOLIB.C part of LVL Libor's Vision Library, (C) Libor Spacek 2009 */
/*************************************************************************/
#include "lvl.h"
#include <string.h>
/********************************************************************
The following globals are set by function dofiles(); for convenience.
There are no other global variables
********************************************************************/
char *PROGNAME = (char *)"unknown", *FILEIN = NULL, *FILEOUT = NULL;
/********************************************************************
Magic numbers for direct recognition of file formats
and machine architectures, without relying on file extensions.
They consist of two bytes for (byte based) portable pixel maps. 
Four bytes are used in .lvl format so that they align 
with int boundary for safer binary fileread of longer types.
(Computer word alignment can be architecture dependent)
**********************************************************************/
#define PGM_ASCI_MAGIC 2
#define PPM_ASCI_MAGIC 3
#define PGM_MAGIC 5
#define PPM_MAGIC 6
#define PAM_MAGIC 7
#define VIS_MAGIC 0x5649533a   /* VIS: lvl (VISION) magic int in hex */
#define VIS_SWMAGIC 0x3a534956 /* :SIV swapped endian on INTEL */
#define VISA_MAGIC 0x56495341   /* VISA indicates ascii file */
#define VISA_SWMAGIC 0x41534956 /* ASIV detects swapped header */
/* #define SUN_MAGIC 0x59a66a95    Y.j. = SUN RASTER MAGIC number */
/* #define SUN_SWMAGIC 0x956aa659  swapped on INTEL */
/********************************************************************/
int sizeofidtype(int id)
{ switch(id) {
	case IMAGE_ID: return(sizeof(IMAGE_TYPE));
	case POSINT_ID: return(sizeof(POSINT_TYPE));
	case INT_ID: return(sizeof(INT_TYPE));
	case FIMAGE_ID: return(sizeof(FIMAGE_TYPE));
	case FVEC_ID: return(sizeof(FVEC_TYPE));
	case IVEC_ID: return(sizeof(IVEC_TYPE));
	case BND_ID: return(sizeof(BND_TYPE));
	default: (void)fprintf(stderr,"sizeofidtype: bad id %d\n",id); return(0); }
}
/*******************************************************************/
int sizeofitem(PICTURE *p)
{ return((p->samples)*sizeofidtype(p->fileid)); }
/*********************************************************************/
int sizeofdata(PICTURE *p)
{ return((p->items)*(p->samples)*sizeofidtype(p->fileid)); }
/*********************************************************************/
void vmessage()
{ static int firstime = 1;
if (firstime) { firstime = 0; (void)fprintf(stderr,MESSAGE); }
}
/**********************************************************************/
void printtype(int fid)
{
switch (fid) {
	case(IMAGE_ID): (void)fprintf(stderr,"unsigned char\n"); break;
	case(POSINT_ID):(void)fprintf(stderr,"unsigned int\n"); break;
	case(INT_ID):   (void)fprintf(stderr,"int\n"); break;
	case(FIMAGE_ID):(void)fprintf(stderr,"float\n"); break;
	case(FVEC_ID): (void)fprintf(stderr,"struct{float fx; float fy;}\n"); break;
	case(IVEC_ID):(void)fprintf(stderr,"struct{int fx; int fy;}\n"); break;
	case(BND_ID): (void)fprintf(stderr,"struct{int fx; int fy;}\n"); break;
	default:(void)fprintf(stderr,
			"printtype error: unrecognised fileid %d\n",fid); }
}
/**********************************************************************/
int printhead(PICTURE *pic, int magic)
{
switch (magic) {
	case PGM_ASCI_MAGIC: (void)fprintf(stderr,"PGM ascii format"); break;
	case PPM_ASCI_MAGIC: (void)fprintf(stderr,"PPM ascii format"); break;
	case PGM_MAGIC: (void)fprintf(stderr,"PGM rawbits format"); break;
	case PPM_MAGIC: (void)fprintf(stderr,"PPM rawbits format"); break;
	case PAM_MAGIC: (void)fprintf(stderr,"PAM portable any map format"); break;
	case VIS_MAGIC: (void)fprintf(stderr,"LVL binary format"); break;
	case VIS_SWMAGIC:(void)fprintf(stderr,"LVL binary format (bytes swapped)");break;
	case VISA_MAGIC: (void)fprintf(stderr,"LVL ascii format"); break;
	case VISA_SWMAGIC:(void)fprintf(stderr,"LVL ascii format (header bytes swapped)");break;
	default: (void)fprintf(stderr,
			"Unsupported magic %x given to printhead\n",magic); return(-1); }
(void)fprintf(stderr,", %d bytes of data\n", sizeofdata(pic));
(void)fprintf(stderr,
		"%d across %d down, origin (%.2f,%.2f)\n%d items, each has ", 
		pic->x,pic->y,pic->xorigin,pic->yorigin,pic->items);
if (pic->samples == 1) (void)fprintf(stderr,"one sample ");
else (void)fprintf(stderr,"%d samples ",pic->samples); 
(void)fprintf(stderr,"of type: ");
printtype(pic->fileid);
(void)fprintf(stderr,"History: %s\n",pic->history);
return(0);
}
/**************************************************************************/
int dofiles ( int a, char *v[] )
	/* sets global PROGNAME, FILEIN, FILEOUT - must be called first */
{
FILEIN = NULL;
FILEOUT = NULL;
vmessage();
if (a > 3) a = 3;
switch (a) { 
	case 3: FILEOUT = v[2]; 
	case 2: FILEIN = v[1];
	case 1: PROGNAME = v[0]; }
return(0);
}
/*************************************************************************/
float swapfloat(float f)
{
  union  { float f; unsigned char b[4]; } indat, outdat;

  indat.f = f;
  outdat.b[0] = indat.b[3];
  outdat.b[1] = indat.b[2];
  outdat.b[2] = indat.b[1];
  outdat.b[3] = indat.b[0];
  return outdat.f;
}
/*************************************************************************/
int swapint(int i)
{
  union  { int i; unsigned char b[4]; } indat, outdat;

  indat.i = i;
  outdat.b[0] = indat.b[3];
  outdat.b[1] = indat.b[2];
  outdat.b[2] = indat.b[1];
  outdat.b[3] = indat.b[0];
  return outdat.i;
}
/*************************************************************************/
void swaphead(PICTURE *pic)
{	pic->fileid = swapint(pic->fileid);
	pic->x = swapint(pic->x);
	pic->y = swapint(pic->y);
	pic->xorigin = swapfloat(pic->xorigin);
	pic->yorigin = swapfloat(pic->yorigin);
	pic->items = swapint(pic->items);
	pic->samples = swapint(pic->samples);
}
/**************************************************************************/
int readhead(PICTURE *pic, FILE *fp)
/* architecture independent binary reader. Reads (and prints) header 
	into the *PICTURE structure with *data untouched */
{
unsigned char firstmagic, secondmagic;
char pline[132];
union { unsigned char charform[4]; int intform; } magic;
int x,y,gmax;
vmessage();
firstmagic = fgetc(fp);
if ( firstmagic == 'P' ) 
	{
	secondmagic = fgetc(fp);
	fgetc(fp);  /* skips linefeed */
	fgets(pline, 132, fp);
	if ( (pline[0] < 48) || (pline[0] > 57) )
		{
		(void)fprintf(stderr,"Pixelmap Comment: %s",pline);
		fscanf(fp,"%d %d",&x,&y);
		}
	else sscanf(pline,"%d %d",&x,&y);
	fscanf(fp,"%d",&gmax);
	if (gmax != GMAX)
		{
		(void)fprintf(stderr,"Unexpected Pixelmap Greylevels: %d\n",gmax);
		return(-1);
		}
	fgetc(fp);  /* skips linefeed */
	switch (secondmagic) {
		case '2': /* call read asci ? */
			magic.intform = PGM_ASCI_MAGIC;
			fillhead(pic,IMAGE_ID,x,y,0.f,0.f,x*y,1,"PGM ascii");
			break;
		case '3': /* call read asci ? */
			magic.intform = PPM_ASCI_MAGIC;
			fillhead(pic,IMAGE_ID,x,y,0.f,0.f,x*y,3,"PPM ascii");
			break;
		case '5': /* .pgm file */
			magic.intform = PGM_MAGIC;
			fillhead(pic,IMAGE_ID,x,y,0.f,0.f,x*y,1,"PGM");
			break;
		case '6': /* .ppm file */
			magic.intform = PPM_MAGIC;
			fillhead(pic,IMAGE_ID,x,y,0.f,0.f,x*y,3,"PPM");
			break;
		case '7': /* .pam file */
			magic.intform = PAM_MAGIC;
			fillhead(pic,IMAGE_ID,x,y,0.f,0.f,x*y,3,"PAM");
			break;
		default: (void)fprintf(stderr,
			"Unexpected magic P%c: supported pixel maps are: P2,P3,P5,P6\n",secondmagic);
			return(-1); }
	}
else /* .lvl format */
 	{
	magic.charform[0] = firstmagic;
	magic.charform[1] = fgetc(fp);
	magic.charform[2] = fgetc(fp);
	magic.charform[3] = fgetc(fp);
	fread(pic,HEADSZ,1,fp);
	switch (magic.intform) {
		case VIS_MAGIC: case VISA_MAGIC: break;
		case VIS_SWMAGIC: case VISA_SWMAGIC: swaphead(pic); break;
		default:
			(void)fprintf(stderr,"readhead: unrecognised magic int\n");
			return(-1); }
	}
strcat(pic->history,"|");
strcat(pic->history,PROGNAME);
pic->magic = magic.intform;
printhead(pic, pic->magic);
return(0); /* success */
}
/**************************************************************************/
/* creates an empty header */
PICTURE *newpic(void)
{
PICTURE *pic;
vmessage();
pic = (PICTURE *)malloc(sizeof(PICTURE));
if (pic == NULL)
	{
		(void)fprintf(stderr,"newpic: failed to allocate memory\n");
		return(NULL);
	}
pic->data = pic->udata = pic->adata = NULL;
pic->magic = 0;
return(pic);
}
/**************************************************************************/
/* fills in header details of an existing pic, used by readhead */
int fillhead(PICTURE *pic, int fileid, int x, int y, 
		float xorig, float yorig, int items, int sampls, char *history)
{
pic->fileid = fileid;
pic->x = x;
pic->y = y;
pic->xorigin = xorig;
pic->yorigin = yorig;
pic->items = items;
pic->samples = sampls;
strcpy(pic->history,history);
return(0);
}
/**************************************************************************/
/* only header contents copied, data memory not allocated; used by copypic */
PICTURE *copyhead(PICTURE *pic, int samples, int fileid)
{
PICTURE *pic2 = newpic();
if (pic == NULL)
	{ (void)fprintf(stderr,"copyhead given null pic\n"); return(NULL); }
fillhead(pic2,fileid,pic->x,pic->y,pic->xorigin,pic->yorigin,pic->items,samples,pic->history);
return(pic2);
}
/**************************************************************************/
/* data not copied, samples and fileid can be changed */
PICTURE *copypic(PICTURE *pic, int samples, int fileid)
{
PICTURE *pic2 = copyhead(pic,samples,fileid);
if (pic == NULL)
	{ (void)fprintf(stderr,"copypic given null pic\n"); return(NULL); }
if (newdata(pic2)) 
{ (void)fprintf(stderr,"copypic failed to allocate memory\n"); return(NULL); }
return(pic2);
}
/**************************************************************************/
PICTURE *makepic(int fileid, int x, int y, 
		float xorig, float yorig, int items, int sampls, char *history)
{
PICTURE *pic = newpic();
fillhead(pic,fileid,x,y,xorig,yorig,items,sampls,history);
if (newdata(pic)) return(NULL);
return(pic);
}
/**************************************************************************/
PICTURE *newvxpic(char *history)
{ return(makepic(IMAGE_ID,412,286,0,0,412*286,1,history)); }
/**************************************************************************/
int newdata(PICTURE *pic)
{
pic->data = (void *)malloc(sizeofdata(pic));
if ((pic->data) == NULL)
	{
	(void)fprintf(stderr,"newdata: failed to allocate memory for the data\n");
	return(-1);
	}
return(0);
}
/**************************************************************************/
 /* change data to same dimensions, new samples and type */
int changepic(PICTURE *pic, int samples, int fileid)
{
int oldsize = sizeofdata(pic);
pic->fileid = fileid;
pic->samples = samples;
if ((pic->data) == NULL)
	{ if (newdata(pic)) return(-1); }
else if (sizeofdata(pic) != oldsize)
	{
		free(pic->data);
		if (newdata(pic)) return(-1);
		/*    pic->data = (void *)realloc(pic->data,pic->size); */
	}
return(0);
}
/**************************************************************************/
void freepic(PICTURE *pic)
{
free(pic->data);
free(pic);
}
/**************************************************************************/
/* instantiates *(pic->data), swaps as needed, readhead called before */
int readdata(PICTURE *pic, FILE *fp)
{ 
int magic = pic->magic;
if ((pic->data) == NULL)
	{ if (newdata(pic)) return(-1); }
switch (magic) {
	case PGM_ASCI_MAGIC: case PPM_ASCI_MAGIC: case VISA_MAGIC: case VISA_SWMAGIC:
		readascidata(pic,fp); break;
	case VIS_MAGIC: case PGM_MAGIC: case PPM_MAGIC:
		if ( fread(pic->data,sizeofdata(pic),1,fp) == 0)
		{
		(void)fprintf(stderr,"readdata: failed to read binary data\n");
		return(-1);
		} break; 
	case VIS_SWMAGIC: /* now does endian swapping */
		{ 
		int x, sz = (pic->items)*(pic->samples);
		switch (pic->fileid) {
			case IMAGE_ID: break; /* no need to swap individual chars */
			case INT_ID: { 
				INT_TYPE *buf = (INT_TYPE *)(pic->data);
				for (x=0; x<sz; x++) { *buf = swapint(*buf); buf++; } }
				break;
			case FIMAGE_ID: { 
				FIMAGE_TYPE *buf = (FIMAGE_TYPE *)(pic->data);
				for (x=0; x<sz; x++) { *buf = swapfloat(*buf); buf++; } }
				break;
			default: (void)fprintf(stderr,
				"readdata: please recreate the input file on this architecture\n"); 
				return(-1);	}
		} break;
	default: (void)fprintf(stderr,"readdata: strange magic\n"); return(-1); }
return(0);
}
/**************************************************************************/
int readascidata(PICTURE *pic, FILE *fp)    /* instantiates *(pic->data)  */
{ 
int x, innum, status, sz=(pic->items)*(pic->samples);
switch (pic->fileid) {
	case IMAGE_ID: { 
		IMAGE_TYPE *buf = (IMAGE_TYPE *)(pic->data);
		for (x=0; x<sz; x++) 
			{ 
			status = fscanf(fp,"%d",&innum);
			if (status != 1)
	   		{
	   		if (status == EOF) (void)fprintf(stderr,
				"%s: readascidata: unexpected end of input file\n",PROGNAME);
	   		else (void)fprintf(stderr,
				"%s: readascidata: failure reading from file\n",PROGNAME);
	   		return(-1);
	   		}
			*(buf++) = (unsigned char)innum; 
			} }
		break;
	case FIMAGE_ID: { 
		FIMAGE_TYPE *buf = (FIMAGE_TYPE *)(pic->data);
		for (x=0; x<sz; x++) 
			{ 
			status = fscanf(fp,"%e",buf++);
			if (status != 1)
	   		{
	   		if (status == EOF) (void)fprintf(stderr,
				"%s: readascidata: unexpected end of input file\n",PROGNAME);
	   		else (void)fprintf(stderr,
				"%s: readascidata: failure reading from file\n",PROGNAME);
	   		return(-1);
	   		}
			} }
		break;
	case INT_ID: { 
		INT_TYPE *buf = (INT_TYPE *)(pic->data);
		for (x=0; x<sz; x++) 
			{ 
			status = fscanf(fp,"%d",buf++);
			if (status != 1)
	   		{
	   		if (status == EOF) (void)fprintf(stderr,
				"%s: readascidata: unexpected end of input file\n",PROGNAME);
	   		else (void)fprintf(stderr,
				"%s: readascidata: failure reading from file\n",PROGNAME);
	   		return(-1);
	   		}
			 } }
		break;
	default: 
		(void)fprintf(stderr,
		"readdata: please recreate the input file on this architecture\n");
		return(-1);	}
return(0);
}
/**************************************************************************/
PICTURE *readpic(char *filename)
{
PICTURE *pic = newpic();
FILE *fp;

if (filename == NULL) { filename = (char *)"stdin"; fp = stdin; }
else
	if ((fp = fopen(filename,"rb")) == NULL )
		{
		(void)fprintf(stderr,"%s: readpic: failed to open input file %s\n",
		PROGNAME,filename);
		return(NULL);
		}
(void)fprintf(stderr,"Reading %s from disk: ",filename);
if ((readhead(pic,fp)) == -1) return(NULL);
if (readdata(pic,fp)) return(NULL);
fclose(fp);
return(pic); 
}
/*********************************************************************/
PICTURE *read_asci_pic(int id,int x,int y,int samples,char *hist,char *filename)
{
PICTURE *pic;
FILE *fpin;

if (filename == NULL) { filename = (char *)"stdin"; fpin = stdin; }
else
	if ((fpin = fopen(filename,"r")) == NULL )
		{
		(void)fprintf(stderr,
		"%s: read_asci_pic: failed to open input file %s\n",PROGNAME,filename);
		return(NULL);
		}
(void)fprintf(stderr,"Reading headerless ascii %s from disk\n",filename);
pic = makepic(id,x,y,0.0,0.0,x*y,samples,hist);
readascidata(pic,fpin);
fclose(fpin);
return(pic);
}
/*********************************************************************
writes the specified window in ascii format to filepointer fp.
No out of bounds checking is done: do it yourself beforehand! */
int write_asci(PICTURE *pic,int xo,int yo,int width,int height,FILE *fp)
{
int count,x,y,samp=pic->samples;
 switch (pic->fileid) { 
	case IMAGE_ID:
		{ IMAGE_TYPE *buff = (IMAGE_TYPE *)pic->data;
		for (y = 0;  y < height;  y++)
		{	count = samp*((yo+y)*(pic->x)+xo);
			for (x = 0;  x < samp*width;  x++)
				(void)fprintf(fp," %d",buff[count++]);
			(void)fprintf(fp,"\n");
		} } break;
	case INT_ID:
		{ INT_TYPE *buff = (INT_TYPE *)pic->data;
		for (y = 0;  y < height;  y++)
		{	count = samp*((yo+y)*(pic->x)+xo);
			for (x = 0;  x < samp*width;  x++)
				(void)fprintf(fp," %d",buff[count++]);
			(void)fprintf(fp,"\n");
		} } break;
	case FIMAGE_ID:
		{ FIMAGE_TYPE *buff = (FIMAGE_TYPE *)pic->data;
		for (y = 0;  y < height;  y++)
		{	count = samp*((yo+y)*(pic->x)+xo);
			for (x = 0;  x < samp*width;  x++)
				(void)fprintf(fp," %e",buff[count++]);
			(void)fprintf(fp,"\n");
		} } break;
	default:
		(void)fprintf(stderr,"write_asci_pic does not yet support this imagetype\n");
		return(-1);
	}
return(0);
}
/*********************************************************************/
/* output functions - easiest to use is writeanypic using magic supplied by readpic */
/************************************************************************/
int writepic(PICTURE *pic, char *filename) /* writes LVL binary format */
{ writeanypic(pic, filename, VIS_MAGIC); return(0); }
/************************************************************************/
int writeascipic(PICTURE *pic, char *filename) /* writes LVL ascii format */
{ writeanypic(pic, filename, VISA_MAGIC); return(0); }
/************************************************************************/
int writepmpic(PICTURE *pic, char *filename) /* writes portable pixel map format */
{  int magic;
	if (pic->fileid == IMAGE_ID) 
	{ switch (pic->samples) {
			case 1: magic = PGM_MAGIC; break;
			case 3: magic = PPM_MAGIC; break;
			default: (void)fprintf(stderr,
				"writepmpic: cannot write more than three samples!\n"); return(-1); }
	} 
	else { (void)fprintf(stderr,
	  "writepmpic: cannot write this type of file, try writepic!\n"); return(-1); }
	writeanypic(pic, filename, magic); /* success*/
	return(0);
}
/************************************************************************/
int writeascipmpic(PICTURE *pic, char *filename) /* writes portable pixel map format */
{  int magic;
	if (pic->fileid == IMAGE_ID) 
	{ switch (pic->samples) {
			case 1: magic = PGM_ASCI_MAGIC; break;
			case 3: magic = PPM_ASCI_MAGIC; break;
			default: (void)fprintf(stderr,
				"writeascipmpic: cannot write more than three samples!\n"); return(-1); }
	} 
	else { (void)fprintf(stderr,
	  "writeascipmpic: cannot write this type of file, try writepic!\n"); return(-1); }
	writeanypic(pic, filename, magic); /* success*/
	return(0);
}
/************************************************************************/
int writeanypic(PICTURE *pic, char *filename, int magic)
{
FILE *fp;
switch (magic) { /* data is written always unswapped (same endian as processor) */
	case VIS_SWMAGIC: magic = VIS_MAGIC; break;
	case VISA_SWMAGIC: magic = VISA_MAGIC; break; }
if (filename == NULL) { filename = (char *)"stdout"; fp = stdout; }
else 	if ((fp = fopen(filename,"wb")) == NULL )
		{ (void)fprintf(stderr,
			"%s: writeanypic: wrong output file %s\n",PROGNAME,filename);
			return(-1); }
(void)fprintf(stderr,"\nWriting %s to disk: ",filename);
printhead(pic, magic);
if ((magic > 1) && (magic < 7)) /* is a portable pixel map file */
	{ (void)fprintf(fp,"P%c\n#Created by LVL\n",magic+48); /* ascii 2-6 */
	  (void)fprintf(fp,"%d %d\n%d\n", pic->x, pic->y, GMAX); }
else {
	if (fwrite(&magic,sizeof(int),1,fp) == 0)
	{	(void)fprintf(stderr,"writeanypic: failed to write magic\n");
		return(-1); }
	if (fwrite(pic,HEADSZ,1,fp) == 0)
	{	(void)fprintf(stderr,"writeanypic: failed to write header\n");
		return(-1); } } /* now wrote all headers */
if ((magic == VISA_MAGIC) || (magic == PGM_ASCI_MAGIC) || (magic == PPM_ASCI_MAGIC))
	write_asci(pic,0,0,pic->x,pic->y,fp); 
else
	if (fwrite(pic->data,sizeofdata(pic),1,fp) == 0)
		{ (void)fprintf(stderr,"writeanypic: failed to write data\n");
		  return(-1); } 
fclose(fp);
return(0);
}
/*********************************************************************/
