/* lbmlib.c*/

#include "cmdlib.h"
#include "lbmlib.h"

#include "qmalloc.h"

bmhd_t  bmhd;

#ifdef 0
/*
============================================================================

						LBM STUFF

============================================================================
*/


#define FORMID ('F'+('O'<<8)+((int)'R'<<16)+((int)'M'<<24))
#define ILBMID ('I'+('L'<<8)+((int)'B'<<16)+((int)'M'<<24))
#define PBMID  ('P'+('B'<<8)+((int)'M'<<16)+((int)' '<<24))
#define BMHDID ('B'+('M'<<8)+((int)'H'<<16)+((int)'D'<<24))
#define BODYID ('B'+('O'<<8)+((int)'D'<<16)+((int)'Y'<<24))
#define CMAPID ('C'+('M'<<8)+((int)'A'<<16)+((int)'P'<<24))


int    Align (int l)
{
	if (l&1)
		return l+1;
	return l;
}



/*
================
=
= LBMRLEdecompress
=
= Source must be evenly aligned!
=
================
*/

byte  *LBMRLEDecompress (byte *source,byte *unpacked, int bpwidth)
{
	int     count;
	byte    b,rept;

	count = 0;

	do
	{
		rept = *source++;

		if (rept > 0x80)
		{
			rept = (rept^0xff)+2;
			b = *source++;
			memset(unpacked,b,rept);
			unpacked += rept;
		}
		else if (rept < 0x80)
		{
			rept++;
			memcpy(unpacked,source,rept);
			unpacked += rept;
			source += rept;
		}
		else
			rept = 0;               /* rept of 0x80 is NOP*/

		count += rept;

	} while (count<bpwidth);

	if (count>bpwidth)
		Error ("Decompression exceeded width!\n");


	return source;
}


#define BPLANESIZE      128
byte    bitplanes[9][BPLANESIZE];       /* max size 1024 by 9 bit planes*/


/*
=================
=
= MungeBitPlanes8
=
= This destroys the bit plane data!
=
=================
*/

void MungeBitPlanes8 (int width, byte *dest)
{
	*dest=width;	/* shut up the compiler warning*/
	Error ("MungeBitPlanes8 not rewritten!");
#if 0
asm     les     di,[dest]
asm     mov     si,-1
asm     mov     cx,[width]
mungebyte:
asm     inc     si
asm     mov     dx,8
mungebit:
asm     shl     [BYTE PTR bitplanes + BPLANESIZE*7 +si],1
asm     rcl     al,1
asm     shl     [BYTE PTR bitplanes + BPLANESIZE*6 +si],1
asm     rcl     al,1
asm     shl     [BYTE PTR bitplanes + BPLANESIZE*5 +si],1
asm     rcl     al,1
asm     shl     [BYTE PTR bitplanes + BPLANESIZE*4 +si],1
asm     rcl     al,1
asm     shl     [BYTE PTR bitplanes + BPLANESIZE*3 +si],1
asm     rcl     al,1
asm     shl     [BYTE PTR bitplanes + BPLANESIZE*2 +si],1
asm     rcl     al,1
asm     shl     [BYTE PTR bitplanes + BPLANESIZE*1 +si],1
asm     rcl     al,1
asm     shl     [BYTE PTR bitplanes + BPLANESIZE*0 +si],1
asm     rcl     al,1
asm     stosb
asm     dec     cx
asm     jz      done
asm     dec     dx
asm     jnz     mungebit
asm     jmp     mungebyte

done:
#endif
}


void MungeBitPlanes4 (int width, byte *dest)
{
	*dest=width;	/* shut up the compiler warning*/
	Error ("MungeBitPlanes4 not rewritten!");
#if 0

asm     les     di,[dest]
asm     mov     si,-1
asm     mov     cx,[width]
mungebyte:
asm     inc     si
asm     mov     dx,8
mungebit:
asm     xor     al,al
asm     shl     [BYTE PTR bitplanes + BPLANESIZE*3 +si],1
asm     rcl     al,1
asm     shl     [BYTE PTR bitplanes + BPLANESIZE*2 +si],1
asm     rcl     al,1
asm     shl     [BYTE PTR bitplanes + BPLANESIZE*1 +si],1
asm     rcl     al,1
asm     shl     [BYTE PTR bitplanes + BPLANESIZE*0 +si],1
asm     rcl     al,1
asm     stosb
asm     dec     cx
asm     jz      done
asm     dec     dx
asm     jnz     mungebit
asm     jmp     mungebyte

done:
#endif
}


void MungeBitPlanes2 (int width, byte *dest)
{
	*dest=width;	/* shut up the compiler warning*/
	Error ("MungeBitPlanes2 not rewritten!");
#if 0
asm     les     di,[dest]
asm     mov     si,-1
asm     mov     cx,[width]
mungebyte:
asm     inc     si
asm     mov     dx,8
mungebit:
asm     xor     al,al
asm     shl     [BYTE PTR bitplanes + BPLANESIZE*1 +si],1
asm     rcl     al,1
asm     shl     [BYTE PTR bitplanes + BPLANESIZE*0 +si],1
asm     rcl     al,1
asm     stosb
asm     dec     cx
asm     jz      done
asm     dec     dx
asm     jnz     mungebit
asm     jmp     mungebyte

done:
#endif
}


void MungeBitPlanes1 (int width, byte *dest)
{
	*dest=width;	/* shut up the compiler warning*/
	Error ("MungeBitPlanes1 not rewritten!");
#if 0
asm     les     di,[dest]
asm     mov     si,-1
asm     mov     cx,[width]
mungebyte:
asm     inc     si
asm     mov     dx,8
mungebit:
asm     xor     al,al
asm     shl     [BYTE PTR bitplanes + BPLANESIZE*0 +si],1
asm     rcl     al,1
asm     stosb
asm     dec     cx
asm     jz      done
asm     dec     dx
asm     jnz     mungebit
asm     jmp     mungebyte

done:
#endif
}


/*
=================
=
= LoadLBM
=
=================
*/

void LoadLBM (char *filename, byte **picture, byte **palette)
{
	byte    *LBMbuffer, *picbuffer, *cmapbuffer;
	int             y,p,planes;
	byte    *LBM_P, *LBMEND_P;
	byte    *pic_p;
	byte    *body_p;
	unsigned        rowsize;

	int    formtype,formlength;
	int    chunktype,chunklength;
	void    (*mungecall) (int, byte *);

/* qiet compiler warnings*/
	picbuffer = NULL;
	cmapbuffer = NULL;
	mungecall = NULL;

/**/
/* load the LBM*/
/**/
	LoadFile (filename, (void **)&LBMbuffer);

/**/
/* parse the LBM header*/
/**/
	LBM_P = LBMbuffer;
	if ( *(int *)LBMbuffer != LittleLong(FORMID) )
	   Error ("No FORM ID at start of file!\n");

	LBM_P += 4;
	formlength = BigLong( *(int *)LBM_P );
	LBM_P += 4;
	LBMEND_P = LBM_P + Align(formlength);

	formtype = LittleLong(*(int *)LBM_P);

	if (formtype != ILBMID && formtype != PBMID)
		Error ("Unrecognized form type: %c%c%c%c\n", formtype&0xff
		,(formtype>>8)&0xff,(formtype>>16)&0xff,(formtype>>24)&0xff);

	LBM_P += 4;

/**/
/* parse chunks*/
/**/

	while (LBM_P < LBMEND_P)
	{
		chunktype = LBM_P[0] + (LBM_P[1]<<8) + (LBM_P[2]<<16) + (LBM_P[3]<<24);
		LBM_P += 4;
		chunklength = LBM_P[3] + (LBM_P[2]<<8) + (LBM_P[1]<<16) + (LBM_P[0]<<24);
		LBM_P += 4;

		switch ( chunktype )
		{
		case BMHDID:
			memcpy (&bmhd,LBM_P,sizeof(bmhd));
			bmhd.w = BigShort(bmhd.w);
			bmhd.h = BigShort(bmhd.h);
			bmhd.x = BigShort(bmhd.x);
			bmhd.y = BigShort(bmhd.y);
			bmhd.pageWidth = BigShort(bmhd.pageWidth);
			bmhd.pageHeight = BigShort(bmhd.pageHeight);
			break;

		case CMAPID:
			cmapbuffer = Q_malloc (768);
			memset (cmapbuffer, 0, 768);
			memcpy (cmapbuffer, LBM_P, chunklength);
			break;

		case BODYID:
			body_p = LBM_P;

			pic_p = picbuffer = Q_malloc (bmhd.w*bmhd.h);
			if (formtype == PBMID)
			{
			/**/
			/* unpack PBM*/
			/**/
				for (y=0 ; y<bmhd.h ; y++, pic_p += bmhd.w)
				{
					if (bmhd.compression == cm_rle1)
						body_p = LBMRLEDecompress ((byte *)body_p
						, pic_p , bmhd.w);
					else if (bmhd.compression == cm_none)
					{
						memcpy (pic_p,body_p,bmhd.w);
						body_p += Align(bmhd.w);
					}
				}

			}
			else
			{
			/**/
			/* unpack ILBM*/
			/**/
				planes = bmhd.nPlanes;
				if (bmhd.masking == ms_mask)
					planes++;
				rowsize = (bmhd.w+15)/16 * 2;
				switch (bmhd.nPlanes)
				{
				case 1:
					mungecall = MungeBitPlanes1;
					break;
				case 2:
					mungecall = MungeBitPlanes2;
					break;
				case 4:
					mungecall = MungeBitPlanes4;
					break;
				case 8:
					mungecall = MungeBitPlanes8;
					break;
				default:
					Error ("Can't munge %i bit planes!\n",bmhd.nPlanes);
				}

				for (y=0 ; y<bmhd.h ; y++, pic_p += bmhd.w)
				{
					for (p=0 ; p<planes ; p++)
						if (bmhd.compression == cm_rle1)
							body_p = LBMRLEDecompress ((byte *)body_p
							, bitplanes[p] , rowsize);
						else if (bmhd.compression == cm_none)
						{
							memcpy (bitplanes[p],body_p,rowsize);
							body_p += rowsize;
						}

					mungecall (bmhd.w , pic_p);
				}
			}
			break;
		}

		LBM_P += Align(chunklength);
	}

	Q_free (LBMbuffer);

	*picture = picbuffer;
	*palette = cmapbuffer;
}


/*
============================================================================

							WRITE LBM

============================================================================
*/

/*
==============
=
= WriteLBMfile
=
==============
*/

void WriteLBMfile (char *filename, byte *data, int width, int height, byte *palette)
{
	byte    *lbm, *lbmptr;
	int    *formlength, *bmhdlength, *cmaplength, *bodylength;
	int    length;
	bmhd_t  basebmhd;

	lbm = lbmptr = Q_malloc (width*height+1000);

/**/
/* start FORM*/
/**/
	*lbmptr++ = 'F';
	*lbmptr++ = 'O';
	*lbmptr++ = 'R';
	*lbmptr++ = 'M';

	formlength = (int*)lbmptr;
	lbmptr+=4;                      /* leave space for length*/

	*lbmptr++ = 'P';
	*lbmptr++ = 'B';
	*lbmptr++ = 'M';
	*lbmptr++ = ' ';

/**/
/* write BMHD*/
/**/
	*lbmptr++ = 'B';
	*lbmptr++ = 'M';
	*lbmptr++ = 'H';
	*lbmptr++ = 'D';

	bmhdlength = (int *)lbmptr;
	lbmptr+=4;                      /* leave space for length*/

	memset (&basebmhd,0,sizeof(basebmhd));
	basebmhd.w = BigShort((short)width);
	basebmhd.h = BigShort((short)height);
	basebmhd.nPlanes = BigShort(8);
	basebmhd.xAspect = BigShort(5);
	basebmhd.yAspect = BigShort(6);
	basebmhd.pageWidth = BigShort((short)width);
	basebmhd.pageHeight = BigShort((short)height);

	memcpy (lbmptr,&basebmhd,sizeof(basebmhd));
	lbmptr += sizeof(basebmhd);

	length = lbmptr-(byte *)bmhdlength-4;
	*bmhdlength = BigLong(length);
	if (length&1)
		*lbmptr++ = 0;          /* pad chunk to even offset*/

/**/
/* write CMAP*/
/**/
	*lbmptr++ = 'C';
	*lbmptr++ = 'M';
	*lbmptr++ = 'A';
	*lbmptr++ = 'P';

	cmaplength = (int *)lbmptr;
	lbmptr+=4;                      /* leave space for length*/

	memcpy (lbmptr,palette,768);
	lbmptr += 768;

	length = lbmptr-(byte *)cmaplength-4;
	*cmaplength = BigLong(length);
	if (length&1)
		*lbmptr++ = 0;          /* pad chunk to even offset*/

/**/
/* write BODY*/
/**/
	*lbmptr++ = 'B';
	*lbmptr++ = 'O';
	*lbmptr++ = 'D';
	*lbmptr++ = 'Y';

	bodylength = (int *)lbmptr;
	lbmptr+=4;                      /* leave space for length*/

	memcpy (lbmptr,data,width*height);
	lbmptr += width*height;

	length = lbmptr-(byte *)bodylength-4;
	*bodylength = BigLong(length);
	if (length&1)
		*lbmptr++ = 0;          /* pad chunk to even offset*/

/**/
/* done*/
/**/
	length = lbmptr-(byte *)formlength-4;
	*formlength = BigLong(length);
	if (length&1)
		*lbmptr++ = 0;          /* pad chunk to even offset*/

/**/
/* write output file*/
/**/
	SaveFile (filename, lbm, lbmptr-lbm);
	Q_free (lbm);
}
#endif

byte pal[768]=
  {0x00,0x00,0x00,0x0F,0x0F,0x0F,0x1F,0x1F,0x1F,0x2F,0x2F,0x2F,0x3F,0x3F,0x3F,0x4B,
   0x4B,0x4B,0x5B,0x5B,0x5B,0x6B,0x6B,0x6B,0x7B,0x7B,0x7B,0x8B,0x8B,0x8B,0x9B,0x9B,
   0x9B,0xAB,0xAB,0xAB,0xBB,0xBB,0xBB,0xCB,0xCB,0xCB,0xDB,0xDB,0xDB,0xEB,0xEB,0xEB,
   0x0F,0x0B,0x07,0x17,0x0F,0x0B,0x1F,0x17,0x0B,0x27,0x1B,0x0F,0x2F,0x23,0x13,0x37,
   0x2B,0x17,0x3F,0x2F,0x17,0x4B,0x37,0x1B,0x53,0x3B,0x1B,0x5B,0x43,0x1F,0x63,0x4B,
   0x1F,0x6B,0x53,0x1F,0x73,0x57,0x1F,0x7B,0x5F,0x23,0x83,0x67,0x23,0x8F,0x6F,0x23,
   0x0B,0x0B,0x0F,0x13,0x13,0x1B,0x1B,0x1B,0x27,0x27,0x27,0x33,0x2F,0x2F,0x3F,0x37,
   0x37,0x4B,0x3F,0x3F,0x57,0x47,0x47,0x67,0x4F,0x4F,0x73,0x5B,0x5B,0x7F,0x63,0x63,
   0x8B,0x6B,0x6B,0x97,0x73,0x73,0xA3,0x7B,0x7B,0xAF,0x83,0x83,0xBB,0x8B,0x8B,0xCB,
   0x00,0x00,0x00,0x07,0x07,0x00,0x0B,0x0B,0x00,0x13,0x13,0x00,0x1B,0x1B,0x00,0x23,
   0x23,0x00,0x2B,0x2B,0x07,0x2F,0x2F,0x07,0x37,0x37,0x07,0x3F,0x3F,0x07,0x47,0x47,
   0x07,0x4B,0x4B,0x0B,0x53,0x53,0x0B,0x5B,0x5B,0x0B,0x63,0x63,0x0B,0x6B,0x6B,0x0F,
   0x07,0x00,0x00,0x0F,0x00,0x00,0x17,0x00,0x00,0x1F,0x00,0x00,0x27,0x00,0x00,0x2F,
   0x00,0x00,0x37,0x00,0x00,0x3F,0x00,0x00,0x47,0x00,0x00,0x4F,0x00,0x00,0x57,0x00,
   0x00,0x5F,0x00,0x00,0x67,0x00,0x00,0x6F,0x00,0x00,0x77,0x00,0x00,0x7F,0x00,0x00,
   0x13,0x13,0x00,0x1B,0x1B,0x00,0x23,0x23,0x00,0x2F,0x2B,0x00,0x37,0x2F,0x00,0x43,
   0x37,0x00,0x4B,0x3B,0x07,0x57,0x43,0x07,0x5F,0x47,0x07,0x6B,0x4B,0x0B,0x77,0x53,
   0x0F,0x83,0x57,0x13,0x8B,0x5B,0x13,0x97,0x5F,0x1B,0xA3,0x63,0x1F,0xAF,0x67,0x23,
   0x23,0x13,0x07,0x2F,0x17,0x0B,0x3B,0x1F,0x0F,0x4B,0x23,0x13,0x57,0x2B,0x17,0x63,
   0x2F,0x1F,0x73,0x37,0x23,0x7F,0x3B,0x2B,0x8F,0x43,0x33,0x9F,0x4F,0x33,0xAF,0x63,
   0x2F,0xBF,0x77,0x2F,0xCF,0x8F,0x2B,0xDF,0xAB,0x27,0xEF,0xCB,0x1F,0xFF,0xF3,0x1B,
   0x0B,0x07,0x00,0x1B,0x13,0x00,0x2B,0x23,0x0F,0x37,0x2B,0x13,0x47,0x33,0x1B,0x53,
   0x37,0x23,0x63,0x3F,0x2B,0x6F,0x47,0x33,0x7F,0x53,0x3F,0x8B,0x5F,0x47,0x9B,0x6B,
   0x53,0xA7,0x7B,0x5F,0xB7,0x87,0x6B,0xC3,0x93,0x7B,0xD3,0xA3,0x8B,0xE3,0xB3,0x97,
   0xAB,0x8B,0xA3,0x9F,0x7F,0x97,0x93,0x73,0x87,0x8B,0x67,0x7B,0x7F,0x5B,0x6F,0x77,
   0x53,0x63,0x6B,0x4B,0x57,0x5F,0x3F,0x4B,0x57,0x37,0x43,0x4B,0x2F,0x37,0x43,0x27,
   0x2F,0x37,0x1F,0x23,0x2B,0x17,0x1B,0x23,0x13,0x13,0x17,0x0B,0x0B,0x0F,0x07,0x07,
   0xBB,0x73,0x9F,0xAF,0x6B,0x8F,0xA3,0x5F,0x83,0x97,0x57,0x77,0x8B,0x4F,0x6B,0x7F,
   0x4B,0x5F,0x73,0x43,0x53,0x6B,0x3B,0x4B,0x5F,0x33,0x3F,0x53,0x2B,0x37,0x47,0x23,
   0x2B,0x3B,0x1F,0x23,0x2F,0x17,0x1B,0x23,0x13,0x13,0x17,0x0B,0x0B,0x0F,0x07,0x07,
   0xDB,0xC3,0xBB,0xCB,0xB3,0xA7,0xBF,0xA3,0x9B,0xAF,0x97,0x8B,0xA3,0x87,0x7B,0x97,
   0x7B,0x6F,0x87,0x6F,0x5F,0x7B,0x63,0x53,0x6B,0x57,0x47,0x5F,0x4B,0x3B,0x53,0x3F,
   0x33,0x43,0x33,0x27,0x37,0x2B,0x1F,0x27,0x1F,0x17,0x1B,0x13,0x0F,0x0F,0x0B,0x07,
   0x6F,0x83,0x7B,0x67,0x7B,0x6F,0x5F,0x73,0x67,0x57,0x6B,0x5F,0x4F,0x63,0x57,0x47,
   0x5B,0x4F,0x3F,0x53,0x47,0x37,0x4B,0x3F,0x2F,0x43,0x37,0x2B,0x3B,0x2F,0x23,0x33,
   0x27,0x1F,0x2B,0x1F,0x17,0x23,0x17,0x0F,0x1B,0x13,0x0B,0x13,0x0B,0x07,0x0B,0x07,
   0xFF,0xF3,0x1B,0xEF,0xDF,0x17,0xDB,0xCB,0x13,0xCB,0xB7,0x0F,0xBB,0xA7,0x0F,0xAB,
   0x97,0x0B,0x9B,0x83,0x07,0x8B,0x73,0x07,0x7B,0x63,0x07,0x6B,0x53,0x00,0x5B,0x47,
   0x00,0x4B,0x37,0x00,0x3B,0x2B,0x00,0x2B,0x1F,0x00,0x1B,0x0F,0x00,0x0B,0x07,0x00,
   0x00,0x00,0xFF,0x0B,0x0B,0xEF,0x13,0x13,0xDF,0x1B,0x1B,0xCF,0x23,0x23,0xBF,0x2B,
   0x2B,0xAF,0x2F,0x2F,0x9F,0x2F,0x2F,0x8F,0x2F,0x2F,0x7F,0x2F,0x2F,0x6F,0x2F,0x2F,
   0x5F,0x2B,0x2B,0x4F,0x23,0x23,0x3F,0x1B,0x1B,0x2F,0x13,0x13,0x1F,0x0B,0x0B,0x0F,
   0x2B,0x00,0x00,0x3B,0x00,0x00,0x4B,0x07,0x00,0x5F,0x07,0x00,0x6F,0x0F,0x00,0x7F,
   0x17,0x07,0x93,0x1F,0x07,0xA3,0x27,0x0B,0xB7,0x33,0x0F,0xC3,0x4B,0x1B,0xCF,0x63,
   0x2B,0xDB,0x7F,0x3B,0xE3,0x97,0x4F,0xE7,0xAB,0x5F,0xEF,0xBF,0x77,0xF7,0xD3,0x8B,
   0xA7,0x7B,0x3B,0xB7,0x9B,0x37,0xC7,0xC3,0x37,0xE7,0xE3,0x57,0x7F,0xBF,0xFF,0xAB,
   0xE7,0xFF,0xD7,0xFF,0xFF,0x67,0x00,0x00,0x8B,0x00,0x00,0xB3,0x00,0x00,0xD7,0x00,
   0x00,0xFF,0x00,0x00,0xFF,0xF3,0x93,0xFF,0xF7,0xC7,0xFF,0xFF,0xFF,0x9F,0x5B,0x53};

int   w[2][3];
int   wp;

inline static int dith(int rv,int gv,int bv)
{
   int   point;
   int   best;
   int   c;
   int   cpoint;

/*   printf("dithering (%i %i %i)\n",rv,gv,bv);*/
   for (c=224;c<256;c++)
   {
      cpoint=(pal[c*3+0]-rv)*(pal[c*3+0]-rv)+
             (pal[c*3+1]-gv)*(pal[c*3+1]-gv)+
             (pal[c*3+2]-bv)*(pal[c*3+2]-bv);
      if (cpoint==0)
      {
/*         printf("perfect match %i\n",c);*/
         return c;
      }
   }
   point=1<<31-1;
   best=0;
   for (c=0;c<256;c++)
   {
      cpoint=(pal[c*3+0]-rv)*(pal[c*3+0]-rv)+
             (pal[c*3+1]-gv)*(pal[c*3+1]-gv)+
             (pal[c*3+2]-bv)*(pal[c*3+2]-bv);
      if (cpoint<point)
      {
         best=c;
         point=cpoint;
      }
      if (cpoint==0)
      {
/*         printf("perfect match %i\n",c);*/
         return c;
      }
   }
/*   printf("best match %i (%i %i %i)\n",
          best,pal[best*3],pal[best*3+1],pal[best*3+2]);*/
   if (point>wp)
   {
      wp=point;
      w[0][0]=rv;
      w[0][1]=gv;
      w[0][2]=bv;
      w[1][0]=pal[best*3];
      w[1][1]=pal[best*3+1];
      w[1][2]=pal[best*3+2];
   }
   return best;
}

typedef struct
{
   char  id[2] __attribute__ ((packed));
   int   size __attribute__ ((packed));
   int   res __attribute__ ((packed));
   int   offbits __attribute__ ((packed));
} bmpfh_t;

typedef struct
{
   int   size __attribute__ ((packed));
   int   width,height __attribute__ ((packed));
   short planes __attribute__ ((packed));
   short bitcount __attribute__ ((packed));
   int   comp __attribute__ ((packed));
   int   isize __attribute__ ((packed));
   int   xpm,ypm __attribute__ ((packed));
   int   clru,clri __attribute__ ((packed));
} bmpih_t;

void LoadLBM (char *filename, byte **picture, byte **palette)
{
   byte  *picbuffer,*cmapbuffer,*cmap2;
   byte  *pic_p;
   FILE *f;
   bmpfh_t fh;
   bmpih_t ih;
   int     x,y;
   int   lt[256];

   f=SafeOpenRead(filename);
   SafeRead(f,&fh,sizeof(fh));
   SafeRead(f,&ih,sizeof(ih));
   picbuffer=Q_malloc(ih.width*ih.height);
   cmapbuffer=Q_malloc(1024);
   SafeRead(f,cmapbuffer,1024);
   cmap2=Q_malloc(768);
   for (y=0;y<256;y++)
   {
      cmap2[y*3]=cmapbuffer[y*4+2];
      cmap2[y*3+1]=cmapbuffer[y*4+1];
      cmap2[y*3+2]=cmapbuffer[y*4];
   }
   Q_free(cmapbuffer);
   fseek(f,fh.offbits,SEEK_SET);
   (int)pic_p=(int)picbuffer+ih.width*(ih.height-1);
   wp=0;
/*   for (x=0;x<256;x++)
      lt[x]=dith(cmap2[x*3],
                 cmap2[x*3+1],
                 cmap2[x*3+2]);*/
   memset(lt,0,sizeof(lt));
   for (y=0;y<ih.height;y++)
   {
      SafeRead(f,pic_p,ih.width);
      for (x=0;x<ih.width;x++)
      {
         if (pic_p[x])
         {
            if (!lt[pic_p[x]])
               lt[pic_p[x]]=dith(cmap2[pic_p[x]*3],cmap2[pic_p[x]*3+1],cmap2[pic_p[x]*3+2]);
            pic_p[x]=lt[pic_p[x]];
         }
      }
      (int)pic_p-=ih.width;
   }
   bmhd.w=ih.width;
   bmhd.h=ih.height;
   fclose(f);
	*picture = picbuffer;
	*palette = cmap2;
   printf("worst dither: %i (%i %i %i) to (%i %i %i)\n",
           wp,
           w[0][0],w[0][1],w[0][2],
           w[1][0],w[1][1],w[1][2]);
}

void WriteLBMfile (char *filename, byte *data, int width, int height, byte *palette)
{
   FILE     *f;
   byte     *datap;
   bmpfh_t  bf;
   bmpih_t  ih;
   int      y;

   f=SafeOpenWrite(filename);
   bf.id[0]='B';
   bf.id[1]='M';
   bf.size=(sizeof(bmpfh_t)+sizeof(bmpih_t)+1024+width*height)/4;
   bf.res=0;
   bf.offbits=sizeof(bmpfh_t)+sizeof(bmpih_t)+1024;
   SafeWrite(f,&bf,sizeof(bmpfh_t));
   ih.size=sizeof(bmpih_t);
   ih.width=width;
   ih.height=height;
   ih.planes=1;
   ih.bitcount=8;
   ih.comp=0;
   ih.isize=width*height;
   ih.xpm=100;
   ih.ypm=100;
   ih.clru=ih.clri=0;
   SafeWrite(f,&ih,sizeof(bmpih_t));
   for (y=0;y<256;y++)
   {
      fputc(palette[y*3+2],f);
      fputc(palette[y*3+1],f);
      fputc(palette[y*3],f);
      fputc(0,f);
   }
/*   SafeWrite(f,palette,768);*/
   (int)datap=(int)data+width*(height-1);
   for (y=0;y<height;y++)
   {
      SafeWrite(f,datap,width);
      (int)datap-=width;
   }
   fclose(f);
}
