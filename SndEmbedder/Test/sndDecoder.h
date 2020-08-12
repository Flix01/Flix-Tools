// https://github.com/Flix01/Flix-Tools
//
/** MIT License
 *
 * Copyright (c) 2020 Flix (https://github.com/Flix01/)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#ifndef SNDDECODER_H_
#define SNDDECODER_H_

#ifndef SNDD_API_DEC
#   define SNDD_API_DEC extern
#endif
#ifndef SNDD_API_DEF
#   define SNDD_API_DEF /* no-op */
#endif

#ifndef SNDD_DEBUG
#   if (!defined(NDEBUG) && !defined(_NDEBUG)) || defined(DEBUG) || defined (_DEBUG)
#       define SNDD_DEBUG
#   endif
#endif

#if (!defined(SNDD_ASSERT) && defined(SNDD_DEBUG))
#include <assert.h>
#define SNDD_ASSERT(X)   assert(X)
#else
#define SNDD_ASSERT(X)   /*no op*/
#endif

#ifndef __cplusplus
#   define SNDD_ZERO_INIT   {0}
#else
#   define SNDD_ZERO_INIT   {}
#endif

#include <stddef.h> // size_t

SNDD_API_DEC size_t sndd_DecodeSerializedSound(float** psamples,size_t* sampleSizeInOut,const char* input,int samplerate,int num_channels);


#ifdef SNDD_HAS_WAV_SAVING_SUPPORT
SNDD_API_DEC size_t sndd_SaveAsWavFile(const char* savePath,const float* samples,size_t num_samples,int samplerate,int num_channels);
#endif //SNDD_HAS_WAV_SAVING_SUPPORT

#ifndef SNDD_MALLOC
#   include <stdlib.h>
#   define SNDD_MALLOC(X)  malloc(X)
#endif
#ifndef SNDD_REALLOC
#   include <stdlib.h>
#   define SNDD_REALLOC(X,Y)  realloc(X,Y)
#endif
#ifndef SNDD_FREE
#   include <stdlib.h>
#   define SNDD_FREE(X)  free(X)
#endif

#endif // SNDDECODER_H_



#ifdef SNDDECODER_IMPLEMENTATION
#ifndef SNDDECODER_IMPLEMENTATION_GUARD
#define SNDDECODER_IMPLEMENTATION_GUARD

#include <math.h>
#ifndef M_PI
#   define M_PI		3.14159265358979323846	/* pi */
#endif



#ifndef SNDD_NO_C99_MATH_FUNCTIONS
#   define sndd_sin(X)   sinf(X)
#   define sndd_cos(X)   cosf(X)
#   if (defined(GNU_SOURCE) || defined(SNDD_HAS_SINCOS))
#       define sndd_sincos(ANGLE,PSIN,PCOS)  {sincosf((ANGLE),(PSIN),(PCOS));} // old compilers
#   else
#       define sndd_sincos(ANGLE,PSIN,PCOS)  {*PSIN=sinf(ANGLE);*PCOS=cosf(ANGLE);}  // new compilers can optimize this in -O2 or -O3
#   endif
#else   //SNDD_NO_C99_MATH_FUNCTIONS
#   define sndd_sin(X)   sin(X)
#   define sndd_cos(X)   cos(X)
#   if (defined(GNU_SOURCE) || defined(SNDD_HAS_SINCOS))
#       define sndd_sincos(ANGLE,PSIN,PCOS)  {sincos((ANGLE),(PSIN),(PCOS));} // old compilers
#   else
#       define sndd_sincos(ANGLE,PSIN,PCOS)  {*PSIN=sin(ANGLE);*PCOS=cos(ANGLE);}  // new compilers can optimize this in -O2 or -O3
#   endif
#endif //SNDD_NO_C99_MATH_FUNCTIONS



static size_t sndd_Base85Decode(const char* input, char** poutput, size_t* poutputSizeInOut)    {
    // Based on code contained in imgui_draw.cpp (Dear ImGui at https://github.com/ocornut/imgui, MIT licensed)
#   define SNDD_DECODE85BYTE(c)   (((c) >= '\\') ? ((c)-36) : ((c)-35))
    const size_t outputSize = (((size_t)strlen(input) + 4) / 5) * 4;
    const unsigned char *src = (const unsigned char *) input;
    unsigned char *dst = (unsigned char *) *poutput;
    SNDD_ASSERT(input);
    SNDD_ASSERT(poutput && poutputSizeInOut);
    if (!input) return 0;
    if (!*poutput || *poutputSizeInOut<outputSize) {
        *poutput = (char*)SNDD_REALLOC(*poutput,outputSize);
        dst = (unsigned char *) *poutput;
        *poutputSizeInOut = outputSize;
    }
    while (*src)    {
        unsigned int tmp = SNDD_DECODE85BYTE(src[0]) + 85*(SNDD_DECODE85BYTE(src[1]) + 85*(SNDD_DECODE85BYTE(src[2]) + 85*(SNDD_DECODE85BYTE(src[3]) + 85*SNDD_DECODE85BYTE(src[4]))));
        dst[0] = ((tmp >> 0) & 0xFF); dst[1] = ((tmp >> 8) & 0xFF); dst[2] = ((tmp >> 16) & 0xFF); dst[3] = ((tmp >> 24) & 0xFF);   // We can't assume little-endianness.
        src += 5;
        dst += 4;
    }
    return outputSize;
#   undef SNDD_DECODE85BYTE
}


// This must match with the one in sndEmbedder.c
struct sndDFTserializerHeader_t {
    char tag[4];                     // "D0L0"  (marker + version + [[L]ittle-endian or [B]ig-endian] + extra-compression)
    unsigned sizeOfThisStruct;
    unsigned num_samples;            // size of input signal
    unsigned size_pot;               // == snd_calculate_next_power_of_two(num_samples)
    unsigned num_dft_samples_to_keep;   // in [0,size_pot/2-1]
    unsigned short samplerate;       // SND_SAMPLERATE, 22050
    unsigned char num_channels;      // MUST be 1
    unsigned char log2size;         // SND_ASSERT(1<<log2size == size_pot);
    unsigned char num_bits_for_modulo_in_high_precision_data;   // default: 8, in [2,14]
    unsigned char num_bits_for_modulo_in_low_precision_data;    // default: 4, in [2,6]
    unsigned char uc_private0;
    unsigned char uc_private1;

    float moduloMax;
    float dft0_re,dftH_re;
    float regularBandModuloTrimmingThreshold;
    float regularBandModuloDequantizationThreshold;

    unsigned num_intervals;             // in each interval, a local 'moduloMax' is stored for better modulo quantization
    unsigned num_samples_per_interval;  // depends on num_dft_samples_to_keep

    unsigned num_high_precision_data;
    unsigned num_low_precision_data;
    unsigned num_skipped_data;

    unsigned u_private[4];

    // total size = 64 + 5*sizeof(float) = 84 bytes
};

static size_t sndd_DeserializeDiscreteFourierTransform(float dft_out[][2], size_t dft_out_size, const unsigned char* bufferIn, size_t bufferInSize) {
    const struct sndDFTserializerHeader_t* S = NULL;
    const unsigned char* buf = bufferIn;
    const unsigned short* dft_high_precision_data = NULL;
    const unsigned char* dft_low_precision_data = NULL;
    const unsigned short* maxModulos = NULL;
    unsigned i;unsigned char mode = 0;
    unsigned cnt,cnt_high,cnt_low,cnt_skipped,interval_index;float tmp;
    const float MaxUnsignedShort = (float) 65535;
    float  MaxHighPrecisionBitValueForModulo = (float) 255;
    float  MaxHighPrecisionBitValueForPhase = (float) 255;
    unsigned short num_bits_for_phase_in_high_precision_data = 8,max_high_precision_bit_value_for_phase = 255;
    float  MaxLowPrecisionBitValueForModulo = (float) 15;
    float  MaxLowPrecisionBitValueForPhase = (float) 15;
    unsigned char num_bits_for_phase_in_low_precision_data = 4,max_low_precision_bit_value_for_phase = 15;
    float modulo,phase,cos_phase,sin_phase;
    long int delta_buf = 0;

    SNDD_ASSERT(dft_out && dft_out_size>=4 && bufferIn && bufferInSize);
    SNDD_ASSERT(sizeof(*S)<=bufferInSize);
    S = (const struct sndDFTserializerHeader_t*) buf;
    SNDD_ASSERT(S->sizeOfThisStruct == (unsigned)sizeof(*S));
    SNDD_ASSERT(S->num_channels==1);
    buf+=sizeof(*S);
    dft_high_precision_data = (const unsigned short*) buf;
    buf+=S->num_high_precision_data*sizeof(*dft_high_precision_data);
    dft_low_precision_data = (const unsigned char*) buf;
    buf+=S->num_low_precision_data*sizeof(*dft_low_precision_data);
    maxModulos = (const unsigned short*) buf;
    buf+=S->num_intervals*sizeof(*maxModulos);
    delta_buf = (long int)(buf-bufferIn) - (long int)bufferInSize;
    SNDD_ASSERT(delta_buf>-4 && delta_buf<4);    // that's because size is clamped to the next integer (4 bytes)

    SNDD_ASSERT((unsigned)dft_out_size>=S->size_pot);

    SNDD_ASSERT(S->num_bits_for_modulo_in_high_precision_data>=2 && S->num_bits_for_modulo_in_high_precision_data<=14);                 // wrong number of modulo bits in num_bits_for_modulo_in_high_precision_data
    num_bits_for_phase_in_high_precision_data = 16-S->num_bits_for_modulo_in_high_precision_data;
    max_high_precision_bit_value_for_phase = (1<<num_bits_for_phase_in_high_precision_data)-1;
    MaxHighPrecisionBitValueForModulo = (float) ((1<<S->num_bits_for_modulo_in_high_precision_data)-1);
    MaxHighPrecisionBitValueForPhase = (float) max_high_precision_bit_value_for_phase;

    SNDD_ASSERT(S->num_bits_for_modulo_in_low_precision_data>=2 && S->num_bits_for_modulo_in_low_precision_data<=6);                 // wrong number of modulo bits in num_bits_for_modulo_in_low_precision_data
    num_bits_for_phase_in_low_precision_data = 8-S->num_bits_for_modulo_in_low_precision_data;
    max_low_precision_bit_value_for_phase = (1<<num_bits_for_phase_in_low_precision_data)-1;
    MaxLowPrecisionBitValueForModulo = (float) ((1<<S->num_bits_for_modulo_in_low_precision_data)-1);
    MaxLowPrecisionBitValueForPhase = (float) max_low_precision_bit_value_for_phase;

    for (i=0;i<(unsigned)dft_out_size;i++) dft_out[i][0]=dft_out[i][1]=0;    // clear all dft_out

    // fill dft_out
    dft_out[0][0] = S->dft0_re;
    dft_out[S->size_pot/2][0] = S->dftH_re;

    SNDD_ASSERT(S->num_dft_samples_to_keep<=S->size_pot/2-1);
    cnt=cnt_high=cnt_low=cnt_skipped=0,interval_index=0;
    tmp=0;mode=0; // 0-> normal, 1-> low quantization interval, 2-> skip interval
    if (S->num_intervals>0)  {
        cnt = 0;tmp = (float)maxModulos[interval_index++]*S->moduloMax/MaxUnsignedShort;
        if (tmp<0) tmp=0;
        else if (tmp>S->moduloMax) tmp=S->moduloMax;
        mode = tmp < S->regularBandModuloTrimmingThreshold ? 2 : (tmp < S->regularBandModuloDequantizationThreshold ? 1 : 0);
    }
    for (i=1;i<=S->num_dft_samples_to_keep;i++)  {
        if (mode < 2)   {
            if (mode==0)    {
                const unsigned short combo = dft_high_precision_data[cnt_high++];
                const unsigned short mdl = combo>>num_bits_for_phase_in_high_precision_data;
                const unsigned short phs = combo&max_high_precision_bit_value_for_phase;
                modulo = ((float)mdl*tmp/MaxHighPrecisionBitValueForModulo);
                phase = ((float)phs*(float)(M_PI*2.0)/MaxHighPrecisionBitValueForPhase - (float)M_PI);
            }
            else    {
                const unsigned char combo = dft_low_precision_data[cnt_low++];
                const unsigned char mdl = combo>>num_bits_for_phase_in_low_precision_data;
                const unsigned char phs = combo&max_low_precision_bit_value_for_phase;
                modulo = ((float)mdl*tmp/MaxLowPrecisionBitValueForModulo);
                phase = ((float)phs*(float)(M_PI*2.0)/MaxLowPrecisionBitValueForPhase - (float)M_PI);
            }
            /*if (modulo<0)           modulo=0;
            else if (modulo>tmp)    modulo=tmp;
            if (phase<-M_PI || phase>M_PI)        phase= -M_PI;
            else if (phase>M_PI)    phase= M_PI;*/
            // Re and Im parts
            sndd_sincos(phase,&sin_phase,&cos_phase);
            dft_out[i][0] = modulo*cos_phase;
            dft_out[i][1] = modulo*sin_phase;
        }
        else {
            dft_out[i][0] = dft_out[i][1]= 0;
            ++cnt_skipped;
        }

        if (++cnt>=S->num_samples_per_interval && interval_index<S->num_intervals)  {
            cnt = 0;tmp = (float)maxModulos[interval_index++]*S->moduloMax/MaxUnsignedShort;
            if (tmp<0) tmp=0;
            else if (tmp>S->moduloMax) tmp=S->moduloMax;
            mode = tmp < S->regularBandModuloTrimmingThreshold ? 2 : (tmp < S->regularBandModuloDequantizationThreshold ? 1 : 0);
        }
    }
    SNDD_ASSERT(cnt_high==S->num_high_precision_data);
    SNDD_ASSERT(cnt_low==S->num_low_precision_data);
    SNDD_ASSERT(cnt_skipped==S->num_skipped_data);

    // To rebuild dft_out completely we're missing this: -----
    dft_out[0][1] = 0;          dft_out[S->size_pot/2][1] = 0;
    for (i=1;i<S->size_pot/2;i++)   {
        const int j = S->size_pot-i;    // j in [S->size_pot-1,S->size_pot/2+1]
        dft_out[j][0] = dft_out[i][0];
        dft_out[j][1] = -dft_out[i][1];
    }
    // --------------------------------------------------------

    return S->size_pot;
}

static void sndd_DiscreteFourierTransform(float out[][2],const float in[][2],int size_pot,int log2size,int inverse_transform,int dont_scale_inverse_transform_like_fftw3_does)  {
    // Code copied from minimath.h (https://github.com/Flix01/Header-Only-GL-Helpers)
    float angle, wtmp, wpr, wpi, wr, wi, tc[2];
    int n = 1, n2, k, m, i, j=0;
    const float pi = (float) M_PI;float sin_angle,cos_angle;

    SNDD_ASSERT(1<<log2size==size_pot);

    // Step (1):
    // Copy 'in' to 'out', rearranging elements so that we can safely operate only on 'out' later.
    // Reference used for step (1): http://librow.com/articles/article-10 [very good article indeed]
    // LICENSE of reference code: The code is property of LIBROW. You can use it on your own when utilizing credit LIBROW site [Hope I've done it with the line above]
    if (in!=out)    {
        for (i = 0; i < size_pot; i++)	{
            m = size_pot;
            out[j][0] = in[i][0]; out[j][1] = in[i][1];
            while (j & (m>>=1))	j&=~m;	// remove m
            j|=m;	// set m
        }
    }
    else    {
#       ifndef SND_SWAP_MACRO
#       define SND_SWAP_MACRO(X,Y,TMP)  {(TMP)=(X);(X)=(Y);(Y)=(TMP);}
#       endif
        for (i=0;i<size_pot;i++)	{
            m = size_pot;
            if (j>i)	{
                //   Swap entries
                SND_SWAP_MACRO(out[j][0],out[i][0],tc[0]);
                SND_SWAP_MACRO(out[j][1],out[i][1],tc[1]);
            }
            while (j & (m>>=1))	j&=~m;	// remove m
            j|=m;	// set m
        }
    }

    // Step (2):
    // Safely operate only on 'out' here
    // Reference used for step (2): https://www.programming-techniques.com/2013/05/calculation-of-discrete-fourier-transformdft-in-c-c-using-naive-and-fast-fourier-transform-fft-method.html
    // LICENSE of reference code: not present (probably public domain). [Otherwise we should stick to LIBROW implementation here too]
    for (k = 0; k < log2size; ++k)	{
        n2 = n; n <<= 1;
        angle = (inverse_transform)?pi/(float)n:-pi/(float)n;   /* half the angle used in the reference code above */
        sndd_sincos(angle,&sin_angle,&cos_angle);    /* it can calculate sin_angle and cos_angle together (in new compilers even without the presence of an explicit sincos(...) function when -O2 or -O3 are used) */
        wtmp= sin_angle; wpr = (float)2.0*sin_angle;
        wpi = wpr*cos_angle;
        wpr*= -sin_angle;
        wr = (float)1.0; wi = (float)0.0;
        for (m=0; m < n2; ++m) {
            for (i=m; i < size_pot; i+=n) {
                j = i+n2;
                tc[0] = wr * out[j][0] - wi * out[j][1];
                tc[1] = wr * out[j][1] + wi * out[j][0];

                out[j][0]=out[i][0]-tc[0];out[i][0]+=tc[0];
                out[j][1]=out[i][1]-tc[1];out[i][1]+=tc[1];
            }
            wr=(wtmp=wr)*wpr-wi*wpi+wr;
            wi=wi*wpr+wtmp*wpi+wi;
        }
    }
    if (inverse_transform && !dont_scale_inverse_transform_like_fftw3_does) {
        const float scale = (float)1.0/(float)size_pot;
        for(i = 0;i < n /*[n == size_pot]*/;i++) {out[i][0]*=scale; out[i][1]*=scale;}
    }
}


SNDD_API_DEF size_t sndd_DecodeSerializedSound(float** psamples,size_t* sampleSizeInOut,const char* input,int samplerate,int num_channels)   {
    char* output=NULL;size_t outputSize = 0;unsigned i;
    size_t size = sndd_Base85Decode(input,&output,&outputSize);
    struct sndDFTserializerHeader_t* H = (struct sndDFTserializerHeader_t*) output;
    const int H_num_channels = (int) H->num_channels;   // MUST BE 1!
#   ifndef SNDD_MAX_LOG2SIZE
#       define SNDD_MAX_LOG2SIZE (17)
#   endif
#   ifdef SNDD_MAX_SIZE
#       error Please define SNDD_MAX_SIZE implicitly by defining SNDD_MAX_LOG2SIZE: SNDD_MAX_SIZE is (1<<SNDD_MAX_LOG2SIZE)
#   endif
#   define SNDD_MAX_SIZE (1<<SNDD_MAX_LOG2SIZE)
    size_t check_size = 0;
    unsigned required_size = 0,num_samples = 0;

//#   define SNDD_DECODE_SERIALIZED_SOUND_DOESNT_USE_BIG_STACK_ALLOCATION    // I've learned how to allocate something to match the (float) (*)[2] signature today! (plain float** does NOT work).
#   ifndef SNDD_DECODE_SERIALIZED_SOUND_DOESNT_USE_BIG_STACK_ALLOCATION
    float dft_output[SNDD_MAX_SIZE][2];  // on the stack ? Well, this should be: 2*128*4 KB = 1MB
#   else
    typedef float two_floats_t[2];  // I didn't know this...
    two_floats_t* dft_output = (two_floats_t*) SNDD_MALLOC(SNDD_MAX_SIZE*sizeof(two_floats_t));
    SNDD_ASSERT(sizeof(two_floats_t)==2*sizeof(float));
#   endif    
    
    SNDD_ASSERT(input && psamples && sampleSizeInOut);
    SNDD_ASSERT(samplerate>0 && (num_channels==1 || num_channels==2));
    SNDD_ASSERT(output && size);
    SNDD_ASSERT(H->sizeOfThisStruct == (unsigned)sizeof(*H));
#   ifndef SNDD_BIG_ENDIAN
    SNDD_ASSERT(strncmp((char*)H->tag,"D0L0",4)==0);    // Wrong file/version/endianess/compression
#   else
    SNDD_ASSERT(strncmp((char*)H->tag,"D0B0",4)==0);    // Wrong file/version/endianess/compression
#   endif
    SNDD_ASSERT(H->num_channels==1);                                // Wrong num_channels
    SNDD_ASSERT(H->log2size<=SNDD_MAX_LOG2SIZE);
    SNDD_ASSERT(H->num_bits_for_modulo_in_low_precision_data>=2 && H->num_bits_for_modulo_in_low_precision_data<=6);                 // wrong number of modulo bits in dft_modulo_plus_phase


    check_size = sndd_DeserializeDiscreteFourierTransform(dft_output,H->size_pot,(const unsigned char*) output,size);
    SNDD_ASSERT(check_size==(size_t)H->size_pot);


    // IDFT:
    sndd_DiscreteFourierTransform(dft_output,dft_output,H->size_pot,H->log2size,1,0);


    SNDD_ASSERT(samplerate>0);
    required_size = samplerate<=H->samplerate?H->num_samples:H->num_samples*(samplerate/H->samplerate);
    required_size*=(num_channels/H_num_channels);
    if (!(*psamples) || *sampleSizeInOut<(size_t)required_size) {
        *psamples = SNDD_REALLOC(*psamples,required_size*sizeof(float));
        *sampleSizeInOut = (size_t)required_size;
    }
    SNDD_ASSERT(required_size>=H->num_samples);
    num_samples = H->num_samples;
    for (i=0;i<num_samples;i++)    {(*psamples)[i] = dft_output[i][0];}
    
#   ifdef SNDD_DECODE_SERIALIZED_SOUND_DOESNT_USE_BIG_STACK_ALLOCATION
    SNDD_FREE(dft_output);dft_output=NULL;
#   endif    
    
    if (samplerate!=H->samplerate)  {
        float* samples = *psamples;
        while (samplerate>=H->samplerate*2)   {
            //SNDD_ASSERT(num_samples%2==0);   // otherwise we must change this code
            SNDD_ASSERT(num_samples*2<=required_size);
            //memset(&samples[num_samples],0,sizeof(num_samples));    // ?
            // we must add a smoothed sample each 2
            // 0 1 2 3 | 4 5 6 7
            // 0 x 1 x | 2 x 3 x    // OK ('num_samples' even)

            // 0 1 2 3 4 5 | 6 7 8 9 10 11
            // 0 x 1 x 2 x | 3 x 4 x 5  x   // OK ('num_samples' even)

            // 0 1 2 3 4 | 5 6 7 8 9
            // 0 X 1 X 2 | x 3 x 4 x    // ?  ('num_samples' odd)
            for (i=num_samples-1;i>0;i--)  {
                samples[2*i] = samples[i];
            }
            for (i=1;i<2*num_samples-1;i+=2)  {
                samples[i] = 0.5f*(samples[i-1]+samples[i+1]);
            }
            samples[2*num_samples-1]=2*num_samples-2;
            //--------------
            samplerate/=2;
            num_samples*=2;

        }
        while (samplerate<=H->samplerate/2) {
            // We must have one sample every two samples
            // 0 1 2 3 4
            //-------------
            int j = 0;
            for (i=0;i<num_samples/2;i++)  {
                samples[i] = 0.5f*(samples[j]+samples[j+1]);
                j+=2;
            }
            memset(&samples[num_samples/2],0,sizeof(num_samples/2));    // optional, but safer
            samplerate*=2;
            num_samples/=2;
        }
    }
    if (num_channels>H_num_channels)    {
        // We must clone the samples and interpolate them
        float* samples = NULL;
        SNDD_ASSERT(num_channels==2 && H_num_channels==1);
        //SNDD_ASSERT(num_samples%2==0);   // otherwise we must change this code
        SNDD_ASSERT(num_samples*2<=required_size);
        samples = *psamples;
        for (i=num_samples-1;i>0;i--)  samples[2*i] = samples[i];
        for (i=1;i<=2*num_samples-1;i+=2)  samples[i] = samples[i-1];
        num_samples*=2;
    }

    SNDD_FREE(output);output=NULL;outputSize=0;
    return num_samples;
}


#ifdef SNDD_HAS_WAV_SAVING_SUPPORT
#include <stdio.h>
SNDD_API_DEF size_t sndd_SaveAsWavFile(const char* savePath,const float* samples,size_t num_samples,int samplerate,int num_channels)    {
#   define SNDD_ENDIAN_SWAP_USHORT(X)  {unsigned short TMP=X;unsigned char *pT=(unsigned char*)&TMP,*pX=(unsigned char*)&X;pX[0]=pT[1];pX[1]=pT[0];}
#   define SNDD_ENDIAN_SWAP_UINT(X)  {unsigned short TMP=X;unsigned char *pT=(unsigned char*)&TMP,*pX=(unsigned char*)&X;pX[0]=pT[3];pX[1]=pT[2];pX[2]=pT[1];pX[3]=pT[0];}
    struct {
        // Wav Header struct info based on: http://truelogic.org/wordpress/2015/09/04/parsing-a-wav-file-in-c/

        // riff chunk [12 bytes]
        unsigned char riff[4];              // 'RIFF' string
        unsigned int overall_size;          // overall size of file in bytes, starting from next field (it's always H.data_size+36 in the default case)
        unsigned char wave[4];              // 'WAVE' string

        // fmt chunk [24 bytes in the default case]
        unsigned char fmt_chunk_marker[4];  // 'fmt ' string
        unsigned int length_of_fmt;         // length of the format data, starting from next field (16 in the default case, otherwise 18 or 40)
        unsigned short format_type;         // format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
        unsigned short channels;            // no.of channels
        unsigned int sample_rate;           // sampling rate (blocks per second)
        unsigned int byterate;              // SampleRate * NumChannels * BitsPerSample/8
        unsigned short block_align;         // NumChannels * BitsPerSample/8
        unsigned short bits_per_sample;     // bits per sample, 8- 8bits, 16- 16 bits etc

        // data chunk [8 bytes]
        unsigned char data_chunk_header[4]; // 'data' string (or FLLR string)
        unsigned int data_size;             // NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk that will be read

        // Note: all the unsigned int and unsigned short fields must be saved in little-endian.
        // I've written two endian-swap macros above, but I don't need to use them on my system.
    } H = SNDD_ZERO_INIT;
    FILE* f = fopen(savePath,"wb");
    size_t i;
    SNDD_ASSERT(f);
    if (!f) return 0;
    SNDD_ASSERT(sizeof(H)==44);
    if (f)  {
        const int num_samples_per_channel = num_samples/num_channels;
        const char* riff = "RIFF";const char* wave = "WAVE";const char* format = "fmt ";const char* data = "data";
        for (i=0;i<4;i++)   {H.riff[i]=riff[i];H.wave[i]=wave[i];H.fmt_chunk_marker[i]=format[i];H.data_chunk_header[i]=data[i];}
        H.format_type = 1;H.channels = (unsigned short) num_channels;
        H.sample_rate = samplerate;H.bits_per_sample = 16;
        H.byterate = H.sample_rate * H.channels * H.bits_per_sample/8;
        H.block_align = H.channels * H.bits_per_sample/8;
        H.data_size = num_samples_per_channel * H.channels *  H.bits_per_sample/8;
        H.overall_size = (unsigned)sizeof(H)-8+H.data_size;
        H.length_of_fmt = 16;

        fwrite(&H,sizeof(H),1,f);

        for (i=0;i<num_samples;i++) {
            // unsigned short range is from -32768 to 32767.
            float vf = samples[i]; short vs;
            vf = vf>=0.f ? (vf*32767.f) : (vf*32768.f);
            if (vf>32767.f) vf = 32767.f;
            else if (vf<-32768.f) vf = -32768.f;
            vs = (short) vf;
            fwrite(&vs,sizeof(short),1,f);
        }

        fclose(f);
    }
    return H.overall_size+8;    // total size of the wav file in bytes
}
#endif //SNDD_HAS_WAV_SAVING_SUPPORT

#endif //SNDDECODER_IMPLEMENTATION_GUARD
#endif //SNDDECODER_IMPLEMENTATION


