#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>


#include "hint.inc"
#include "skip.inc"


FILE *f;

typedef struct
{
   char id[4];
   int numdir;
   int dirofs;
} wadh_t;

typedef struct
{
   int ofs;
   int wsize,msize;
   unsigned char type;
   unsigned char cmpr;
   unsigned short dummy;
   char name[16];
} wade_t __attribute__ ((packed));

wadh_t wh;
wade_t *we;
int nwe;

FILE *f;

int main(int argc, char **argv)
{
   int i;

   printf("%s - version 2 (" __DATE__ ")\n"
          "Created by Alexander Malmberg <alexander@malmberg.org>\n",
          argv[0]);

   if (argc!=2)
   {
      printf("Syntax: %s wadfile.wad\n"
             "  Adds HINT and SKIP textures to the specified .wad file.\n",
             argv[0]);
      return 0;
   }

   f=fopen(argv[1],"rb+");
   if (!f) printf("Can't open '%s'!\n",argv[1]),exit(1);

   fread(&wh,1,sizeof(wadh_t),f);
   fseek(f,wh.dirofs,SEEK_SET);
   we=malloc(sizeof(wade_t)*(2+(nwe=wh.numdir)));
   if (!we) printf("Out of memory!\n"),exit(1);
   fread(we,1,sizeof(wade_t)*nwe,f);

   fseek(f,0,SEEK_END);
   i=ftell(f);
   if (i!=wh.dirofs+wh.numdir*sizeof(wade_t))
      printf("%i!=%i=%i+%i*%i: Odd .wad file, directory not at end!\n",
      i,wh.dirofs+wh.numdir*(int)sizeof(wade_t),wh.dirofs,wh.numdir,
      (int)sizeof(wade_t))
      ,exit(1);

   printf("- %s - %i entries\n",argv[1],nwe);
   for (i=0;i<nwe;i++)
   {
      if (!(strcasecmp(we[i].name,"hint") && strcasecmp(we[i].name,"skip")))
      {
         printf("'%s' already in .wad file!\n",we[i].name);
         exit(1);
      }
   }

/*   for (i=0;i<nwe;i++)
   {
      FILE *o;
      int j;

      o=fopen(we[i].name,"wt");
      fprintf(o,"unsigned char %s[]=\n{",we[i].name);
      fseek(f,we[i].ofs,SEEK_SET);
      for (j=0;j<we[i].wsize;j++)
      {
         if (!(j&15)) fprintf(o,"\n ");
         fprintf(o,"0x%02x,",fgetc(f));
      }
      fprintf(o,"\n}\n");
      fclose(o);
   }*/

   memset(&we[nwe],0,sizeof(wade_t)*2);

   strcpy(we[nwe].name,"HINT");
   we[nwe].type=0x44;
   we[nwe].ofs=wh.dirofs;
   we[nwe].msize=we[nwe].wsize=sizeof(HINT);
   nwe++;
   strcpy(we[nwe].name,"SKIP");
   we[nwe].type=0x44;
   we[nwe].ofs=wh.dirofs+sizeof(HINT);
   we[nwe].msize=we[nwe].wsize=sizeof(SKIP);
   nwe++;

   fseek(f,wh.dirofs,SEEK_SET);
   fwrite(HINT,1,sizeof(HINT),f);
   fwrite(SKIP,1,sizeof(SKIP),f);
   fwrite(we,1,sizeof(wade_t)*nwe,f);

   wh.numdir+=2;
   wh.dirofs+=sizeof(SKIP)+sizeof(HINT);
   fseek(f,0,SEEK_SET);
   fwrite(&wh,1,sizeof(wadh_t),f);

   fclose(f);
   
   printf("Done!\n");

   return 0;
}

