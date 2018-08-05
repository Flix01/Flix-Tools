// https://github.com/Flix01/Flix-Tools
//
// TO COMPILE THIS ON LINUX:
//
// gcc -Os --std=gnu89 -no-pie pngEmbedder.c -o pngEmbedder -lm
//

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#ifndef ASSERT
#include <assert.h>
#define ASSERT(X) assert(X)
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


//#define INVERT_FLIPY_DEFAULT
//#define NO_USAGE_IN_INL_FILES

static const char* decodeImageUsage[] = {"void DecodeImage(unsigned* pPixelsOut,unsigned* palette,const unsigned numPalette,const char* indices) {\n"
                                         "    unsigned hasReps=0,*pRaw = pPixelsOut;(void)(numPalette);\n"
                                         "    const char* pc;char lastChar=(char)255,c,j;\n"
                                         "#   ifdef USE_BIG_ENDIAN_MACHINE /* define this on big endian machines */\n"
                                         "    unsigned tmp;const unsigned char* pTmp = (const unsigned char*) &tmp;\n"
                                         "    unsigned char* ppal = (unsigned char*) palette;\n"
                                         "    for (i=0;i<numPalette;i++) {\n"
                                         "        tmp = palette[i];   /* Not sure this is correct: */\n"
                                         "        *ppal++ = pTmp[3];  *ppal++ = pTmp[2];  *ppal++ = pTmp[1];  *ppal++ = pTmp[0];\n"
                                         "    }\n"
                                         "#   endif\n"
                                         "    for (pc=indices;*pc!='\\0';++pc)  {\n"
                                         "        c = *pc;if (hasReps==0 && c=='~') {hasReps=1;continue;}\n"
                                         "        c = c>='\\\\' ? (c-'1') : (c-'0');\n"
                                         "        if (hasReps) {\n"
                                         "            hasReps = palette[(unsigned char)lastChar];\n"
                                         "            for (j=0;j<c;j++) *pRaw++ = hasReps;\n"
                                         "            hasReps=0;continue;\n"
                                         "        }\n"
                                         "        *pRaw++ = palette[(unsigned char)c];lastChar = c;\n"
                                         "    }\n"
                                         "}\n\n"
                                         "// unsigned int raw[width*height];\n"
                                         "// DecodeImage(&raw[0],palette,sizeof(palette)/sizeof(palette[0]),*indices);\n"
};

static const char* decodeImageIntUsage[] = {"void DecodeImageInt(unsigned* pPixelsOut,unsigned* palette,const unsigned numPalette,const int* indices,const unsigned numIndices) {\n"
                                         "    unsigned pal,i,*pRaw = pPixelsOut;(void)(numPalette);\n"
                                         "    int lastIdx=-1,idx=-1,j,numReps=0;\n"
                                         "#   ifdef USE_BIG_ENDIAN_MACHINE /* define this on big endian machines */\n"
                                         "    unsigned tmp;const unsigned char* pTmp = (const unsigned char*) &tmp;\n"
                                         "    unsigned char* ppal = (unsigned char*) palette;\n"
                                         "    for (i=0;i<numPalette;i++) {\n"
                                         "        tmp = palette[i];   /* Not sure this is correct: */\n"
                                         "        *ppal++ = pTmp[3];  *ppal++ = pTmp[2];  *ppal++ = pTmp[1];  *ppal++ = pTmp[0];\n"
                                         "    }\n"
                                         "#   endif\n"
                                         "    for (i=0;i<numIndices;i++)  {\n"
                                         "        idx = indices[i];\n"
                                         "        if (idx<0) {\n"
                                         "            numReps=-idx;pal = palette[lastIdx];\n"
                                         "            for (j=0;j<numReps;j++) *pRaw++ = pal;\n"
                                         "        }\n"
                                         "        else {*pRaw++ = palette[idx];lastIdx = idx;}\n"
                                         "    }\n"
                                         "}\n\n"
                                         "// unsigned int raw[width*height];\n"
                                         "// DecodeImageInt(&raw[0],palette,sizeof(palette)/sizeof(palette[0]),indices,sizeof(indices)/sizeof(indices[0]));\n"
                                         };





typedef struct {
    unsigned int color;
    unsigned long reps;
} PaletteEntry;
static __inline int PaletteEntryCmp(const void* pa,const void* pb) {
    const unsigned long* pra = &((const PaletteEntry*)pa)->reps;
    const unsigned long* prb = &((const PaletteEntry*)pb)->reps;
    return (*pra<*prb ? 1 : (*pra>*prb ? -1 : 0));
}


int main(int argc,char* argv[]) {
    const char* filename = NULL;
    char savename[2049] = "";
    int i,j;FILE* f = NULL;
    const int numCharsPerLine = 800;int numChars=0;
    unsigned long cnt=0,numReps=0;char lastChar='+',curChar='+';
    int lastIdx=-1;
    int flipOptionUsed = 0;

    {
        flipOptionUsed = (argc==3 && (strlen(argv[1])==2 && argv[1][0]=='-' && argv[1][1]=='f'));
        if (argc<2 || argc>3 || (argc==3 && !flipOptionUsed)) {
            printf("pngEmbedder embeds a paletted png into C/C++ source code.\n");
            printf("USAGE: pngEmbedder inputFile.png\n");
            printf("FLIPY: pngEmbedder -f inputFile.png\n");
            printf("Tip: use: pngnq -n 75 inputFile.png\n");
            printf("to get a suitable paletted image.\n");
            return -1;
        }
#       ifndef INVERT_FLIPY_DEFAULT
        stbi_set_flip_vertically_on_load(flipOptionUsed);
#       else
        stbi_set_flip_vertically_on_load(!flipOptionUsed);
#       endif

        filename = argv[argc-1];

        if (strlen(filename)+4>=2048) {
            printf("Error input file path is too long.\n");
            return -1;
        }
        strcpy(savename,filename);
        strcat(savename,".inl");
    }

#   ifdef NEVER
    const int numVarNames = 26 /* A-Z */ + 24 /* a-z without i,j */ + 52 * (26*2+10);   // = 3274
    char varNames[numVarNames][3];
    {
        int cnt=0;
        for (i=0;i<26;i++) {varNames[cnt][0]=(char)(i+'A');varNames[cnt++][1]='\0';}
        for (i=0;i<26;i++) {
            if ((char)(i+'a') == 'i' || (char)(i+'a')=='j') continue;
            varNames[cnt][0]=(char)(i+'a');
            varNames[cnt++][1]='\0';
        }
        for (i=0;i<52;i++) {
            for (j=0;j<62;j++) {
                varNames[cnt][0]=(char)(i<26?(i+'A'):(i-26+'a'));
                varNames[cnt][1]=(char)(j<26?(j+'A'):(j<52?(j-26+'a'):(j-52+'0')));
                varNames[cnt++][2]='\0';
            }
        }
        ASSERT(cnt==numVarNames);
        //for (i=0;i<numVarNames;i++) printf("%d)\t\"%s\"\n",i,varNames[i]);
    }
#   endif //NEVER

    int w=0,h=0,c=0,area=0;
    unsigned char* raw = stbi_load(filename,&w,&h,&c,4);
    {
        if (!raw) {
            printf("Filename \"%s\" invalid (not found or invalid image file).\n",filename);
            return -2;
        }
        if (c!=4) printf("Converting \"%s\" to RGBA (the only supported number of channels).\n",filename);
        area=w*h;
    }

    const int maxNumPaletteEntries = 500;
    PaletteEntry palette[maxNumPaletteEntries];
    int numPaletteColors = 0;
    for (i=0;i<maxNumPaletteEntries;i++) {palette[i].color=0;palette[i].reps=0;}
    const unsigned int* pRaw = (const unsigned int*) raw;
    for (i=0;i<area;i++) {
        if (numPaletteColors+1>=maxNumPaletteEntries) {
            printf("Error: Too many colors in input image. Please use pngnq to reduce them.\n");
            if (raw) {STBI_FREE(raw);raw=NULL;}
            return -3;
        }
        ASSERT(numPaletteColors+1<maxNumPaletteEntries);    // Otherwise we should increase maxNumPaletteEntries
        for (j=0;j<numPaletteColors+1;j++) {
            PaletteEntry* pe = &palette[j];
            if (pe->reps==0) {
                ++pe->reps;
                pe->color=*pRaw;
                numPaletteColors=j+1;
                break;
            }
            if (pe->color==*pRaw) {++pe->reps;break;}
        }
        ++pRaw;
    }
    qsort(&palette[0],numPaletteColors,sizeof(PaletteEntry),&PaletteEntryCmp);
    /*for (j=0;j<numPaletteColors;j++) {
        PaletteEntry* pe = &palette[j];
        printf("palette[%d] = %uU (reps=%lu)\n",j,pe->color,pe->reps);
    }*/

    if (numPaletteColors==0) {
        printf("Error: numPaletteColors==0.\n");
        if (raw) {STBI_FREE(raw);raw=NULL;}
        return -4;
    }

    f = fopen(savename,"wt");
    if (!f) {
        printf("Error: cannot save file: \"%s\".\n",savename);
        if (raw) {STBI_FREE(raw);raw=NULL;}
        return -5;
    }

    // Write image dimensions
    fprintf(f,"const int width = %d, height = %d;\n\n",w,h);

    // Write Palette Here
    numChars = 0;
    fprintf(f,"/* RGBA palette in little-endian encoding */\n");
    fprintf(f,"unsigned int palette[%d] = {\n",numPaletteColors);
    for (j=0;j<numPaletteColors;j++) {
        PaletteEntry* pe = &palette[j];
        fprintf(f,"%uU",pe->color);numChars+=pe->color==0?2:11;
        if (j<numPaletteColors-1) {fprintf(f,",");++numChars;}
        if (numChars>=numCharsPerLine) {fprintf(f,"\n");numChars=0;}
    }
    fprintf(f,"};\n\n");

    // Write indices here
    numChars = 0;
    pRaw = (const unsigned int*) raw;
    if (numPaletteColors>76) {
        fprintf(f,"/* int indices into palette (%dx%d once uncompressed). Negative values represent further repetitions of the last index */\n",w,h);
        fprintf(f,"const int indices[] = {\n");
        for (i=0;i<area;i++) {
            for (j=0;j<numPaletteColors;j++) {
                const PaletteEntry* pe = &palette[j];
                if (pe->color==*pRaw) {
                    if (lastIdx==j) {++numReps;}
                    else {
                        if (numChars>0) {fprintf(f,",");++numChars;}
                        if (numReps==1) {
                            // Write 'lastIdx' and 'j'
                            fprintf(f,"%d",lastIdx);numChars+=lastIdx<10?1:(lastIdx<100?2:(lastIdx<1000?3:lastIdx<10000?4:lastIdx<100000?5:6));
                            fprintf(f,",");++numChars;
                            if (numChars>=numCharsPerLine) {fprintf(f,"\n");numChars=0;}
                        }
                        else if (numReps>1) {
                            // Write '-numReps' and 'j'
                            fprintf(f,"-%lu",numReps);++numChars;numChars+=numReps<10?1:(numReps<100?2:(numReps<1000?3:numReps<10000?4:numReps<100000?5:6));
                            fprintf(f,",");++numChars;
                            if (numChars>=numCharsPerLine) {fprintf(f,"\n");numChars=0;}
                        }
                        // Write 'j'
                        fprintf(f,"%d",j);numChars+=j<10?1:(j<100?2:(j<1000?3:j<10000?4:j<100000?5:6));
                        lastIdx=j;numReps=0;
                    }
                    break;
                }
            }
            if (numChars>=numCharsPerLine) {
                if (i<area-1) fprintf(f,",");
                fprintf(f,"\n");numChars=0;
            }
            ++pRaw;
        }
        fprintf(f,"};\n\n");

#       ifndef NO_USAGE_IN_INL_FILES
        fprintf(f,"#ifdef USAGE\n%s\n#endif //USAGE\n\n",*decodeImageIntUsage);
#       endif
    }
    else {
        char minChar = 'b',maxChar='a';
        fprintf(f,"/* Each entry when uncompressed to %d*%d represents an index into the palette array */\n",w,h);
        fprintf(f,"const char* indices[] = {\n");
        for (i=0;i<area;i++) {
            for (j=0;j<numPaletteColors;j++) {
                const PaletteEntry* pe = &palette[j];
                if (pe->color==*pRaw) {

                    curChar = (char)(j+(int)'0');
                    if (curChar>='\\') curChar = (char) ((int)curChar+1);

                    if (minChar>curChar) minChar=curChar;
                    if (maxChar<curChar) maxChar=curChar;
                    ASSERT(curChar!='\\');

                    if (lastChar==curChar && numReps<76) {++numReps;}
                    else {
                        if (numChars==0) {fprintf(f,"\"");++numChars;}
                        else if (numReps==0 &&
                                 ((lastChar=='?' && curChar=='?') || (lastChar=='<' && curChar==':') || (lastChar==':' && curChar=='>'))
                                 ) {fprintf(f,"\"\"");++numChars;++numChars;}   // Trigraphs removal
                        if (numReps==1) {
                            // Write 'lastChar' (again)
                            if (lastChar=='?') {fprintf(f,"\"\"");++numChars;++numChars;}   // Trigraphs removal
                            fprintf(f,"%c",lastChar);++numChars;
                            if (numChars>=numCharsPerLine) {fprintf(f,"\"\n\"");numChars=1;}
                            if ((lastChar=='?' && curChar=='?') || (lastChar=='<' && curChar==':') || (lastChar==':' && curChar=='>'))
                                {fprintf(f,"\"\"");++numChars;++numChars;}   // Trigraphs removal
                        }
                        else if (numReps>1) {
                            // Write '~' + numReps
                            char numRepsChar = (char)((int)numReps+(int)'0');
                            if (numRepsChar>='\\') numRepsChar = (char) ((int)numRepsChar+1);
                            ASSERT(numRepsChar!='\\' && numRepsChar>='0' && numRepsChar<'~');
                            fprintf(f,"~%c",numRepsChar);++numChars;++numChars;
                            if (numChars>=numCharsPerLine) {fprintf(f,"\"\n\"");numChars=1;}
                            if ((numRepsChar=='?' && curChar=='?') || (numRepsChar=='<' && curChar==':') || (numRepsChar==':' && curChar=='>'))
                                {fprintf(f,"\"\"");++numChars;++numChars;}   // Trigraphs removal
                        }
                        // Write 'curChar'
                        fprintf(f,"%c",curChar);++numChars;
                        lastChar=curChar;numReps=0;
                    }
                    break;
                }
            }
            if (numChars>=numCharsPerLine) {fprintf(f,"\"\n");numChars=0;}
            ++pRaw;
        }
        fprintf(f,"\"};\n\n");

#       ifndef NO_USAGE_IN_INL_FILES
        fprintf(f,"#ifdef USAGE\n%s\n#endif //USAGE\n\n",*decodeImageUsage);
#       endif
    }

    fclose(f);f=NULL;

    if (raw) {STBI_FREE(raw);raw=NULL;}
    //printf("\"%s\" -> \"%s\"OK\n",filename,savename);
    return 0;
}

