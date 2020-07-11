// https://github.com/Flix01/Flix-Tools

// COMPILATION INSTRUCTIONS:
/*
// with gcc (for clang just replace gcc with clang and see if it works):
gcc -O3 -no-pie -fno-pie test_sokol.c -o test_sokol -lasound -lpthread
// with mingw (here for Windows 64bit):
x86_64-w64-mingw32-gcc -O3 -no-pie -fno-pie -mconsole test_sokol.c -o test_sokol.exe -DWINVER=0x0800 -D_WIN32 -D_WIN64 -luser32 -lkernel32
// with cl (I cannot test this, because VC 7.1 2003 is not enough to build sokol_audio.h: it misses required C99 support)
cl /TC /O2 /MT test_sokol.c /link /out:test_sokol.exe Shell32.lib user32.lib kernel32.lib OpenAL32.lib
// with emscripten (be sure to create the 'html' subfolder BEFORE running emcc)
emcc -O3 test_sokol.c -o html/test_sokol.html -s ALLOW_MEMORY_GROWTH=1 -s ASYNCIFY=1 -s ASYNCIFY_IGNORE_INDIRECT=1

// NOTES ABOUT COMMANDLINE OPTIONS:
// -no-pie -fno-pie just make sure that the compiled source is considered an executable (and not a shared object)
// -fopenmp-simd -DNDEBUG can be used to increase performace (-fopenmp-simd does NOT need openmp!).
// -DNDEBUG should remove all the asserts, so it's probably better to avoid it when performance is not a priority.
// -s ASYNCIFY=1 is mandatory for emscripten_sleep(...), since plain usleep(...) does not seem to work correctly. It's recommended to use -O3 (instead of -O2) to riduce file size when using it.
// -s ASYNCIFY_IGNORE_INDIRECT=1 should reduce the overhead of -s ASYNCIFY=1

// SOKOL_AUDIO depends on:
//    Windows: WASAPI
//    macOS/iOS: CoreAudio
//    Linux: ALSA (-lasound and -lpthread)
//    emscripten: WebAudio + ScriptProcessorNode (doesn't use the emscripten-provided OpenAL or SDL Audio wrappers)
*/


#define SOKOL_IMPL
#include "sokol_audio.h"

// TWEAKABLE DEFINITIONS ================================================
//#define USE_SOKOL_AUDIO_PUSH_MODEL  // tweakable (this demo seems big, but it contains both implementations!)
#define NUM_SOUND_REPETITIONS   (2)   // tweakable
// ======================================================================

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
// 'sokol_audio.h' needs 'samplerate' and 'num_channels' in 'saudio_setup(...)': so they're
// global for every playback source. 'sndDecoder.h' can make some very basic conversions:
#ifndef SAMPLERATE
// Supported samplerates:
//#   define SAMPLERATE (22050)   // <- used when recording audio
# define SAMPLERATE (44100)
//# define SAMPLERATE (11025)
#endif //SAMPLERATE
#ifndef NUM_CHANNELS
// Supported num_channels:
#   define NUM_CHANNELS 1       // <- used when recording audio
//# define NUM_CHANNELS 2
#endif //NUM_CHANNELS


#define SNDDECODER_IMPLEMENTATION
#include "sndDecoder.h"     // sndd_DecodeSerializedSound(...)

#ifndef USE_SOKOL_AUDIO_PUSH_MODEL
float* gSamples = NULL;size_t gNumSamples = 0;
void my_stream_callback(float* buffer, int num_frames, int num_channels,void* used_data) {
    (void) used_data;
    static size_t samples_pos = 0,total_num_frames = 0;
    size_t buffer_pos = 0;
    num_frames*=num_channels;
    total_num_frames+=num_frames;
    while (samples_pos+num_frames>=gNumSamples) {
        memcpy(&buffer[buffer_pos],&gSamples[samples_pos],(gNumSamples-samples_pos)*sizeof(float));
        buffer_pos+=(gNumSamples-samples_pos);
        num_frames-=(gNumSamples-samples_pos);samples_pos=0;
    }
    if (total_num_frames>=NUM_SOUND_REPETITIONS*gNumSamples) return;
    if (num_frames>0) {
        SNDD_ASSERT(samples_pos+num_frames<gNumSamples);
        memcpy(&buffer[buffer_pos],&gSamples[samples_pos],num_frames*sizeof(float));
        buffer_pos+=num_frames;samples_pos+=num_frames;
        num_frames=0;
    }
}
#endif //USE_SOKOL_AUDIO_PUSH_MODEL

int main (int argc,char* argv[])    {
    // we'll use a single serialized sound, but we can easily combine more of them this way:
    const char* serializedSound[] =    {
#   include "./sounds/bells.inl"
/*            ,
#   include "./sounds/coin.inl"
              ,
#   include "./sounds/scream.inl"*/
    };
    float* samples = NULL;size_t samples_size = 0;
    size_t num_samples = sndd_DecodeSerializedSound(&samples,&samples_size,serializedSound[0],SAMPLERATE,NUM_CHANNELS);
    // 'samples_size' is the allocated size of the 'samples' array (in floats), but 'num_samples' is the real number of sound samples
    SNDD_ASSERT(samples && num_samples && samples_size);

    saudio_desc desc = {0};desc.sample_rate = SAMPLERATE;desc.num_channels=NUM_CHANNELS;
#   ifndef USE_SOKOL_AUDIO_PUSH_MODEL
    desc.stream_userdata_cb = my_stream_callback;
    gSamples = samples;gNumSamples = num_samples;
#   endif //USE_SOKOL_AUDIO_PUSH_MODEL
    // init sokol-audio
    saudio_setup(&desc);
    SNDD_ASSERT(saudio_channels() == 1 || saudio_channels() == 2);

#   ifdef USE_SOKOL_AUDIO_PUSH_MODEL
    {
        // push samples from main loop
        size_t samples_pos = 0,total_num_frames = 0;//size_t cnt = 0;
        const int saudio_expect_duration_ms = saudio_buffer_frames()*1000/desc.sample_rate; // ms needed to play saudio_expect() AFAICU
        while (1) {
            // generate and push audio samples...
            size_t num_frames = (size_t) saudio_expect()*desc.num_channels;
            //fprintf(stderr,"%lu) num_frames=%lu samples_pos=%lu/num_samples=%lu\n",cnt,num_frames,samples_pos,num_samples);++cnt;
            total_num_frames+=num_frames;
            while (samples_pos+num_frames>=num_samples) {
                saudio_push(&samples[samples_pos], num_samples-samples_pos);
                num_frames-=(num_samples-samples_pos);samples_pos=0;
            }
            if (total_num_frames>=NUM_SOUND_REPETITIONS*num_samples) break;
            if (num_frames>0) {
                SNDD_ASSERT(samples_pos+num_frames<num_samples);
                saudio_push(&samples[samples_pos], num_frames);
                samples_pos+=num_frames;
                num_frames=0;
            }

            // we should sleep a bit here... to wait for sokol_audio to consume the frame
            sleep_ms(saudio_expect_duration_ms);
        }
        sleep_ms(saudio_expect_duration_ms*5);  // we must wait a bit further for sokol_audio to play last audio frames
    }
#   else //USE_SOKOL_AUDIO_PUSH_MODEL
    {
        // callback model
        sleep_ms(NUM_SOUND_REPETITIONS*gNumSamples*1000/(desc.sample_rate*desc.num_channels));  // we must wait all the required time for the playback to end
    }
#   endif //USE_SOKOL_AUDIO_PUSH_MODEL

    // shutdown sokol-audio
    saudio_shutdown();
    // we must free 'samples'
    SNDD_FREE(samples);samples=NULL;samples_size=0;

    (void)argc;(void)argv;
    return 0;
}
