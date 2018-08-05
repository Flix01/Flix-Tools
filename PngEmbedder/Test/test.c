// https://github.com/Flix01/Flix-Tools
//
// TO COMPILE THIS ON LINUX:
//
// gcc -Os -no-pie test.c -o test
//

#ifndef ASSERT
#include <assert.h>
#define ASSERT(X) assert(X)
#endif

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


/* #define USE_BIG_ENDIAN_MACHINE // define this on big endian machines */

void DecodeImage(unsigned* pPixelsOut,unsigned* palette,const unsigned numPalette,const char* indices) {
    unsigned hasReps=0,*pRaw = pPixelsOut;(void)(numPalette);
    const char* pc;char lastChar=(char)255,c,j;
#   ifdef USE_BIG_ENDIAN_MACHINE /* define this on big endian machines */
    unsigned tmp;const unsigned char* pTmp = (const unsigned char*) &tmp;
    unsigned char* ppal = (unsigned char*) palette;
    for (i=0;i<numPalette;i++) {
        tmp = palette[i];   /* Not sure this is correct: */
        *ppal++ = pTmp[3];  *ppal++ = pTmp[2];  *ppal++ = pTmp[1];  *ppal++ = pTmp[0];
    }
#   endif
    for (pc=indices;*pc!='\0';++pc)  {
        c = *pc;if (hasReps==0 && c=='~') {hasReps=1;continue;}
        c = c>='\\' ? (c-'1') : (c-'0');
        if (hasReps) {
            hasReps = palette[(unsigned char)lastChar];
            for (j=0;j<c;j++) *pRaw++ = hasReps;
            hasReps=0;continue;
        }
        *pRaw++ = palette[(unsigned char)c];lastChar = c;
    }
}

/* Longer version with a longer signature and asserts (dbg only) */
void DecodeImageDebug(unsigned* pRGBAOut,int w,int h,unsigned* palette,const unsigned numPalette,const char* indices) {
    unsigned j,cnt=0,hasReps=0;const unsigned area = (unsigned) (w*h);
    unsigned* pRaw = pRGBAOut;char lastChar=(char)255,c;const char* pc = indices;
#   ifdef USE_BIG_ENDIAN_MACHINE /* define this on big endian machines */
    unsigned tmp;const unsigned char* pTmp = (const unsigned char*) &tmp;
    unsigned char* ppal = (unsigned char*) palette;
    for (i=0;i<numPalette;i++) {
        tmp = palette[i];
        // Not sure this is correct
        *ppal++ = pTmp[3];  *ppal++ = pTmp[2];  *ppal++ = pTmp[1];  *ppal++ = pTmp[0];
    }
#   endif
    for (pc=indices;*pc!='\0';++pc)  {
        c = *pc;
        if (c=='\0') break;
        if (hasReps==0 && c=='~') {
            hasReps=1;
            continue;
        }
        c = c>='\\' ? (char)((int)c-(int)'1') : (char)((int)c-(int)'0');
        if (hasReps) {
            /* Write c times lastChar */
            cnt+=(unsigned)c;
            ASSERT((unsigned)c!=0 && (unsigned)c!=1);
            ASSERT(cnt<=area);
            for (j=0;j<(unsigned)c;j++) *pRaw++ = palette[(unsigned)lastChar];
            hasReps=0;
            continue;
        }
        /* Write c */
        ASSERT(cnt<area);
        ASSERT((unsigned)c<numPalette);
        *pRaw++ = palette[(unsigned)c];
        ++cnt;lastChar = c;
    }
    ASSERT(cnt==area);
}

void DecodeImageInt(unsigned* pPixelsOut,unsigned* palette,const unsigned numPalette,const int* indices,const unsigned numIndices) {
    unsigned pal,i,*pRaw = pPixelsOut;(void)(numPalette);
    int lastIdx=-1,idx=-1,j,numReps=0;
#   ifdef USE_BIG_ENDIAN_MACHINE /* define this on big endian machines */
    unsigned tmp;const unsigned char* pTmp = (const unsigned char*) &tmp;
    unsigned char* ppal = (unsigned char*) palette;
    for (i=0;i<numPalette;i++) {
        tmp = palette[i];   /* Not sure this is correct: */
        *ppal++ = pTmp[3];  *ppal++ = pTmp[2];  *ppal++ = pTmp[1];  *ppal++ = pTmp[0];
    }
#   endif
    for (i=0;i<numIndices;i++)  {
        idx = indices[i];
        if (idx<0) {
            numReps=-idx;pal = palette[lastIdx];
            for (j=0;j<numReps;j++) *pRaw++ = pal;
        }
        else {*pRaw++ = palette[idx];lastIdx = idx;}
    }
}

int main (int argc,char* argv[])
{

    /* I suggest to embed the .inl file directly whenever possible. Anyway: */
#   include "../Tile8x8-nq8.png.inl"


    /*  Now the .inl file can be in 2 flavours:
        1) Compact: indices are stored as chars (if the number of colors in the image is less than about 76 AFAIR)
        2) Larger: indices are stored as integers (not recommended)
        Use DecodeImage(...) for (1) and DecodeImageInt(...) for (2)
    */
    unsigned int raw[width*height];
    DecodeImage(&raw[0],palette,sizeof(palette)/sizeof(palette[0]),*indices);
    /*DecodeImageInt(&raw[0],palette,sizeof(palette)/sizeof(palette[0]),indices,sizeof(indices)/sizeof(indices[0]));*/
    stbi_write_png("image_out.png",width,height,4,(const void*)raw,width*4);

return 0;
}
