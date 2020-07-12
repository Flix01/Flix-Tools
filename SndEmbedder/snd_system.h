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

#ifndef SND_SYSTEM_H_
#define SND_SYSTEM_H_

#ifndef SND_DEBUG
#   if (!defined(NDEBUG) && !defined(_NDEBUG)) || defined(DEBUG) || defined (_DEBUG)
#       define SND_DEBUG
#   endif
#endif

#if (!defined(SND_ASSERT) && defined(SND_DEBUG))
#include <assert.h>
#define SND_ASSERT(X)   assert(X)
#else
#define SND_ASSERT(X)   /*no op*/
#endif

#ifndef __cplusplus
#   define SND_ZERO_INIT   {0}
#else
#   define SND_ZERO_INIT   {}
#endif


// we don't include <math.h> here
#ifndef SND_USE_DOUBLE_PRECISION
typedef float snoat;
#else
typedef double snoat;
#endif

#if (!defined(SND_USE_DOUBLE_PRECISION) && !defined(SND_NO_C99_MATH_FUNCTIONS))
#   define snd_round(v)    ((v) < 0.0f ? ceilf((v) - 0.5f) : floorf((v) + 0.5f))
#   define snd_fabs(X)  fabsf(X)
#   define snd_sin(X)   sinf(X)
#   define snd_cos(X)   cosf(X)
#   define snd_log10(X) log10f(X)
#   define snd_exp(X)   expf(X)
#   define snd_floor(X) floorf(X)
#   define snd_sqrt(X) 	sqrtf(X)
#   define snd_asin(X) 	asinf(X)
#   define snd_acos(X) 	acosf(X)
#   define snd_atan2(X,Y) 	atan2f(X,Y)
#   if (defined(GNU_SOURCE) || defined(SND_HAS_SINCOS))
#       define snd_sincos(ANGLE,PSIN,PCOS)  {sincosf(ANGLE,PSIN,PCOS);} // old compilers
#   else
#       define snd_sincos(ANGLE,PSIN,PCOS)  {*PSIN=sinf(ANGLE);*PCOS=cosf(ANGLE);}  // new compilers can optimize this in -O2 or -O3
#   endif
#else
#   define snd_round(v)    ((v) < 0.0 ? ceil((v) - 0.5) : floor((v) + 0.5))
#   define snd_fabs(X)  fabs(X)
#   define snd_sin(X)   sin(X)
#   define snd_cos(X)   cos(X)
#   define snd_log10(X) log10(X)
#   define snd_exp(X)   exp(X)
#   define snd_floor(X) floor(X)
#   define snd_sqrt(X) 	sqrt(X)
#   define snd_asin(X) 	asin(X)
#   define snd_acos(X) 	acos(X)
#   define snd_atan2(X,Y) 	atan2(X,Y)
#   if (defined(GNU_SOURCE) || defined(SND_HAS_SINCOS))
#       define snd_sincos(ANGLE,PSIN,PCOS)  {sincos(ANGLE,PSIN,PCOS);} // old compilers
#   else
#       define snd_sincos(ANGLE,PSIN,PCOS)  {*PSIN=sin(ANGLE);*PCOS=cos(ANGLE);}  // new compilers can optimize this in -O2 or -O3
#   endif
#endif



#define SND_SAMPLERATE (22050)                 // max audiable frequency range we use
#define SND_FRAMESIZE (512)                    // float32 mono audio sampled at 512*1000/22050Hz => 23,219955 ms are 512 samples

#define SND_FORMAT      AL_FORMAT_MONO_FLOAT32 // this cannot be changed!


// INIT API
extern struct sndcontext_t* snd_context_create(void);
extern void snd_context_destroy(struct sndcontext_t* p);


// CAPTURE API
// returns 1 if successfull.
extern int sndcontext_setOpenCaptureDevice(struct sndcontext_t* p,int new_capture_device_index);
extern const char* sndcontext_getOpenCaptureDeviceName(struct sndcontext_t* p,int* pOptionalOpenCaptureDeviceIndexOut);
// if 'capture_device_name_index==0' it returns the first available device name
// that still contains all of names (separated by '\0' end ended by '\0\0')
extern const char* sndcontext_getCaptureDeviceNames(struct sndcontext_t* p,int capture_device_name_index /*=0*/,int* pOptionalCaptureDeviceNamesCountOut);
// must be called continuously quite often. Returns the number of new mono samples
// appended at the end of 'monoBuffer' (return_value is >=0, but can be >monoBufferSize
// if we lose 'return_value-monoBufferSize' captured samples)
extern int sndcontext_updateCaptureBuffer(struct sndcontext_t* p,snoat* monoBuffer,int monoBufferSize);

// Using the functions above should be enough, but if we need to manually stop/restart capture, we can use:
extern void sndcontext_stopOpenCaptureDevice(struct sndcontext_t* p);
extern int sndcontext_startOpenCaptureDevice(struct sndcontext_t* p);   // returns 1 if is capturing audio
extern int sndcontext_isOpenCaptureDeviceStarted(struct sndcontext_t* p);   // returns 1 if is capturing audio


// PLAYBACK API
// returns 1 if successfull.
extern int sndcontext_setOpenPlaybackDevice(struct sndcontext_t* p,int new_playback_device_index);
extern const char* sndcontext_getOpenPlaybackDeviceName(struct sndcontext_t* p,int* pOptionalOpenPlaybackDeviceIndexOut);
// if 'playback_device_name_index==0' it returns the first available device name
// that still contains all of names (separated by '\0' end ended by '\0\0')
extern const char* sndcontext_getPlaybackDeviceNames(struct sndcontext_t* p,int playback_device_name_index /*=0*/,int* pOptionalPlaybackDeviceNamesCountOut);

// Using the functions above should be enough, but if we need to manually stop/restart playback, we can use:
extern void sndcontext_stopOpenPlaybackDevice(struct sndcontext_t* p);
extern int sndcontext_startOpenPlaybackDevice(struct sndcontext_t* p);   // returns 1 if is capturing audio
extern int sndcontext_isOpenPlaybackDeviceStarted(struct sndcontext_t* p);   // returns 1 if is capturing audio

extern void sndcontext_feedPlaybackData(struct sndcontext_t* p,const void* pData,int numSamplesInBytes);
extern void sndcontext_playbackPlay(struct sndcontext_t* p);
extern void sndcontext_playbackPause(struct sndcontext_t* p);
extern void sndcontext_playbackStop(struct sndcontext_t *p);
typedef enum {
    SND_PLAYBACK_INITIAL = 0x1011,
    SND_PLAYBACK_PLAYING = 0x1012,
    SND_PLAYBACK_PAUSED =  0x1013,
    SND_PLAYBACK_STOPPED = 0x1014
} snd_playback_state;
extern void sndcontext_setPlaybackState(struct sndcontext_t* p,snd_playback_state state);   //alSourcePlay(source);
extern snd_playback_state sndcontext_getPlaybackState(struct sndcontext_t* p);   //alSourcePlay(source);
extern void sndcontext_setPlaybackPosition(struct sndcontext_t* p,int pos_in_num_samples);
extern int sndcontext_getPlaybackPosition(struct sndcontext_t* p);    // in num_samples



// HELPERS
// 1-dimensional DFT (and its inverse) when 'size_pot' is a power of two, and 2^('log2size')='size_pot'.
// 'inverse_transform' and 'dont_scale_inverse_transform_like_fftw3_does' are boolean values.
void snd_DiscreteFourierTransform(snoat out[][2],const snoat in[][2],int size_pot,int log2size,int inverse_transform,int dont_scale_inverse_transform_like_fftw3_does);
unsigned snd_calculate_next_power_of_two( unsigned x );
#ifndef SND_NO_STDIO
const char* snd_GetFileSizeString(size_t size_in_bytes); // e.g 1024 -> "1 Kb"
#endif

typedef struct sndDFTCompressionParams_t {
    // These values should be used from top to bottom, because they're applied in cascade one after the other (each param depends on the one above it)
    // Usually the first ones give the bigger compression
    snoat high_frequency_trimmer;       // = 0.05   in [0,1]                                            bigger: less quality <-> more compression
    snoat regular_frequency_trimmer;    // = 0.025  in [0,1], usually in [0,high_frequency_trimmer]     bigger: less quality <-> more compression
    snoat quantization_enhancer;        // = 5.0     in [0,...]                                         bigger: more quality <-> less compression... but in some cases more
    snoat quantization_worsening;       // = 0.2       in [0,1]                                         bigger: less quality <-> more compression
    unsigned char num_high_precision_bits_for_modulos;  // = 8      in [2,14]
    unsigned char num_low_precision_bits_for_modulos;   // = 4      in [2,6]
} sndDFTCompressionParams_t;
#define SNDDFTCOMPRESSIONPARAMS_DEFAULT_INIT   {0.035,0.025,7.5,0.5,8,4}
unsigned char *snd_SerializeDiscreteFourierTransform(size_t *bufferOutSize, int num_samples_input_signal, const snoat dft[][2], const sndDFTCompressionParams_t* params /*=NULL*/);
size_t snd_DeserializeDiscreteFourierTransform(snoat dft_out[][2], size_t dft_out_size, const unsigned char* bufferIn, size_t bufferInSize);

// In these two functions: 'output' if present must be allocated using SND_MALLOC(...)
// If 'output' is NULL it will be allocated for you. In any case user must free it using SND_FREE(...)
// Returns the number of used bytes in 'output'
size_t snd_Base85Encode(const char* input, size_t inputSize, char** poutput, size_t* poutputSizeInOut, int stringifiedMode/*=0*/, int numCharsPerLineInStringifiedMode/*=112*/, int noBackslashesInStringifiedMode /*=0*/);
size_t snd_Base85Decode(const char* input, char** poutput, size_t* poutputSizeInOut);

#ifndef SND_NO_STDIO
size_t snd_SaveAsWavFile(const char* savePath,const float* samples,size_t num_samples,int samplerate,int num_channels);
#endif //SND_NO_STDIO

#ifndef SND_MALLOC
#   include <stdlib.h>
#   define SND_MALLOC(X)  malloc(X)
#endif
#ifndef SND_REALLOC
#   include <stdlib.h>
#   define SND_REALLOC(X,Y)  realloc(X,Y)
#endif
#ifndef SND_FREE
#   include <stdlib.h>
#   define SND_FREE(X)  free(X)
#endif

#endif //SND_SYSTEM_H_








#ifdef SND_SYSTEM_IMPLEMENTATION
#ifndef SND_SYSTEM_IMPLEMENTATION_H
#define SND_SYSTEM_IMPLEMENTATION_H

// openal headers
//#define SND_ALEXT_H_IS_PRESENT    // optional (well, if you have 'AL/alext.h', define it!)
#ifdef SND_ALEXT_H_IS_PRESENT
#   include <AL/alext.h>
#else //SND_ALEXT_H_IS_PRESENT
#   ifndef AL_EXT_float32      // we assume AL_EXT_float32 is implemented
#      define AL_EXT_float32 1
#      define AL_FORMAT_MONO_FLOAT32                   0x10010
#      define AL_FORMAT_STEREO_FLOAT32                 0x10011
#   endif //SND_AL_EXT_float32
#endif //__EMSCRIPTEN__ */
#include <AL/al.h>
#include <AL/alc.h>
#include <string.h> // memset

#include <math.h>
#ifndef M_PI
#   define M_PI		3.14159265358979323846
#endif

#ifdef SND_DEBUG
#include <stdio.h>
static void snd_checkError() {
    ALenum error = alGetError();
    if (error!=AL_NO_ERROR) fprintf(stderr,"AL_ERROR: %s\n",alGetString(error));
    SND_ASSERT(error==AL_NO_ERROR);
}
static void sndc_checkError(ALCdevice* device) {
    ALCenum error = alcGetError(device);
    if (error!=ALC_NO_ERROR) fprintf(stderr,"ALC_ERROR: %s\n",alcGetString(device,error));
    SND_ASSERT(error==ALC_NO_ERROR);
}
#define SND_AL_CHECKERROR    snd_checkError()
#define SND_ALC_CHECKERROR(device)    sndc_checkError(device)
#else
#define SND_AL_CHECKERROR    /*no-op*/
#define SND_ALC_CHECKERROR(device)    /*no-op*/
#endif

#define SND_BUFFERSIZE (SND_FRAMESIZE*2)      // we need space for at least two frames [this can be changed! but must be>=SND_FRAMESIZE*2]
//#define SND_NYQUISTSIZE (SND_FRAMESIZE/2)    // this cannot be changed! fny = fs/2


struct sndcontext_t {

    ALCdevice *mCaptureDevice;
    const char* mCaptureDeviceNames[200];int mCaptureDeviceNamesCount;   // but 'mCaptureDeviceNames[0]' still contains all of them (separated by '\0' end ended by '\0\0')
    const char* mOpenCaptureDeviceName;int mOpenCaptureDeviceIndex;
    int mOpenCaptureDeviceStarted;
/*
    ALfloat mBuffer[SND_BUFFERSIZE]; // audio buffer with 512+512 samples (float32 mono audio sampled at 22050Hz => 23,219955 ms are 512 samples).
    ALsizei mBufferFrameStart;      // in [0,SND_BUFFERSIZE/2). It's the beginning of the 'frame' inside the 'buffer'
    ALsizei mBufferWritePointer;    // mest be >=mBufferFrameStart
*/

    ALCdevice *mPlaybackDevice;
    ALCcontext * mPlaybackContext;
    const char* mPlaybackDeviceNames[200];int mPlaybackDeviceNamesCount;   // but 'mPlaybackDeviceNames[0]' still contains all of them (separated by '\0' end ended by '\0\0')
    const char* mOpenPlaybackDeviceName;int mOpenPlaybackDeviceIndex;
    int mOpenPlaybackDeviceStarted;

    ALuint mPlaybackSource,mPlaybackBuffer;
    ALCenum SND_ALC_DEVICE_SPECIFIER;

};


int sndcontext_startOpenCaptureDevice(struct sndcontext_t* p) {
    SND_ASSERT(p);
    if (p->mCaptureDevice)  {
        alcCaptureStart(p->mCaptureDevice);
        p->mOpenCaptureDeviceStarted = 1;
    }
    return p->mOpenCaptureDeviceStarted;
}
void sndcontext_stopOpenCaptureDevice(struct sndcontext_t* p) {
    SND_ASSERT(p);
    alcCaptureStop(p->mCaptureDevice);
    p->mOpenCaptureDeviceStarted = 0;
}
int sndcontext_isOpenCaptureDeviceStarted(struct sndcontext_t* p)   {
    SND_ASSERT(p);
    return p->mOpenCaptureDeviceStarted;
}

int sndcontext_updateCaptureBuffer(struct sndcontext_t* p,snoat* monoBuffer,int monoBufferSize) {
    /*  Here we simply empty all the pending capture buffer into monoBuffer
        Return value should always be positive or null, but can be > monoBufferSize when this
        function should be called more often (we lose a part of the capture buffer, and the
        lost part can't be recovered).
        [Usually these kind of functions take a callback that can be called multiple times
        per 'sndcontext_updateCaptureBuffer(...)' call so that we don't lose anything:
        we don't need that]
    */
    int count_total = 0;
    SND_ASSERT(p && monoBuffer && monoBufferSize>0);
    if (p->mCaptureDevice) {
        ALCenum err = alcGetError(p->mCaptureDevice);
        if (err == ALC_NO_ERROR)    {
            ALCint count = 0;
            alcGetIntegerv(p->mCaptureDevice, ALC_CAPTURE_SAMPLES, 1, &count_total);
            count = count_total;
            while (count>=monoBufferSize) {
                /* Note that here we consume (waste) a part of the 'count_total' captured data */
                alcCaptureSamples(p->mCaptureDevice, monoBuffer, monoBufferSize);
                count-=monoBufferSize;
            }
            if (count>0)  {
                SND_ASSERT(count<monoBufferSize);
                /* shift left last 'monoBufferSize-count' samples */
                memmove(&monoBuffer[0],&monoBuffer[count],(monoBufferSize-count)*sizeof(snoat));
                /* append last 'count' samples at the end of 'monoBuffer' */
                alcCaptureSamples(p->mCaptureDevice, &monoBuffer[monoBufferSize-count], count);
                count = 0;
            }
        }
    }
    return count_total;
}

int sndcontext_startOpenPlaybackDevice(struct sndcontext_t* p) {
    SND_ASSERT(p);
    p->mOpenPlaybackDeviceStarted = 0;
    if (p->mPlaybackContext)  {
        SND_ASSERT(p->mPlaybackDevice);
        SND_ASSERT(p->mPlaybackSource);
        SND_ASSERT(p->mPlaybackBuffer);
        alcMakeContextCurrent(p->mPlaybackContext);
        p->mOpenPlaybackDeviceStarted = 1;
    }
    //SND_ASSERT(p->mOpenPlaybackDeviceStarted == 1);
    return p->mOpenPlaybackDeviceStarted;
}
void sndcontext_stopOpenPlaybackDevice(struct sndcontext_t* p) {
    SND_ASSERT(p);
    if (p->mPlaybackContext)  {
        SND_ASSERT(p->mPlaybackDevice);
        SND_ASSERT(p->mPlaybackSource);
        SND_ASSERT(p->mPlaybackBuffer);
        alcMakeContextCurrent(NULL);
    }
    p->mOpenPlaybackDeviceStarted = 0;
}
int sndcontext_isOpenPlaybackDeviceStarted(struct sndcontext_t* p)   {
    SND_ASSERT(p);
    return p->mOpenPlaybackDeviceStarted;
}
void sndcontext_feedPlaybackData(struct sndcontext_t* p,const void* pData,int numSamplesInBytes)   {
    SND_ASSERT(p);
    SND_ASSERT(p->mOpenPlaybackDeviceStarted);
    SND_ASSERT(p->mPlaybackContext);
    SND_ASSERT(p->mPlaybackDevice);
    SND_ASSERT(p->mPlaybackSource);
    SND_ASSERT(p->mPlaybackBuffer);
    SND_ASSERT(AL_EXT_float32);
    SND_ASSERT(alIsExtensionPresent("AL_EXT_float32"));
    SND_AL_CHECKERROR;
    SND_ALC_CHECKERROR(p->mPlaybackDevice);
    alSourceStop(p->mPlaybackSource);
    alSourcei(p->mPlaybackSource, AL_BUFFER, 0);    // The NULL Buffer is extremely useful for detaching buffers from a source which were attached using this call or with alSourceQueueBuffers.
    alBufferData(p->mPlaybackBuffer,SND_FORMAT/*AL_MONO32F_SOFT*/,(ALvoid*)pData,(ALsizei) numSamplesInBytes,(ALsizei)SND_SAMPLERATE);
    SND_AL_CHECKERROR; // AL_INVALID_OPERATION error occurs when trying to fill buffer with data, when the buffer is still queued or assigned to the source.
    alSourcei(p->mPlaybackSource, AL_BUFFER, p->mPlaybackBuffer);
    SND_AL_CHECKERROR;
}
void sndcontext_setPlaybackState(struct sndcontext_t* p,snd_playback_state state)  {
    SND_ASSERT(p);
    if (p->mOpenPlaybackDeviceStarted)  {
        SND_ASSERT(p->mPlaybackContext);
        SND_ASSERT(p->mPlaybackDevice);
        SND_ASSERT(p->mPlaybackSource);
        SND_ASSERT(p->mPlaybackBuffer);
        switch (state)  {
            case SND_PLAYBACK_PLAYING: alSourcePlay(p->mPlaybackSource);break;
            case SND_PLAYBACK_PAUSED: alSourcePause(p->mPlaybackSource);break;
            case SND_PLAYBACK_STOPPED: alSourceStop(p->mPlaybackSource);break;
            case SND_PLAYBACK_INITIAL: alSourceRewind(p->mPlaybackSource);break;
        default:break;
        }
    }
    SND_AL_CHECKERROR;
}
snd_playback_state sndcontext_getPlaybackState(struct sndcontext_t* p) {
    snd_playback_state rv = SND_PLAYBACK_STOPPED;
    SND_ASSERT(p);
    if (p->mOpenPlaybackDeviceStarted)  {
        SND_ASSERT(p->mPlaybackContext);
        SND_ASSERT(p->mPlaybackDevice);
        SND_ASSERT(p->mPlaybackSource);
        SND_ASSERT(p->mPlaybackBuffer);
        alGetSourcei(p->mPlaybackSource, AL_SOURCE_STATE,(ALint*) &rv);
    }
    return rv;
}
void sndcontext_playbackPlay(struct sndcontext_t* p)    {sndcontext_setPlaybackState(p,SND_PLAYBACK_PLAYING);}
void sndcontext_playbackPause(struct sndcontext_t* p)    {sndcontext_setPlaybackState(p,SND_PLAYBACK_PAUSED);}
void sndcontext_playbackStop(struct sndcontext_t* p)    {sndcontext_setPlaybackState(p,SND_PLAYBACK_STOPPED);}
void sndcontext_setPlaybackPosition(struct sndcontext_t* p,int pos_in_num_samples)  {
    SND_ASSERT(p);
    if (p->mOpenPlaybackDeviceStarted)  {
        alSourcei(p->mPlaybackSource, AL_SAMPLE_OFFSET,pos_in_num_samples);
    }
}
int sndcontext_getPlaybackPosition(struct sndcontext_t* p)  {
    int rv = 0;
    SND_ASSERT(p);
    if (p->mOpenPlaybackDeviceStarted)  {
        alGetSourcei(p->mPlaybackSource, AL_SAMPLE_OFFSET,(ALint*) &rv);
    }
    return rv;
}


struct sndcontext_t* sndcontext_create()   {
    struct sndcontext_t* p = (struct sndcontext_t*) SND_MALLOC(sizeof(struct sndcontext_t));
    ALCboolean isEnumerationExtensionPresent = ALC_FALSE;
    ALCboolean isEnumerateAllExtensionPresent = ALC_FALSE;
    SND_ASSERT(sizeof(ALCfloat)==sizeof(float) && sizeof(int)==sizeof(ALCint));
    SND_ASSERT(p);
    memset(p,0,sizeof(*p));
    alGetError(); // clear error code
    isEnumerationExtensionPresent=alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
    isEnumerateAllExtensionPresent=alcIsExtensionPresent(NULL, "ALC_ENUMERATE_ALL_EXT");
    if (isEnumerationExtensionPresent)	{
        size_t len;const ALCchar* n;
        p->mCaptureDeviceNamesCount=0;
        n = p->mCaptureDeviceNames[p->mCaptureDeviceNamesCount] = alcGetString(NULL,ALC_CAPTURE_DEVICE_SPECIFIER);
        while ((len=strlen(n)))	{
            if (p->mCaptureDeviceNamesCount>=199) break;
            n+=len+1;p->mCaptureDeviceNames[++p->mCaptureDeviceNamesCount]=n;
        }
    }
    // Here we open (and start) the default capture device
    p->mCaptureDevice = alcCaptureOpenDevice(NULL, SND_SAMPLERATE, SND_FORMAT, SND_BUFFERSIZE);	// AL_FORMAT_MONO16,AL_FORMAT_STEREO16 or FLOAT
    if (p->mCaptureDevice)	{
        SND_ALC_CHECKERROR(p->mCaptureDevice);
        p->mOpenCaptureDeviceName = alcGetString(p->mCaptureDevice, ALC_CAPTURE_DEVICE_SPECIFIER);
        sndcontext_startOpenCaptureDevice(p);
    }

    if (isEnumerationExtensionPresent || isEnumerateAllExtensionPresent)	{
        size_t len;const ALCchar* n;
        p->SND_ALC_DEVICE_SPECIFIER = isEnumerateAllExtensionPresent ? ALC_ALL_DEVICES_SPECIFIER : ALC_DEVICE_SPECIFIER;
        p->mPlaybackDeviceNamesCount=0;
        n = p->mPlaybackDeviceNames[p->mPlaybackDeviceNamesCount] = alcGetString(NULL,p->SND_ALC_DEVICE_SPECIFIER);
        while ((len=strlen(n)))	{
            if (p->mPlaybackDeviceNamesCount>=199) break;
            n+=len+1;p->mPlaybackDeviceNames[++p->mPlaybackDeviceNamesCount]=n;
        }
    }
    // Here we open (but don't start) the default playback device
    p->mPlaybackDevice = alcOpenDevice(NULL);
    if (p->mPlaybackDevice)	{
        SND_ALC_CHECKERROR(p->mPlaybackDevice);
        p->mOpenPlaybackDeviceName = alcGetString(p->mPlaybackDevice, p->SND_ALC_DEVICE_SPECIFIER);

        p->mPlaybackContext = alcCreateContext(p->mPlaybackDevice, NULL);
        if (p->mPlaybackContext)    {
            ALboolean isFloatExtensionPresent = AL_FALSE;//alIsExtensionPresent("AL_EXT_float32");
            alcMakeContextCurrent(p->mPlaybackContext);
            alGenBuffers(1,&p->mPlaybackBuffer);
            alGenSources(1,&p->mPlaybackSource);
            if (p->mPlaybackSource && p->mPlaybackBuffer) {
                alSourcei(p->mPlaybackSource, AL_SOURCE_RELATIVE, AL_TRUE);
                alSource3f(p->mPlaybackSource, AL_POSITION, 0.0f, 0.0f, 0.0f);
                alSource3f(p->mPlaybackSource, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
            }
            SND_AL_CHECKERROR;
            isFloatExtensionPresent = alIsExtensionPresent("AL_EXT_float32");
            /*if (isFloatExtensionPresent==AL_FALSE)
            {
                fprintf(stdout,"AL_VERSION: %s\n",alGetString(AL_VERSION));
                fprintf(stdout,"AL_EXTENSIONS: %s\n",alGetString(AL_EXTENSIONS));
                fprintf(stdout,"ALC_EXTENSIONS: %s\n",alcGetString(p->mCaptureDevice,ALC_EXTENSIONS));
            }*/
            SND_ASSERT(isFloatExtensionPresent==AL_TRUE);

            alcMakeContextCurrent(NULL);
        }
    }
    SND_ASSERT(p->mPlaybackBuffer);
    return p;
}
void sndcontext_destroy(struct sndcontext_t* p)    {
    SND_ASSERT(p);
    if (p->mPlaybackContext)    {
        alcMakeContextCurrent(p->mPlaybackContext);
        if (p->mPlaybackBuffer) {alDeleteBuffers(1,&p->mPlaybackBuffer);p->mPlaybackBuffer=0;}
        if (p->mPlaybackSource) {alDeleteSources(1,&p->mPlaybackSource);p->mPlaybackSource=0;}
        alcMakeContextCurrent(NULL);
        alcDestroyContext(p->mPlaybackContext);p->mPlaybackContext=NULL;
        p->mOpenPlaybackDeviceStarted = 0;
    }
    if (p->mPlaybackDevice) {alcCloseDevice(p->mPlaybackDevice);p->mPlaybackDevice=NULL;}

    if (p->mCaptureDevice)	{sndcontext_stopOpenCaptureDevice(p);alcCaptureCloseDevice(p->mCaptureDevice);p->mCaptureDevice=NULL;}
    memset(p,0,sizeof(*p));
    SND_FREE(p);p=NULL;
}
const char* sndcontext_getOpenCaptureDeviceName(struct sndcontext_t* p,int* pOptionalOpenCaptureDeviceIndexOut)   {
    SND_ASSERT(p);
    if (pOptionalOpenCaptureDeviceIndexOut)    {*pOptionalOpenCaptureDeviceIndexOut=p->mOpenCaptureDeviceIndex;}
    return p->mOpenCaptureDeviceName;
}
const char* sndcontext_getCaptureDeviceNames(struct sndcontext_t* p,int capture_device_name_index /*=0*/,int* pOptionalCaptureDeviceNamesCountOut)  {
    // if 'device_name_index==0' it returns the first available device name
    // that still contains all of names (separated by '\0' end ended by '\0\0')
    SND_ASSERT(p);
    if (pOptionalCaptureDeviceNamesCountOut) *pOptionalCaptureDeviceNamesCountOut=p->mCaptureDeviceNamesCount;
    if (p->mCaptureDeviceNamesCount<=0) return NULL;
    if (capture_device_name_index<0) capture_device_name_index=0;
    else if (capture_device_name_index>=p->mCaptureDeviceNamesCount) capture_device_name_index=p->mCaptureDeviceNamesCount-1;
    return p->mCaptureDeviceNames[capture_device_name_index];
}
int sndcontext_setOpenCaptureDevice(struct sndcontext_t* p, int new_capture_device_index) {
    int mustStartCaptureDevice = 0;
    SND_ASSERT(p);
    mustStartCaptureDevice = p->mOpenCaptureDeviceStarted;
    if (new_capture_device_index<0 || new_capture_device_index>=p->mCaptureDeviceNamesCount || !p->mCaptureDevice) return 0;
    if (new_capture_device_index!=p->mOpenCaptureDeviceIndex)  {
        if (p->mCaptureDevice)	{sndcontext_stopOpenCaptureDevice(p);alcCaptureCloseDevice(p->mCaptureDevice);p->mCaptureDevice=NULL;}
        p->mCaptureDevice = alcCaptureOpenDevice(p->mCaptureDeviceNames[new_capture_device_index], SND_SAMPLERATE, SND_FORMAT, SND_BUFFERSIZE);
        if (p->mCaptureDevice)	{
            p->mOpenCaptureDeviceName = alcGetString(p->mCaptureDevice, ALC_CAPTURE_DEVICE_SPECIFIER);
            p->mOpenCaptureDeviceIndex = new_capture_device_index;
            if (mustStartCaptureDevice) sndcontext_startOpenCaptureDevice(p);
        }
        else {
            p->mCaptureDevice = alcCaptureOpenDevice(NULL, SND_SAMPLERATE, SND_FORMAT, SND_BUFFERSIZE);
            if (p->mCaptureDevice)	{
                p->mOpenCaptureDeviceName = alcGetString(p->mCaptureDevice, ALC_CAPTURE_DEVICE_SPECIFIER);
                p->mOpenCaptureDeviceIndex = 0;
                if (mustStartCaptureDevice) sndcontext_startOpenCaptureDevice(p);
            }
            else {p->mOpenCaptureDeviceName = NULL;p->mOpenCaptureDeviceIndex = 0;p->mOpenCaptureDeviceStarted = 0;}
        }
    }
    return 1;
}

const char* sndcontext_getOpenPlaybackDeviceName(struct sndcontext_t* p,int* pOptionalOpenPlaybackDeviceIndexOut)   {
    SND_ASSERT(p);
    if (pOptionalOpenPlaybackDeviceIndexOut)    {*pOptionalOpenPlaybackDeviceIndexOut=p->mOpenPlaybackDeviceIndex;}
    return p->mOpenPlaybackDeviceName;
}
const char* sndcontext_getPlaybackDeviceNames(struct sndcontext_t* p,int playback_device_name_index /*=0*/,int* pOptionalPlaybackDeviceNamesCountOut)  {
    // if 'device_name_index==0' it returns the first available device name
    // that still contains all of names (separated by '\0' end ended by '\0\0')
    SND_ASSERT(p);
    if (pOptionalPlaybackDeviceNamesCountOut) *pOptionalPlaybackDeviceNamesCountOut=p->mPlaybackDeviceNamesCount;
    if (p->mPlaybackDeviceNamesCount<=0) return NULL;
    if (playback_device_name_index<0) playback_device_name_index=0;
    else if (playback_device_name_index>=p->mPlaybackDeviceNamesCount) playback_device_name_index=p->mPlaybackDeviceNamesCount-1;
    return p->mPlaybackDeviceNames[playback_device_name_index];
}
int sndcontext_setOpenPlaybackDevice(struct sndcontext_t* p, int new_playback_device_index) {
    int mustStartPlaybackDevice = 0;
    SND_ASSERT(p);
    mustStartPlaybackDevice = p->mOpenPlaybackDeviceStarted;
    if (new_playback_device_index<0 || new_playback_device_index>=p->mPlaybackDeviceNamesCount || !p->mPlaybackDevice) return 0;
    if (new_playback_device_index!=p->mOpenPlaybackDeviceIndex)  {
        if (p->mPlaybackContext)    {
            alcMakeContextCurrent(p->mPlaybackContext);
            if (p->mPlaybackBuffer) {alDeleteBuffers(1,&p->mPlaybackBuffer);p->mPlaybackBuffer=0;}
            if (p->mPlaybackSource) {alDeleteSources(1,&p->mPlaybackSource);p->mPlaybackSource=0;}
            alcMakeContextCurrent(NULL);
            alcDestroyContext(p->mPlaybackContext);p->mPlaybackContext=NULL;
            p->mOpenPlaybackDeviceStarted = 0;
        }
        if (p->mPlaybackDevice) {alcCloseDevice(p->mPlaybackDevice);p->mPlaybackDevice=NULL;}
        p->mPlaybackDevice = alcOpenDevice(p->mPlaybackDeviceNames[new_playback_device_index]);
        if (p->mPlaybackDevice)	{
            SND_ALC_CHECKERROR(p->mPlaybackDevice);
            p->mOpenPlaybackDeviceName = alcGetString(p->mPlaybackDevice, p->SND_ALC_DEVICE_SPECIFIER);
            p->mOpenPlaybackDeviceIndex = new_playback_device_index;
            p->mPlaybackContext = alcCreateContext(p->mPlaybackDevice, NULL);
            if (p->mPlaybackContext)    {
                alcMakeContextCurrent(p->mPlaybackContext);
                alGenBuffers(1,&p->mPlaybackBuffer);
                alGenSources(1,&p->mPlaybackSource);
                if (p->mPlaybackSource && p->mPlaybackBuffer) {
                    alSourcei(p->mPlaybackSource, AL_SOURCE_RELATIVE, AL_TRUE);
                    alSource3f(p->mPlaybackSource, AL_POSITION, 0.0f, 0.0f, 0.0f);
                    alSource3f(p->mPlaybackSource, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
                }
                SND_AL_CHECKERROR;
                if (mustStartPlaybackDevice) p->mOpenPlaybackDeviceStarted = 1;
                else alcMakeContextCurrent(NULL);
            }
        }
        else {
            p->mPlaybackDevice = alcOpenDevice(NULL);
            if (p->mPlaybackDevice)	{
                p->mOpenPlaybackDeviceName = alcGetString(p->mPlaybackDevice, p->SND_ALC_DEVICE_SPECIFIER);
                p->mOpenPlaybackDeviceIndex = 0;
                p->mPlaybackContext = alcCreateContext(p->mPlaybackDevice, NULL);
                if (p->mPlaybackContext)    {
                    alcMakeContextCurrent(p->mPlaybackContext);
                    alGenBuffers(1,&p->mPlaybackBuffer);
                    alGenSources(1,&p->mPlaybackSource);
                    if (p->mPlaybackSource && p->mPlaybackBuffer) {
                        alSourcei(p->mPlaybackSource, AL_SOURCE_RELATIVE, AL_TRUE);
                        alSource3f(p->mPlaybackSource, AL_POSITION, 0.0f, 0.0f, 0.0f);
                        alSource3f(p->mPlaybackSource, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
                    }
                    SND_AL_CHECKERROR;
                    if (mustStartPlaybackDevice) p->mOpenPlaybackDeviceStarted = 1;
                    else alcMakeContextCurrent(NULL);
                }
            }
            else {p->mOpenPlaybackDeviceName = NULL;p->mOpenPlaybackDeviceIndex = 0;p->mOpenPlaybackDeviceStarted = 0;}
        }
    }
    return 1;
}




void snd_DiscreteFourierTransform(snoat out[][2],const snoat in[][2],int size_pot,int log2size,int inverse_transform,int dont_scale_inverse_transform_like_fftw3_does)  {
    // Code copied from minimath.h (https://github.com/Flix01/Header-Only-GL-Helpers)
    /*  DFT Stuff
        For k and n in [0,N-1]:
        X(k) = Σ[n:0->N-1] x(n)*exp(-i*2*PI*k*n/N) = Σ[n:0->N-1] {x(n)*[cos(2*PI*k*n/N) - i*sin(2*PI*k*n/N)]};
        x(n) = (1/N)*Σ[k:0->N-1] X(k)*exp(i*2*PI*k*n/N) = (1/N)*Σ[k:0->N-1] {X(k)*[cos(2*PI*k*n/N) + i*sin(2*PI*k*n/N)]};

        DFT of real and purely imaginary signals
        ->  If x in [0,N-1] are real numbers, as they often are in practical applications,
        then the DFT X in [0,N−1] is even symmetric: X(k) = X*(-k) -> real part (and |X(k)| too) even, imaginary part (and arg(X(k)) too) odd.
        It follows that, for even N, X(0) and X(N/2) are real-values, and the remainder of the DFT is completely specified
        by just N/2−1 complex numbers.

        -> If x in [0,N-1] are purely imaginary numbers, then the DFT X in [0,N−1] is odd symmetric


        The complex numbers X(k) 'represent' the amplitude and phase of the different spectral components of the input signal x(n).
        We can express the numbers X(k) in polar form, so we have the 'have' the amplitude and phase of each component explicitly:
        A(k) =  |X(k)|   = sqrt(Re(X(k))^2 + Im(X(k))^2);
        φ(k) = arg(X(k)) = atan2(Im(X(k)), Re(X(k)));
        So that:
        X(k) = |X(k)|*exp(i*arg(X(k))) = A(k)*exp(i*φ(k))
        Re(X(k)) = |X(k)|*cos(arg(X(k))) = A(k)*cos(φ(k))
        Im(X(k)) = |X(k)|*sin(arg(X(k))) = A(k)*sin(φ(k))


        Parsifal theorem: Σ[n:0->N-1] |x(n)|^2 = (1/N)*Σ[k:0->N-1] |X(k)|^2

        For a complex number z = x+iy       z* = x-iy
        For a complex number z = r*exp(iθ), r>=0 e θ real,      z* = r*exp(-iθ)
        For a complex number z:     z^-1 = (z*)/|z|

        Euler Formula: exp(ix) = cosx + isinx

    */
    // WORKS! Tested against -lfftw3

//#   define SND_NO_DFT_OPTIMIZATION    // optional (it was reference implementation)
    snoat angle, wtmp, wpr, wpi, wr, wi, tc[2];
    int n = 1, n2, k, m, i, j=0;
#   ifdef SND_NO_DFT_OPTIMIZATION
    const snoat pi2 = (snoat)(M_PI*2.0);
#   else
    const snoat pi = (snoat) M_PI;snoat sin_angle,cos_angle;
#   endif

    SND_ASSERT(1<<log2size==size_pot);

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
/*                    tc[0]=out[j][0];	    tc[1]=out[j][1];
                out[j][0]=out[i][0];	out[j][1]=out[i][1];
                out[i][0]=    tc[0];	out[i][1]=    tc[1];*/
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
#       ifdef SND_NO_DFT_OPTIMIZATION
        angle = (inverse_transform)?pi2/(snoat)n:-pi2/(snoat)n;
        wtmp= snd_sin((snoat)0.5*angle); wpr = -(snoat)2.0*wtmp*wtmp;
        wpi = snd_sin(angle);   /* note that here: wtmp = sin(0.5*angle) and wpi = sin(angle) */
#       else
        angle = (inverse_transform)?pi/(snoat)n:-pi/(snoat)n;   /* half the angle used in the reference code above */
        snd_sincos(angle,&sin_angle,&cos_angle);    /* it can calculate sin_angle and cos_angle together (in new compilers even without the presence of an explicit sincos(...) function when -O2 or -O3 are used) */
        wtmp= sin_angle; wpr = (snoat)2.0*sin_angle;
        wpi = wpr*cos_angle;   /* because: sin(2*a) = 2*sin(a)*cos(a); */
        wpr*= -sin_angle;
#       endif
        wr = (snoat)1.0; wi = (snoat)0.0;
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
        const snoat scale = (snoat)1.0/(snoat)size_pot;
        for(i = 0;i < n /*[n == size_pot]*/;i++) {out[i][0]*=scale; out[i][1]*=scale;}
    }
}

unsigned snd_calculate_next_power_of_two( unsigned x ) {
    // http://stackoverflow.com/questions/1322510/given-an-integer-how-do-i-find-the-next-largest-power-of-two-using-bit-twiddlin
    if (x == 0) return 0;
    x--;
    x |= x >> 1;   // Divide by 2^k for consecutive doublings of k up to 32,
    x |= x >> 2;   // and then or the results.
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x++;           // The result is a number of 1 bits equal to the number
    // of bits in the original number, plus 1. That's the
    // next highest power of 2.
    return x;
}
/*static unsigned snd_calculate_prev_power_of_two( unsigned x ) {
    // http://stackoverflow.com/questions/2679815/previous-power-of-2
    if (x == 0) return 0;
    // x--; Uncomment this, if you want a strictly less than 'x' result.
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    return x - (x >> 1);
}*/

#ifndef SND_NO_STDIO
#include <stdio.h>
const char* snd_GetFileSizeString(size_t size_in_bytes) {
    static char ch[6]="\0";
    static char rv[512]="\0";
    const size_t B = size_in_bytes%1024;
    float sf = 0;
    size_t K = size_in_bytes/1024;
    size_t M = K/1024;
    size_t G = M/1024;
    size_t T = G/1024;
    K = K%1024;
    G = G%1024;
    if (T>0) {strcpy(ch,"Tb");sf=T+(float)G/1024.f;}
    else if (G>0) {strcpy(ch,"Gb");sf=G+(float)M/1024.f;}
    else if (M>0) {strcpy(ch,"Mb");sf=M+(float)K/1024.f;}
    else if (K>0) {strcpy(ch,"Kb");sf=K+(float)B/1024.f;}
    else {strcpy(ch,"b");sf=B;}
    sprintf(rv,"%1.3f %s",sf,ch);
    return rv;
}
#endif // SND_NO_STDIO


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

    snoat moduloMax;
    snoat dft0_re,dftH_re;
    snoat regularBandModuloTrimmingThreshold;
    snoat regularBandModuloDequantizationThreshold;

    unsigned num_intervals;             // in each interval, a local 'moduloMax' is stored for better modulo quantization
    unsigned num_samples_per_interval;  // depends on num_dft_samples_to_keep

    unsigned num_high_precision_data;
    unsigned num_low_precision_data;
    unsigned num_skipped_data;

    unsigned u_private[4];

    // total size = 64 + 5*sizeof(snoat)
};


unsigned char* snd_SerializeDiscreteFourierTransform(size_t* bufferOutSize,int num_samples_input_signal,const snoat dft[][2],const sndDFTCompressionParams_t* params/* = NULL*/) {
    sndDFTCompressionParams_t par = SNDDFTCOMPRESSIONPARAMS_DEFAULT_INIT;
    struct sndDFTserializerHeader_t S = SND_ZERO_INIT;
    unsigned i;unsigned char mode = 0;
    unsigned cnt,interval_index;snoat tmp;
    const snoat MaxUnsignedShort = (snoat) 65535;
    snoat a,b,modulo,modulo2,modulo2max=0,modulo2threshold=0,maxPhase=-10000,minPhase=10000,phase;
    unsigned char* bufferOut = NULL;

    unsigned short* maxModulos = NULL;
    unsigned maxNumIntervalsToUse = 0;

    unsigned short num_bits_for_phase_in_high_precision_data = 8,max_high_precision_bit_value_for_phase = 255;
    snoat  MaxHighPrecisionBitValueForModulo = (snoat) 255;
    snoat  MaxHighPrecisionBitValueForPhase = (snoat) 255;

    unsigned char num_bits_for_phase_in_low_precision_data = 4,max_low_precision_bit_value_for_phase = 15;
    snoat  MaxLowPrecisionBitValueForModulo = (snoat) 15;
    snoat  MaxLowPrecisionBitValueForPhase = (snoat) 15;

    unsigned short* dft_high_precision_data = NULL;     // 'S.num_high_precision_data' values
    unsigned char*  dft_low_precision_data = NULL;      //  'S.num_low_precision_data' values
    unsigned cnt_high = 0,cnt_low = 0,cnt_skipped = 0;
    
    unsigned char* buf = NULL;
    const char* tagNames = "D0L0";
    for (i=0;i<4;i++) {S.tag[i]=tagNames[i];}
#   ifdef SND_BIG_ENDIAN
    S.tag[2]='B';
#   endif
    S.num_channels = 1; // num_channels (mandatory: we assume it's always 1 in any case)

    SND_ASSERT(sizeof(struct sndDFTserializerHeader_t)==64 + 5*sizeof(snoat));
    SND_ASSERT(dft);
    SND_ASSERT(num_samples_input_signal>=4);
    SND_ASSERT(bufferOutSize);
    if (params) par = *params;

    // clamp 'par' values
    if (par.high_frequency_trimmer<0) par.high_frequency_trimmer=0;
    else if (par.high_frequency_trimmer>1) par.high_frequency_trimmer=1;
    if (par.regular_frequency_trimmer<0) par.regular_frequency_trimmer=0;
    else if (par.regular_frequency_trimmer>1) par.regular_frequency_trimmer=1;
    if (par.quantization_enhancer<0) par.quantization_enhancer=0;
    if (par.quantization_worsening<0) par.quantization_worsening=0;
    else if (par.quantization_worsening>1) par.quantization_worsening=1;
    if (par.num_high_precision_bits_for_modulos<2) par.num_high_precision_bits_for_modulos=2;
    else if (par.num_high_precision_bits_for_modulos>14) par.num_high_precision_bits_for_modulos=14;
    if (par.num_low_precision_bits_for_modulos<2) par.num_low_precision_bits_for_modulos=2;
    else if (par.num_low_precision_bits_for_modulos>6) par.num_low_precision_bits_for_modulos=6;

    S.num_bits_for_modulo_in_high_precision_data = par.num_high_precision_bits_for_modulos;   // num bits reserved for the modulo in 'num_high_precision_data' in [2,14], we hard-code it to 8.
    S.num_bits_for_modulo_in_low_precision_data = par.num_low_precision_bits_for_modulos;    // num bits reserved for the modulo in 'num_low_precision_data' in [2,6], we hard-code it to 4.

    SND_ASSERT(S.num_bits_for_modulo_in_high_precision_data>=2 && S.num_bits_for_modulo_in_high_precision_data<=14);                 // wrong number of modulo bits in dft_modulo_plus_phase
    num_bits_for_phase_in_high_precision_data = 16-S.num_bits_for_modulo_in_high_precision_data;
    max_high_precision_bit_value_for_phase = (1<<num_bits_for_phase_in_high_precision_data)-1;
    MaxHighPrecisionBitValueForModulo = (snoat) ((1<<S.num_bits_for_modulo_in_high_precision_data)-1);
    MaxHighPrecisionBitValueForPhase = (snoat) max_high_precision_bit_value_for_phase;

    SND_ASSERT(S.num_bits_for_modulo_in_low_precision_data>=2 && S.num_bits_for_modulo_in_low_precision_data<=6);                 // wrong number of modulo bits in dft_modulo_plus_phase
    num_bits_for_phase_in_low_precision_data = 8-S.num_bits_for_modulo_in_low_precision_data;
    max_low_precision_bit_value_for_phase = (1<<num_bits_for_phase_in_low_precision_data)-1;
    MaxLowPrecisionBitValueForModulo = (snoat) ((1<<S.num_bits_for_modulo_in_low_precision_data)-1);
    MaxLowPrecisionBitValueForPhase = (snoat) max_low_precision_bit_value_for_phase;

    S.sizeOfThisStruct = (unsigned) sizeof(struct sndDFTserializerHeader_t);
    S.samplerate = SND_SAMPLERATE;
    S.num_samples = num_samples_input_signal;
    S.size_pot = (int) snd_calculate_next_power_of_two((unsigned) S.num_samples);
    S.log2size=0;while((1U<<(unsigned)S.log2size)<S.size_pot) {++S.log2size;if (S.log2size==255) break;}
    S.num_dft_samples_to_keep = 0;
    SND_ASSERT(1U<<(unsigned)S.log2size==S.size_pot);

    // trimming dft using minDFTmoduloToKeep
    for (i=S.size_pot/2-1;i>0;--i)  {
        a=dft[i][0];b=dft[i][1];a*=a;b*=b;modulo2=a+b;
        if (modulo2max<modulo2) modulo2max=modulo2;
    }
    modulo2threshold = modulo2max*par.high_frequency_trimmer*par.high_frequency_trimmer;
    for (i=S.size_pot/2-1;i>0;--i)  {
        a=dft[i][0];b=dft[i][1];a*=a;b*=b;modulo2=a+b;
        // please keep >, and not >=, in next line (for the case modulo2max==0 -> no signal -> num_dft_samples_to_keep=0)
        if (modulo2>modulo2threshold) {
            // We must discard samples [i+1,S.size_pot/2-1], and keep samples in [1,i]
            S.num_dft_samples_to_keep = i;
            break;
        }
    }
    SND_ASSERT(S.num_dft_samples_to_keep<=S.size_pot/2-1);
    if (modulo2max==0)  {S.moduloMax = 0;SND_ASSERT(S.num_dft_samples_to_keep==0);}
    else S.moduloMax = snd_sqrt(modulo2max);
    S.regularBandModuloTrimmingThreshold = S.moduloMax*par.regular_frequency_trimmer;
    S.regularBandModuloDequantizationThreshold = S.moduloMax*par.quantization_worsening;

    // Test: renormalize these two values so they can be equal to reconstructed floating-point values
//#   define SND_FORCE_RENORMALIZE_REGULARBANDMODULODEQUANTIZATIONTHRESHOLD   // This could be used
//#   define SND_FORCE_RENORMALIZE_REGULARBANDMODULOTRIMMINGTHRESHOLD         // Better not
#   ifdef SND_FORCE_RENORMALIZE_REGULARBANDMODULODEQUANTIZATIONTHRESHOLD
    S.regularBandModuloDequantizationThreshold = MaxUnsignedShort*(snoat)((unsigned short)(S.regularBandModuloDequantizationThreshold*S.moduloMax/MaxUnsignedShort))/S.moduloMax;
#   endif
#   ifdef SND_FORCE_RENORMALIZE_REGULARBANDMODULOTRIMMINGTHRESHOLD
    S.regularBandModuloTrimmingThreshold = MaxUnsignedShort*(snoat)((unsigned short)(S.regularBandModuloTrimmingThreshold*S.moduloMax/MaxUnsignedShort))/S.moduloMax;
#   endif

    //  |--|--|--|--|--|
    //  0  1  2  3  4  5=S.size_pot/2
    //
    // samples at 0 and at S.size_pot/2 are processed separately.
    // The remaining S.size_pot/2-1 samples go from 1 to S.size_pot/2-1 included,
    // i.e. are in [1,S.size_pot/2-1]
    //
    // here: num_dft_samples_to_keep is in [0,S.size_pot/2-1], and starts at sample 1.
    // =>   for (i=1;i<=num_dft_samples_to_keep;i++) is the correct for loop to use


    // allocating S.num_intervals and calculating maxModulo in each interval (better quantization)
    SND_ASSERT(S.size_pot/2-1>0);
    maxModulos = NULL;
    maxNumIntervalsToUse = (int) par.quantization_enhancer*(S.num_dft_samples_to_keep/200);
    //int maxNumIntervalsToUse = (int) ((snoat)3*(snoat)S.log2size*((snoat)1+(snoat)S.num_dft_samples_to_keep/(snoat)(S.size_pot/2-1)));
    if (maxNumIntervalsToUse<2) maxNumIntervalsToUse = 2;
    if (maxNumIntervalsToUse>=S.num_dft_samples_to_keep/2) maxNumIntervalsToUse = S.num_dft_samples_to_keep/2;
    S.num_intervals = S.num_dft_samples_to_keep<=maxNumIntervalsToUse ? S.num_dft_samples_to_keep : maxNumIntervalsToUse;
    S.num_samples_per_interval = S.num_intervals==0?0:S.num_dft_samples_to_keep/S.num_intervals;
    if (S.num_intervals==0) {SND_ASSERT(S.num_dft_samples_to_keep==0 && S.num_samples_per_interval==0);}
    else {
        mode = 0;tmp = 0;
        SND_ASSERT(S.moduloMax>0);
        maxModulos = (unsigned short *) SND_MALLOC(S.num_intervals*sizeof(*maxModulos));
        memset(maxModulos,0,S.num_intervals*sizeof(*maxModulos));
        cnt=0,interval_index=0;tmp=0;
        SND_ASSERT(S.num_high_precision_data==0 && S.num_low_precision_data==0 && S.num_skipped_data==0);
        for (i=1;i<=S.num_dft_samples_to_keep;i++)  {
            a=dft[i][0];b=dft[i][1];a*=a;b*=b;modulo2=a+b;
            if (tmp<modulo2) tmp=modulo2;
            if (++cnt>=S.num_samples_per_interval && (interval_index!=S.num_intervals-1 || i==S.num_dft_samples_to_keep))  {                        
                unsigned short *pMaxModulo = &maxModulos[interval_index];
                SND_ASSERT(interval_index<S.num_intervals);
                modulo = tmp>0 ? snd_sqrt(tmp) : 0;
                SND_ASSERT(modulo>=0 && modulo<=S.moduloMax);
                /*if (modulo==S.moduloMax)    {
                    SND_ASSERT(modulo>S.moduloMax*0.5);
                }*/
                *pMaxModulo = modulo>0 ? (unsigned short)((MaxUnsignedShort*modulo)/S.moduloMax) : 0;

                modulo =  (snoat)(*pMaxModulo)*S.moduloMax/MaxUnsignedShort; // this is MANDATORY! When we reload, we use the reconstructed value to partition data!
                if (modulo < S.regularBandModuloTrimmingThreshold) {
                    S.num_skipped_data+= cnt;  // No data will be saved in this interval
#                   ifdef SND_FORCE_RESET_MODULOS_WHEN_SKIPPED
                    *pMaxModulo = (snoat) 0;    // (optional-BETTER NOT) helps fixing some floating-point errors later
#                   endif
                }
                else if (modulo < S.regularBandModuloDequantizationThreshold)   S.num_low_precision_data+=cnt;
                else    S.num_high_precision_data+=cnt;;

                tmp = 0;cnt = 0;++interval_index;
            }
        }
        SND_ASSERT(interval_index==S.num_intervals);
        SND_ASSERT(S.num_high_precision_data+S.num_low_precision_data+S.num_skipped_data == S.num_dft_samples_to_keep);

        // However when S.regularBandModuloDequantizationThreshold == S.maxModulo,
        // then S.num_high_precision_data can be positive (40). Why?
    }

    SND_ASSERT(sizeof(*dft_high_precision_data)==sizeof(unsigned short));
    dft_high_precision_data = (unsigned short*) SND_MALLOC(S.num_high_precision_data*sizeof(*dft_high_precision_data));
    dft_low_precision_data = (unsigned char*) SND_MALLOC(S.num_low_precision_data*sizeof(*dft_low_precision_data));
    cnt_high = 0;cnt_low = 0;cnt_skipped = 0;
    cnt=0,interval_index=0;
    tmp=0;mode=0; // 0-> normal, 1-> low quantization interval, 2-> skip interval
    if (S.num_intervals>0)  {
        SND_ASSERT(interval_index<S.num_intervals);
        tmp = (snoat)maxModulos[interval_index++]*S.moduloMax/MaxUnsignedShort;
        SND_ASSERT(tmp>=0 && tmp<=S.moduloMax);
        cnt = 0;mode = tmp < S.regularBandModuloTrimmingThreshold ? 2 : (tmp < S.regularBandModuloDequantizationThreshold ? 1 : 0);
    }
    for (i=1;i<=S.num_dft_samples_to_keep;i++)  {
        //unsigned char *dft_modulo = &dft_modulos[i-1],*dft_phase = &dft_phases[i-1];
        a=dft[i][0];b=dft[i][1];
        if (mode<2) {
            // modulo
            modulo2=a*a+b*b;
            modulo = modulo2>0 ? snd_sqrt(modulo2) : 0;
#           define SND_FORCE_CLAMP_MODULOS
#           ifdef SND_FORCE_CLAMP_MODULOS
            if (modulo>tmp) modulo=tmp; // (optional-better not) floating-point precision fix
#           endif
            // phase
            phase = snd_atan2(b,a);
#           define SND_FORCE_CLAMP_PHASES
#           ifdef SND_FORCE_CLAMP_PHASES
            if (phase<-M_PI)        phase=-M_PI;
            else if (phase>M_PI)    phase=M_PI;
#           endif
            if (minPhase>phase)         minPhase=phase;
            else if (maxPhase<phase)    maxPhase=phase;
            if (mode==0)    {
                const unsigned short mdl = tmp>0 ? (unsigned short) (MaxHighPrecisionBitValueForModulo*modulo/tmp) : 0;
                const unsigned short phs = (unsigned short) (MaxHighPrecisionBitValueForPhase*(phase+(snoat)M_PI)/(snoat)(M_PI*2.0));
                const unsigned short combo = mdl<<num_bits_for_phase_in_high_precision_data | phs;
                dft_high_precision_data[cnt_high++] = combo;
                SND_ASSERT((combo&max_high_precision_bit_value_for_phase)==phs);
                SND_ASSERT(combo>>num_bits_for_phase_in_high_precision_data==mdl);
            }
            else    {                
                const unsigned char mdl = tmp>0 ? (unsigned char) (MaxLowPrecisionBitValueForModulo*modulo/tmp) : 0;
                const unsigned char phs = (unsigned char) (MaxLowPrecisionBitValueForPhase*(phase+(snoat)M_PI)/(snoat)(M_PI*2.0));
                const unsigned char combo = mdl<<num_bits_for_phase_in_low_precision_data | phs;
                dft_low_precision_data[cnt_low++] = combo;
                SND_ASSERT((combo&max_low_precision_bit_value_for_phase)==phs);
                SND_ASSERT(combo>>num_bits_for_phase_in_low_precision_data==mdl);
            }
        }
        else ++cnt_skipped; // We do nothing

        if (++cnt>=S.num_samples_per_interval && interval_index<S.num_intervals)  {
            tmp = (snoat)maxModulos[interval_index++]*S.moduloMax/MaxUnsignedShort;
            SND_ASSERT(tmp>=0 && tmp<=S.moduloMax);
            cnt = 0;mode = tmp < S.regularBandModuloTrimmingThreshold ? 2 : (tmp < S.regularBandModuloDequantizationThreshold ? 1 : 0);
        }
    }
    if (S.num_dft_samples_to_keep>0)    {
        SND_ASSERT(minPhase>=-M_PI-0.001);
        SND_ASSERT(maxPhase<=M_PI+0.001);
    }
    SND_ASSERT(cnt_high==S.num_high_precision_data);
    SND_ASSERT(cnt_low==S.num_low_precision_data);  // cnt_low==S.num_low_precision_data-1 ???


    // other S fields
    S.dft0_re = dft[0][0];
    S.dftH_re = dft[S.size_pot/2][0];

    // serialize S
    *bufferOutSize = sizeof(struct sndDFTserializerHeader_t) + S.num_high_precision_data * sizeof(*dft_high_precision_data) + S.num_low_precision_data * sizeof(*dft_low_precision_data) + S.num_intervals * sizeof(*maxModulos);
    bufferOut = (unsigned char*) SND_MALLOC(*bufferOutSize);
    memcpy(bufferOut,&S,sizeof(S));
    buf = &bufferOut[sizeof(S)];
    memcpy(buf,dft_high_precision_data,S.num_high_precision_data*sizeof(*dft_high_precision_data));
    buf+=S.num_high_precision_data*sizeof(*dft_high_precision_data);
    memcpy(buf,dft_low_precision_data,S.num_low_precision_data*sizeof(*dft_low_precision_data));
    buf+=S.num_low_precision_data*sizeof(*dft_low_precision_data);
    memcpy(buf,maxModulos,S.num_intervals*sizeof(*maxModulos));
    buf+=S.num_intervals*sizeof(*maxModulos);
    SND_ASSERT(buf-bufferOut==(long int)(*bufferOutSize));
    /*
        // So we must store:
        A) dft[0][0];dft[S.size_pot/2][0];
        B) for (i=1;i<S.size_pot/2;i++)   {
                dft_output[i][0];dft_output[i][1];
            }
        num_values_to_store_total = 2 + 2*(S.size_pot/2-1-num_dft_samples_to_discard) = S.size_pot-num_dft_samples_to_discard (which is >= S.num_samples if num_dft_samples_to_discard==0!)
        [num_dft_samples_to_keep = 2*(S.size_pot/2-1-num_dft_samples_to_discard)]

        // To rebuild dft_output:
        A) dft[0][1] = dft[S.size_pot/2][1] = 0;
        B) for (i=1;i<S.size_pot/2;i++)   {
                const int j = S.size_pot-i;
                dft[j][0] = dft[i][0];
                dft[j][1] = -dft[i][1];
           }
    */

    SND_FREE(maxModulos);maxModulos=NULL;
    SND_FREE(dft_high_precision_data);dft_high_precision_data=NULL;
    SND_FREE(dft_low_precision_data);dft_low_precision_data=NULL;


    return bufferOut;

}
size_t snd_DeserializeDiscreteFourierTransform(snoat dft_out[][2], size_t dft_out_size, const unsigned char* bufferIn, size_t bufferInSize) {
    const struct sndDFTserializerHeader_t* S = NULL;
    const unsigned char* buf = bufferIn;
    const unsigned short* dft_high_precision_data = NULL;
    const unsigned char* dft_low_precision_data = NULL;
    const unsigned short* maxModulos = NULL;
    unsigned i;unsigned char mode = 0;
    unsigned cnt,cnt_high,cnt_low,cnt_skipped,interval_index;snoat tmp;
    const snoat MaxUnsignedShort = (snoat) 65535;
    snoat  MaxHighPrecisionBitValueForModulo = (snoat) 255;
    snoat  MaxHighPrecisionBitValueForPhase = (snoat) 255;
    unsigned short num_bits_for_phase_in_high_precision_data = 8,max_high_precision_bit_value_for_phase = 255;
    snoat  MaxLowPrecisionBitValueForModulo = (snoat) 15;
    snoat  MaxLowPrecisionBitValueForPhase = (snoat) 15;
    unsigned char num_bits_for_phase_in_low_precision_data = 4,max_low_precision_bit_value_for_phase = 15;
    snoat modulo,phase,cos_phase,sin_phase;
    long int delta_buf = 0;
    snoat minPhase=10000,maxPhase=-10000;

    SND_ASSERT(dft_out && dft_out_size>=4 && bufferIn && bufferInSize);
    SND_ASSERT(sizeof(*S)<=bufferInSize);
    S = (const struct sndDFTserializerHeader_t*) buf;
    SND_ASSERT(S->sizeOfThisStruct == (unsigned)sizeof(*S));
#   ifndef SND_BIG_ENDIAN
    SND_ASSERT(strncmp((char*)S->tag,"D0L0",4)==0);    // Wrong file/version/endianess/compression
#   else
    SND_ASSERT(strncmp((char*)S->tag,"D0B0",4)==0);    // Wrong file/version/endianess/compression
#   endif
    SND_ASSERT(S->num_channels==1);                 // Wrong num_channels

    SND_ASSERT(S->num_bits_for_modulo_in_high_precision_data>=2 && S->num_bits_for_modulo_in_high_precision_data<=14);                 // wrong number of modulo bits in num_bits_for_modulo_in_high_precision_data
    num_bits_for_phase_in_high_precision_data = 16-S->num_bits_for_modulo_in_high_precision_data;
    max_high_precision_bit_value_for_phase = (1<<num_bits_for_phase_in_high_precision_data)-1;
    MaxHighPrecisionBitValueForModulo = (snoat) ((1<<S->num_bits_for_modulo_in_high_precision_data)-1);
    MaxHighPrecisionBitValueForPhase = (snoat) max_high_precision_bit_value_for_phase;

    SND_ASSERT(S->num_bits_for_modulo_in_low_precision_data>=2 && S->num_bits_for_modulo_in_low_precision_data<=6);                 // wrong number of modulo bits in num_bits_for_modulo_in_low_precision_data
    num_bits_for_phase_in_low_precision_data = 8-S->num_bits_for_modulo_in_low_precision_data;
    max_low_precision_bit_value_for_phase = (1<<num_bits_for_phase_in_low_precision_data)-1;
    MaxLowPrecisionBitValueForModulo = (snoat) ((1<<S->num_bits_for_modulo_in_low_precision_data)-1);
    MaxLowPrecisionBitValueForPhase = (snoat) max_low_precision_bit_value_for_phase;

    buf+=sizeof(*S);
    dft_high_precision_data = (const unsigned short*) buf;
    buf+=S->num_high_precision_data*sizeof(*dft_high_precision_data);
    dft_low_precision_data = (const unsigned char*) buf;
    buf+=S->num_low_precision_data*sizeof(*dft_low_precision_data);
    maxModulos = (const unsigned short*) buf;
    buf+=S->num_intervals*sizeof(*maxModulos);
    delta_buf = (long int)(buf-bufferIn) - (long int)bufferInSize;
    SND_ASSERT(delta_buf>-4 && delta_buf<4);    // that's because size is clamped to the next integer (4 bytes)

    SND_ASSERT((unsigned)dft_out_size>=S->size_pot);

    for (i=0;i<(unsigned)dft_out_size;i++) dft_out[i][0]=dft_out[i][1]=0;    // clear all dft_out

    // fill dft_out
    dft_out[0][0] = S->dft0_re;
    dft_out[S->size_pot/2][0] = S->dftH_re;

    SND_ASSERT(S->num_dft_samples_to_keep<=S->size_pot/2-1);
    cnt=cnt_high=cnt_low=cnt_skipped=0,interval_index=0;
    tmp=0;mode=0; // 0-> normal, 1-> low quantization interval, 2-> skip interval
    if (S->num_intervals>0)  {
        cnt = 0;tmp = (snoat)maxModulos[interval_index++]*S->moduloMax/MaxUnsignedShort;
        if (tmp<0) tmp=0;
        else if (tmp>S->moduloMax) tmp=S->moduloMax;
        mode = tmp < S->regularBandModuloTrimmingThreshold ? 2 : (tmp < S->regularBandModuloDequantizationThreshold ? 1 : 0);
    }
    for (i=1;i<=S->num_dft_samples_to_keep;i++)  {
        if (mode < 2)   {
            if (mode==0)    {
                /*modulo = ((snoat)dft_modulos[cnt_high]*tmp/MaxDftModuloSize);
                phase = ((snoat)dft_phases[cnt_high]*(snoat)(M_PI*2.0)/MaxDftPhaseSize - (snoat)M_PI);
                ++cnt_high;*/
                const unsigned short combo = dft_high_precision_data[cnt_high++];
                const unsigned short mdl = combo>>num_bits_for_phase_in_high_precision_data;
                const unsigned short phs = combo&max_high_precision_bit_value_for_phase;
                modulo = ((snoat)mdl*tmp/MaxHighPrecisionBitValueForModulo);
                phase = ((snoat)phs*(snoat)(M_PI*2.0)/MaxHighPrecisionBitValueForPhase - (snoat)M_PI);
            }
            else    {
                const unsigned char combo = dft_low_precision_data[cnt_low++];
                const unsigned char mdl = combo>>num_bits_for_phase_in_low_precision_data;
                const unsigned char phs = combo&max_low_precision_bit_value_for_phase;
                modulo = ((snoat)mdl*tmp/MaxLowPrecisionBitValueForModulo);
                phase = ((snoat)phs*(snoat)(M_PI*2.0)/MaxLowPrecisionBitValueForPhase - (snoat)M_PI);
            }
            if (minPhase>phase) minPhase=phase;
            else if (maxPhase<phase) maxPhase=phase;

            /*if (modulo<0)           modulo=0;
            else if (modulo>tmp)    modulo=tmp;
            if (phase<-M_PI
                //|| phase>M_PI
                )        phase= -M_PI;
            else if (phase>M_PI)    phase= M_PI;*/

            // Re and Im parts
            snd_sincos(phase,&sin_phase,&cos_phase);
            dft_out[i][0] = modulo*cos_phase;
            dft_out[i][1] = modulo*sin_phase;
        }
        else {
            dft_out[i][0] = dft_out[i][1]= 0;
            ++cnt_skipped;
        }



        if (++cnt>=S->num_samples_per_interval && interval_index<S->num_intervals)  {
            cnt = 0;tmp = (snoat)maxModulos[interval_index++]*S->moduloMax/MaxUnsignedShort;
            if (tmp<0) tmp=0;
            else if (tmp>S->moduloMax) tmp=S->moduloMax;
            mode = tmp < S->regularBandModuloTrimmingThreshold ? 2 : (tmp < S->regularBandModuloDequantizationThreshold ? 1 : 0);
        }
    }
    SND_ASSERT(cnt_high==S->num_high_precision_data);
    SND_ASSERT(cnt_low==S->num_low_precision_data);
    SND_ASSERT(cnt_skipped==S->num_skipped_data);

    SND_ASSERT(minPhase>=-M_PI-0.001);
    SND_ASSERT(maxPhase<=M_PI+0.001);

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




size_t snd_Base85Decode(const char* input, char** poutput, size_t* poutputSizeInOut)    {
    // Based on code contained in imgui_draw.cpp (Dear ImGui at https://github.com/ocornut/imgui, MIT licensed)
#   define SND_DECODE85BYTE(c)   (((c) >= '\\') ? ((c)-36) : ((c)-35))        
    const size_t outputSize = (((size_t)strlen(input) + 4) / 5) * 4;
    const unsigned char *src = (const unsigned char *) input;
    unsigned char *dst = (unsigned char *) *poutput;
    SND_ASSERT(input);
    SND_ASSERT(poutput && poutputSizeInOut);
    if (!input) return 0;
    if (!*poutput || *poutputSizeInOut<outputSize) {
        *poutput = (char*)SND_REALLOC(*poutput,outputSize);
        dst = (unsigned char *) *poutput;
        *poutputSizeInOut = outputSize;
    }
    while (*src)    {
        unsigned int tmp = SND_DECODE85BYTE(src[0]) + 85*(SND_DECODE85BYTE(src[1]) + 85*(SND_DECODE85BYTE(src[2]) + 85*(SND_DECODE85BYTE(src[3]) + 85*SND_DECODE85BYTE(src[4]))));
        dst[0] = ((tmp >> 0) & 0xFF); dst[1] = ((tmp >> 8) & 0xFF); dst[2] = ((tmp >> 16) & 0xFF); dst[3] = ((tmp >> 24) & 0xFF);   // We can't assume little-endianness.
        src += 5;
        dst += 4;
    }
    return outputSize;
#   undef SND_DECODE85BYTE
}


__inline static char snd_Encode85Byte(unsigned int x) 	{x = (x % 85) + 35;return (x>='\\') ? x+1 : x;}
size_t snd_Base85Encode(const char* input,size_t inputSize,char** poutput,size_t* poutputSizeInOut,int stringifiedMode/*=0*/,int numCharsPerLineInStringifiedMode/*=112*/,int noBackslashesInStringifiedMode/*=0*/)	{
    // Adapted from binary_to_compressed_c(...) inside imgui_draw.cpp
    // in the Dear ImGui library (MIT licensed)
    const size_t outputSize = (size_t)((float)inputSize*1.3f);size_t size = 0;
    char prev_c = 0;int cnt=0;
    size_t src_i;unsigned int n5;
    SND_ASSERT(input && inputSize>0);
    SND_ASSERT(poutput && poutputSizeInOut);
    if (!input || inputSize==0) return 0;
    if (!*poutput || *poutputSizeInOut<outputSize) {
        *poutput = (char*)SND_REALLOC(*poutput,outputSize);
        *poutputSizeInOut = outputSize;
    }
    if (numCharsPerLineInStringifiedMode<=12) numCharsPerLineInStringifiedMode = 12;

    if (stringifiedMode) {
        if (noBackslashesInStringifiedMode) {
            //SND_ASSERT(size<*poutputSizeInOut);(*poutput)[size++]='{';
            //SND_ASSERT(size<*poutputSizeInOut);(*poutput)[size++]='\n';
            SND_ASSERT(size<*poutputSizeInOut);(*poutput)[size++]='	';
        }
        SND_ASSERT(size<*poutputSizeInOut);(*poutput)[size++]='"';
    }
    prev_c = 0;cnt=0;
    for (src_i = 0; src_i < inputSize; src_i += 4)	{
        unsigned int d = *(unsigned int*)(input + src_i);
        for (n5 = 0; n5 < 5; n5++, d /= 85)	{
            char c = snd_Encode85Byte(d);
            if (stringifiedMode && c == '?' && prev_c == '?') {SND_ASSERT(size<*poutputSizeInOut);(*poutput)[size++]='\\';}	// This is made a little more complicated by the fact that ??X sequences are interpreted as trigraphs by old C/C++ compilers. So we need to escape pairs of ??.
            SND_ASSERT(size<*poutputSizeInOut);(*poutput)[size++]=c;
            prev_c = c;
        }
        cnt+=4;
        if (stringifiedMode && cnt>=numCharsPerLineInStringifiedMode)	{
            SND_ASSERT(size+4<*poutputSizeInOut);
            (*poutput)[size++]='"';
            if (!noBackslashesInStringifiedMode) {
                (*poutput)[size++]='	';
                (*poutput)[size++]='\\';
            }
            (*poutput)[size++]='\n';
            if (noBackslashesInStringifiedMode)  {
                (*poutput)[size++]='	';
            }
            (*poutput)[size++]='"';
            cnt=0;
        }
    }
    // End
    if (stringifiedMode) {
        SND_ASSERT(size+3<*poutputSizeInOut);
        (*poutput)[size++]='"';
        if (!noBackslashesInStringifiedMode) {
            (*poutput)[size++]=';';
            (*poutput)[size++]='\n';
            (*poutput)[size++]='\n';
        }
        else {
            (*poutput)[size++]='\n';
            //(*poutput)[size++]='}';
            (*poutput)[size++]='\n';
        }
    }
    if (!stringifiedMode) {SND_ASSERT(size<*poutputSizeInOut);(*poutput)[size++]='\0';}	// End character

    return size;
}



#ifndef SND_NO_STDIO
#include <stdio.h>
size_t snd_SaveAsWavFile(const char* savePath,const float* samples,size_t num_samples,int samplerate,int num_channels)    {
#   define SND_ENDIAN_SWAP_USHORT(X)  {unsigned short TMP=X;unsigned char *pT=(unsigned char*)&TMP,*pX=(unsigned char*)&X;pX[0]=pT[1];pX[1]=pT[0];}
#   define SND_ENDIAN_SWAP_UINT(X)  {unsigned short TMP=X;unsigned char *pT=(unsigned char*)&TMP,*pX=(unsigned char*)&X;pX[0]=pT[3];pX[1]=pT[2];pX[2]=pT[1];pX[3]=pT[0];}
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
    } H = SND_ZERO_INIT;
    FILE* f = fopen(savePath,"wb");
    size_t i;
    SND_ASSERT(f);
    if (!f) return 0;
    SND_ASSERT(sizeof(H)==44);
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
#endif //SND_NO_STDIO


#endif //SND_SYSTEM_IMPLEMENTATION_H
#endif //SND_SYSTEM_IMPLEMENTATION
