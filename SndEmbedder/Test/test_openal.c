// https://github.com/Flix01/Flix-Tools

// COMPILATION INSTRUCTIONS:
/*
// with gcc (for clang just replace gcc with clang and see if it works):
gcc  -O3 -no-pie -fno-pie test_openal.c -o test_openal -lasound -lpthread
// with mingw (here for Windows 64bit):
x86_64-w64-mingw32-gcc -O3 -no-pie -fno-pie -mconsole test_openal.c -o test_openal.exe -DWINVER=0x0800 -D_WIN32 -D_WIN64 -luser32 -lkernel32 -lOpenAL32
// with cl (here for Windows 32bit, VC 7.1 2003: hence /DSNDD_NO_C99_MATH_FUNCTIONS)
cl /TC /O2 /ML /DSNDD_NO_C99_MATH_FUNCTIONS test_openal.c /link /out:test_openal.exe Shell32.lib user32.lib kernel32.lib OpenAL32.lib
// with emscripten (be sure to create the 'html' subfolder BEFORE running emcc)
emcc -O3 test_openal.c -o html/test_openal.html -s ALLOW_MEMORY_GROWTH=1 -s ASYNCIFY=1 -s ASYNCIFY_IGNORE_INDIRECT=1

// NOTES ABOUT COMMANDLINE OPTIONS:
// -no-pie -fno-pie just make sure that the compiled source is considered an executable file (and not a shared object)
// -fopenmp-simd -DNDEBUG can be used to increase performace (-fopenmp-simd does NOT need openmp!).
// -DNDEBUG should remove all the asserts, so it's probably better to avoid it when performance is not a priority.
// -s ASYNCIFY=1 is mandatory for emscripten_sleep(...), since plain usleep(...) does not seem to work correctly. It's recommended to use -O3 (instead of -O2) to riduce file size when using it.
// -s ASYNCIFY_IGNORE_INDIRECT=1 should reduce the overhead of -s ASYNCIFY=1
// -std=gnu89 -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-function -Werror=declaration-after-statement  can be used to restric the C version
*/

// openal headers
//#define ALEXT_H_IS_PRESENT    // optional (well, if you have 'AL/alext.h', define it!)
#ifdef ALEXT_H_IS_PRESENT
#   include <AL/alext.h>
#else //ALEXT_H_IS_PRESENT
#   ifndef AL_EXT_float32      // we assume AL_EXT_float32 is implemented
#      define AL_EXT_float32 1
#      define AL_FORMAT_MONO_FLOAT32                   0x10010
#      define AL_FORMAT_STEREO_FLOAT32                 0x10011
#   endif //AL_EXT_float32
#endif //__EMSCRIPTEN__ */
#include <AL/al.h>
#include <AL/alc.h>
#include <string.h> // memset

// sleep_ms(...)
#ifdef _WIN32
#   include <windows.h>    // Sleep()
#   define sleep_ms(X)     Sleep((X))
#elif defined(__EMSCRIPTEN__)
#   include <emscripten.h>
#   define sleep_ms(X)      emscripten_sleep(X)     // needs -s ASYNCIFY=1 [simply using usleep(...) compiles but does not work correctly]
#else   // tested only on Linux
#   include <unistd.h>     // usleep
#   define sleep_ms(X)     usleep((X)*1000)
#endif

// audio format (note that sounds in .inl files use 1 channel with samplerate 22050 Hz, but we can request them in a bunch of different formats)
// ONLY FLOAT32 SAMPLES ARE SUPPORTED (although user can easily convert them to short samples if necessary)!
// Unlike in sokol_audio, in openal leaving the params used when recording audio
// has no further constraints on playback (they are assigned to a single audio source)
#ifndef SAMPLERATE
#   define SAMPLERATE (22050)
#endif //SAMPLERATE
#ifndef NUM_CHANNELS
#   define NUM_CHANNELS 1
#endif //NUM_CHANNELS


#define SNDDECODER_IMPLEMENTATION
#include "sndDecoder.h"     // sndd_DecodeSerializedSound(...)


// some openal helper functions (so that we don't pollute our 'main' function)
typedef struct openal_t {
    ALCdevice *mPlaybackDevice;
    ALCcontext * mPlaybackContext;
    ALuint mPlaybackSource,mPlaybackBuffer;
} openal_t;
void openal_init(openal_t* al)  {
    al->mPlaybackDevice = alcOpenDevice(NULL);
    if (al->mPlaybackDevice)	{
        al->mPlaybackContext = alcCreateContext(al->mPlaybackDevice, NULL);
        if (al->mPlaybackContext)    {
            ALboolean isFloatExtensionPresent = AL_FALSE;
            alcMakeContextCurrent(al->mPlaybackContext);
            alGenBuffers(1,&al->mPlaybackBuffer);
            alGenSources(1,&al->mPlaybackSource);
            if (al->mPlaybackSource && al->mPlaybackBuffer) {
                alSourcei(al->mPlaybackSource, AL_SOURCE_RELATIVE, AL_TRUE);
                alSource3f(al->mPlaybackSource, AL_POSITION, 0.0f, 0.0f, 0.0f);
                alSource3f(al->mPlaybackSource, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
            }
            isFloatExtensionPresent = alIsExtensionPresent("AL_EXT_float32");
            SNDD_ASSERT(isFloatExtensionPresent==AL_TRUE);
            //alcMakeContextCurrent(NULL);  // nope
        }
    }
}
void openal_destroy(openal_t* al)  {
    if (al->mPlaybackContext)    {
        alcMakeContextCurrent(al->mPlaybackContext);
        if (al->mPlaybackBuffer) {alDeleteBuffers(1,&al->mPlaybackBuffer);al->mPlaybackBuffer=0;}
        if (al->mPlaybackSource) {alDeleteSources(1,&al->mPlaybackSource);al->mPlaybackSource=0;}
        alcMakeContextCurrent(NULL);
        alcDestroyContext(al->mPlaybackContext);al->mPlaybackContext=NULL;
    }
    if (al->mPlaybackDevice) {alcCloseDevice(al->mPlaybackDevice);al->mPlaybackDevice=NULL;}
    memset(al,0,sizeof(*al));
}
void openal_feedPlaybackData(openal_t* al,const void* pData,int numSamplesInBytes,ALenum sndFormat,ALsizei sndSamplerate)  {
    alSourceStop(al->mPlaybackSource);
    alSourcei(al->mPlaybackSource, AL_BUFFER, 0);    // The NULL Buffer is extremely useful for detaching buffers from a source which were attached using this call or with alSourceQueueBuffers.
    alBufferData(al->mPlaybackBuffer,sndFormat,(ALvoid*)pData,(ALsizei) numSamplesInBytes,sndSamplerate);
    alSourcei(al->mPlaybackSource, AL_BUFFER, al->mPlaybackBuffer);
}
typedef enum openal_playback_state_e {
    OPENAL_PLAYBACK_STATE_INITIAL = 0x1011,
    OPENAL_PLAYBACK_STATE_PLAYING = 0x1012,
    OPENAL_PLAYBACK_STATE_PAUSED =  0x1013,
    OPENAL_PLAYBACK_STATE_STOPPED = 0x1014
} openal_playback_state_e;
void openal_setPlaybackState(struct openal_t* al,openal_playback_state_e state)  {
    switch (state)  {
    case OPENAL_PLAYBACK_STATE_PLAYING: alSourcePlay(al->mPlaybackSource);break;
    case OPENAL_PLAYBACK_STATE_PAUSED: alSourcePause(al->mPlaybackSource);break;
    case OPENAL_PLAYBACK_STATE_STOPPED: alSourceStop(al->mPlaybackSource);break;
    case OPENAL_PLAYBACK_STATE_INITIAL: alSourceRewind(al->mPlaybackSource);break;
    default:break;
    }
}
// end openal helper functions


int main (int argc,char* argv[])    {
    openal_t al = SNDD_ZERO_INIT;

    // we'll use a single serialized sound, but we can easily combine more of them this way:
    const char* serializedSound[] =    {
#   include "./sounds/bells.inl"   // I usually copy and paste these directly to have one less header file
/*            ,
#   include "./sounds/coin.inl"
        ,
#   include "./sounds/scream.inl"*/
    };
    const int serializedSoundIndex = 0; // we'll only use this
    float* samples = NULL;size_t samples_size = 0;
    size_t num_samples = sndd_DecodeSerializedSound(&samples,&samples_size,serializedSound[serializedSoundIndex],SAMPLERATE,NUM_CHANNELS);
    // 'samples_size' is the allocated size of the 'samples' array (in floats), but 'num_samples' is the real number of sound samples
    const size_t length_ms = (num_samples*1000)/(NUM_CHANNELS*SAMPLERATE);
    SNDD_ASSERT(samples && num_samples && samples_size);

    // openal stuff
    openal_init(&al);
    openal_feedPlaybackData(&al,(const void*) samples,samples_size*sizeof(float),NUM_CHANNELS==1?AL_FORMAT_MONO_FLOAT32:AL_FORMAT_STEREO_FLOAT32,SAMPLERATE);
    openal_setPlaybackState(&al,OPENAL_PLAYBACK_STATE_PLAYING);
    sleep_ms(length_ms);
    openal_destroy(&al);

    // we must free 'samples'
    SNDD_FREE(samples);samples=NULL;samples_size=0;

    (void)argc;(void)argv;
    return 0;
}
