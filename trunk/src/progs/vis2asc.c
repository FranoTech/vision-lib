/* Libor Spacek (C) 2009 vis2asc.c 
   conversion from .lvl binary to .lvl ascii */
#include "lvl.h"
/********************************************************************/
int main(int argc, char *argv[])
{
  PICTURE *pic;
  dofiles(argc,argv);
  if ((pic = readpic(FILEIN)) == NULL) exit(1);
  writeascipic(pic,FILEOUT);
  exit(0);
}
/*********************************************************************/

