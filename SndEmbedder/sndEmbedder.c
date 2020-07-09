// https://github.com/Flix01/Flix-Tools

// COMPILATION INSTRUCTIONS
/*
// with gcc:
gcc -O3 -no-pie -fno-pie sndEmbedder.c -o sndEmbedder -lopenal -lm
// with clang:
clang -O3 -no-pie -fno-pie sndEmbedder.c -o sndEmbedder -lopenal -lm
// with mingw (here for Windows 64bit):
x86_64-w64-mingw32-gcc -O3 -no-pie -fno-pie -mconsole sndEmbedder.c -o sndEmbedder.exe -DWINVER=0x0800 -D_WIN32 -D_WIN64 -luser32 -lkernel32 -lOpenAL32
// with cl (here for Windows 32bit, VC 7.1 2003: hence /DSND_NO_C99_MATH_FUNCTIONS)
cl /TC /O2 /ML /DSND_NO_C99_MATH_FUNCTIONS sndEmbedder.c /link /out:sndEmbedder.exe Shell32.lib user32.lib kernel32.lib OpenAL32.lib

// emscripten untested (probably getch() and kbhit() are not supported)
// (but both tests can run with emscripten)

// NOTES ON COMMANDLINE OPTIONS:
// -no-pie -fno-pie just make sure than the compiled source considered an .exe (and not a .dll)
// -fopenmp-simd -DNDEBUG can be used to increase performace (-fopenmp-simd does NOT need openmp!).
// -DNDEBUG should remove all the asserts, so it's probably better to avoid it when performance is not a priority.
// -std=gnu89 -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-function -Werror=declaration-after-statement  can be used to restric the C version

// Windows build does not run correctly on Linux though wine (user can't interact and choose any option)
*/

//#define ADD_DEBUG_OPTIONS // optional (to save as .wav or as .bin)


#include <math.h>
#include <stdio.h>


#ifdef _WIN32
#   include <conio.h>
#   include <windows.h>    // Sleep()
#   define sleep_ms(X)     Sleep((X))
#   include <stdlib.h>
#   define clrscr()        system("cls")    // Not sure if this is required [system("clear") ???]
#else // tested only on Linux
#   define clrscr() printf("\e[1;1H\e[2J")
#   include <unistd.h>     // usleep and other stuff
#   include <termios.h>
char getch(void)    {
    /* https://stackoverflow.com/questions/7469139/what-is-the-equivalent-to-getch-getche-in-linux */
    char buf = 0;
    struct termios old = {0};
    fflush(stdout);
    if(tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if(tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if(read(0, &buf, 1) < 0)
        perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if(tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    //printf("%c\n", buf);
    return buf;
}
#   include <sys/ioctl.h>
int kbhit() {
    /* https://stackoverflow.com/questions/29335758/using-kbhit-and-getch-on-linux?rq=1 */
    struct termios term,term2;int byteswaiting;
    tcgetattr(0, &term);

    term2 = term;
    term2.c_lflag &= ~ICANON;
    tcsetattr(0, TCSANOW, &term2);

    ioctl(0, FIONREAD, &byteswaiting);

    tcsetattr(0, TCSANOW, &term);

    return (byteswaiting > 0) ? 1 : 0;
}
#   ifdef __EMSCRIPTEN__
#      include <emscripten.h>
#      define sleep_ms(X)      emscripten_sleep(X)     // needs -s ASYNCIFY=1 [simply using usleep(...) compiles but does not work correctly]
#   else //__EMSCRIPTEN__
#       define sleep_ms(X)     usleep((X)*1000)
#   endif //__EMSCRIPTEN__
#endif // _WIN32


__inline static char async_getch(void)  {return kbhit() ? getch() : 0;}


#define SND_SYSTEM_IMPLEMENTATION
#include "snd_system.h"


/* stdio

   printf()      Print a formatted string to stdout.
   scanf()       Read formatted data from stdin.
   putchar()     Print a single character to stdout.
   getchar()     Read a single character from stdin.
   puts()        Print a string to stdout.
   gets()        Read a line from stdin.

    There exists a function "kbhit()" which only checks the input-buffer:
    if (kbhit()) ta=getchar();
    Borland provides getch(), getche(), and kbhit() which work on DOS &
    OS/2 character mode.

    Outputting the backspace character '\b' may help to move the output point back (like left arrow).
    Specifically, outputting the string "\b \b" should blank out the last character output.
    putch('\b');putch(' ');putch('\b');
*/

static const char* getDisplayValueString(snoat value)  {
    static char displayer[61];
    snoat absValue = snd_fabs(value);
    unsigned char valueIn0_60,i;
    //SND_ASSERT(absValue>=0);
    if (absValue > (snoat)1) absValue = (snoat)1;
    valueIn0_60 = (unsigned char) (absValue*(snoat)60);
    //SND_ASSERT(valueIn0_60<=60);
    memset(displayer,' ',60);
    for (i=0;i<60;i++) {
        if (valueIn0_60>i)  displayer[i] = '=';
        else break;
    }
    displayer[60]='\0';
    return displayer;
}
static __inline void displayValue(snoat value)  {printf("%s",getDisplayValueString(value));}
static __inline void displayValueUpdated(snoat value)  {
    printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b"
           "                                                            "
           "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b"
           "%s",getDisplayValueString(value));
}





int main(int argc,char* argv[]) {
    struct sndcontext_t* c = sndcontext_create();
#   ifndef CAPTURE_BUFFER_SIZE
#       define CAPTURE_BUFFER_SIZE (1<<17)   // Example: (4096)/22050 = 0,185759637 s
#   endif
#   define CAPTURE_BUFFER_DURATION_MS     (1000*CAPTURE_BUFFER_SIZE/SND_SAMPLERATE)
    snoat captureBuffer[CAPTURE_BUFFER_SIZE] = SND_ZERO_INIT;
    const snoat* captureBufferLastSample = &captureBuffer[CAPTURE_BUFFER_SIZE-1];
    snoat captureBufferReconstructed[CAPTURE_BUFFER_SIZE] = SND_ZERO_INIT;
    int sleepAmountMs = CAPTURE_BUFFER_DURATION_MS/10;
    SND_ASSERT(CAPTURE_BUFFER_SIZE>1);
    if (sleepAmountMs<1) sleepAmountMs=1;
    else if (sleepAmountMs>50) sleepAmountMs=50;

#   ifndef LOG2_CAPTURE_BUFFER_SIZE_NEXT_POT
#       define LOG2_CAPTURE_BUFFER_SIZE_NEXT_POT (17)
#   endif
#   if (1<<(LOG2_CAPTURE_BUFFER_SIZE_NEXT_POT-1))>=CAPTURE_BUFFER_SIZE
#      error Please define LOG2_CAPTURE_BUFFER_SIZE_NEXT_POT correctly... is too big
#   endif

#   define CAPTURE_BUFFER_SIZE_NEXT_POT (1<<LOG2_CAPTURE_BUFFER_SIZE_NEXT_POT)
#   if CAPTURE_BUFFER_SIZE_NEXT_POT<CAPTURE_BUFFER_SIZE
#      error Please define LOG2_CAPTURE_BUFFER_SIZE_NEXT_POT correctly
#   endif


    do {
        char ch;int captureDeviceIndex=0,playbackDeviceIndex=0;
        int captureDeviceStarted = sndcontext_isOpenCaptureDeviceStarted(c);
        int mustRedisplay = 0;
        clrscr();
        //printf("SLEEP_AMOUNT_MS = %d\n",sleepAmountMs);
        printf("===============\n");
        printf("(A) SNDEMBEDDER\n");
        printf("===============\n");
        printf("(1) Change %s Capture  Device: \"%s\"\n",captureDeviceStarted?"ACTIVE " : "Current",sndcontext_getOpenCaptureDeviceName(c,&captureDeviceIndex));
        printf("(2) EMBED captured sound (last %d ms)\n",CAPTURE_BUFFER_DURATION_MS);
        printf("(ESC) or any other key to QUIT.\n");

        // This is simply a 'ch=async_getch()' that updates the capture buffer and displays a simple volume meter
        sndcontext_updateCaptureBuffer(c,captureBuffer,CAPTURE_BUFFER_SIZE);
        printf("\n");displayValue(*captureBufferLastSample);
        while ((ch=async_getch())=='\0') {
            int num_new_samples;
            sleep_ms(sleepAmountMs);  // well, it might depend on CPU too
            num_new_samples = sndcontext_updateCaptureBuffer(c,captureBuffer,CAPTURE_BUFFER_SIZE);
            if (num_new_samples>0)  {
                displayValueUpdated(*captureBufferLastSample);
                fflush(stdout);
                //SND_ASSERT(num_new_samples<=CAPTURE_BUFFER_SIZE);   // But not necessary for this app
                SND_ASSERT(sndcontext_isOpenPlaybackDeviceStarted(c)==0);    // Dbg
            }
        }
        printf("\n");


        if (ch=='1')   {
            mustRedisplay = 1;
            do {
                int count,i,currentIndex;size_t len;
                const char* names = NULL;char ch2 = '\0';
                clrscr();
                printf("==========================\n");
                printf("(A1) CHOOSE CAPTURE DEVICE\n");
                currentIndex = captureDeviceIndex;
                names = sndcontext_getCaptureDeviceNames(c,0,&count);
                printf("==========================\n");
                for (i=0;i<count;i++)   {
                    printf("(%d) [%s] \"%s\"\n",i+1,i==currentIndex?"x":" ",names);
                    len = strlen(names);
                    names = &names[len+1];
                }
                printf("(ESC) or any other key to go BACK TO MENU A.\n");
                ch2 = getch();
                if (ch2>='1' && ch2<'1'+count)    {
                    currentIndex =  (int)ch2-'1';
                    SND_ASSERT(currentIndex>=0 && currentIndex<count);
                    captureDeviceIndex = currentIndex;
                    sndcontext_setOpenCaptureDevice(c,currentIndex);
                    continue;
                }
                else break;
                //if (ch2==(char)27) break;
            } while (1);
        }
        else if (ch=='2')   {
            int mustRedisplay2 = 0;
            char ch3 = '\0';int i;
            int playbackDeviceStarted = 0;
            int startCaptureSample,endCaptureSample,detectedSoundDurationMs,captureSampleLength;
#           ifndef SND_SILENCE_THRESHOLD
#               define SND_SILENCE_THRESHOLD (0.05)
#           endif
            const snoat eps = SND_SILENCE_THRESHOLD;

            mustRedisplay = 1;
            if (sndcontext_isOpenCaptureDeviceStarted(c))   sndcontext_stopOpenCaptureDevice(c);
            if (!sndcontext_isOpenPlaybackDeviceStarted(c)) sndcontext_startOpenPlaybackDevice(c);
            playbackDeviceStarted = sndcontext_isOpenPlaybackDeviceStarted(c);

            //--- Extract valid sound chunk (trimming silence) ----
            startCaptureSample=0;endCaptureSample=CAPTURE_BUFFER_SIZE;detectedSoundDurationMs = CAPTURE_BUFFER_DURATION_MS;
            captureSampleLength = (endCaptureSample-startCaptureSample);

            for (i=0;i<CAPTURE_BUFFER_SIZE;i++) {
                const snoat s = captureBuffer[i];
                if (s<eps && s>=-eps) startCaptureSample=i;
                else break;
            }
            for (i=CAPTURE_BUFFER_SIZE-1;i>=0;--i) {
                const snoat s = captureBuffer[i];
                if (s<eps && s>=-eps) endCaptureSample=i+1;
                else break;
            }
            if (startCaptureSample>=endCaptureSample)   {startCaptureSample=0;endCaptureSample=CAPTURE_BUFFER_SIZE;}
            captureSampleLength = (endCaptureSample-startCaptureSample);
            detectedSoundDurationMs = (1000*captureSampleLength/SND_SAMPLERATE);
            //-----------------------------------------------------

            do  {
                const int sleepAmountForPlayback = sleepAmountMs;//(sleepAmountMs+100)/2;   // 100 ?
                mustRedisplay2 = 0;
                clrscr();
                printf("===============================\n");
                printf("(B) UNCOMPRESSED CAPTURED SOUND\n");
                printf("===============================\n");
                printf("Detected sound duration: %d ms\n",detectedSoundDurationMs);
                printf("(1) Change %s Playback Device: \"%s\"\n",playbackDeviceStarted?"ACTIVE " : "Current",sndcontext_getOpenPlaybackDeviceName(c,&playbackDeviceIndex));
                printf("(2) CONFIRM that you want to compress and serialize this sound\n");
#               ifdef ADD_DEBUG_OPTIONS
                printf("(3) DEBUG: Save this uncompressed sound as .WAV FILE\n");
#               endif
                printf("(ESC) or any other key to go BACK TO MENU A.\n");

                sndcontext_feedPlaybackData(c,&captureBuffer[startCaptureSample],captureSampleLength*sizeof(snoat));
                sndcontext_playbackPlay(c);

                // This is simply a 'ch=async_getch()' that repeats the playback of the capture buffer and displays a simple volume meter
                printf("\n");displayValue((snoat)0);
                while ((ch3=async_getch())=='\0') {
                    int pos;
                    sleep_ms(sleepAmountForPlayback);    // ?
                    if (sndcontext_getPlaybackState(c)!=SND_PLAYBACK_PLAYING) sndcontext_playbackPlay(c);
                    pos = sndcontext_getPlaybackPosition(c)+startCaptureSample;
                    SND_ASSERT(pos>=0 && pos<CAPTURE_BUFFER_SIZE);
                    displayValueUpdated(captureBuffer[pos]);
                    fflush(stdout);
                    SND_ASSERT(sndcontext_isOpenCaptureDeviceStarted(c)==0);    // Dbg
                };
                printf("\n");

                if (ch3=='1')   {
                    mustRedisplay2 = 1;
                    do {
                        int count,i,currentIndex;size_t len;
                        const char* names = NULL;char ch2 = '\0';
                        clrscr();
                        printf("===========================\n");
                        printf("(B1) CHOOSE PLAYBACK DEVICE\n");
                        currentIndex = playbackDeviceIndex;
                        names = sndcontext_getPlaybackDeviceNames(c,0,&count);
                        printf("===========================\n");
                        for (i=0;i<count;i++)   {
                            printf("(%d) [%s] \"%s\"\n",i+1,i==currentIndex?"x":" ",names);
                            len = strlen(names);
                            names = &names[len+1];
                        }
                        printf("(ESC) or any other key to go BACK TO MENU B.\n");
                        ch2 = getch();
                        if (ch2>='1' && ch2<'1'+count)    {
                            currentIndex =  (int)ch2-'1';
                            SND_ASSERT(currentIndex>=0 && currentIndex<count);
                            playbackDeviceIndex = currentIndex;
                            sndcontext_setOpenPlaybackDevice(c,currentIndex);
                            playbackDeviceStarted = sndcontext_isOpenPlaybackDeviceStarted(c);
                            SND_ASSERT(playbackDeviceStarted);
                            continue;
                        }
                        else break;
                        //if (ch2==(char)27) break;
                    } while (1);
                }
                else if (ch3=='2')   {
                    static snoat dft_input[CAPTURE_BUFFER_SIZE_NEXT_POT][2],dft_output[CAPTURE_BUFFER_SIZE_NEXT_POT][2],dft_output2[CAPTURE_BUFFER_SIZE_NEXT_POT][2];
                    const int dft_size_pot = (int) snd_calculate_next_power_of_two((unsigned)captureSampleLength);
                    int dft_log2size = 0;
                    int i;
                    static sndDFTCompressionParams_t params = SNDDFTCOMPRESSIONPARAMS_DEFAULT_INIT;
                    size_t serializeBufferSize=0,check_size=0;
                    unsigned char* serializeBuffer = NULL;

                    int mustRedisplay3 = 0;
                    char ch4 = '\0';
                    mustRedisplay2 = 1;  // if we must go back to 'Embed Captured Sound'
                    SND_ASSERT(playbackDeviceStarted);
                    sndcontext_playbackStop(c);
                    // =========================================================================
                    // Here we calculate the DFT Coefficients
                    while ((1<<dft_log2size)<dft_size_pot) ++dft_log2size;
                    SND_ASSERT(1<<dft_log2size==dft_size_pot);

                    // DFT:
                    for (i=0;i<captureSampleLength;i++)    {dft_input[i][0]=captureBuffer[i+startCaptureSample];dft_input[i][1]=0;}  // fill dft_input
                    for (i=captureSampleLength;i<dft_size_pot;i++)    {dft_input[i][0]=dft_input[i][1]=0;}  // Pad the rest with zero
                    snd_DiscreteFourierTransform(dft_output,dft_input,dft_size_pot,dft_log2size,0,0);

                    // DFT serialize

                    serializeBufferSize = 0;
                    serializeBuffer = snd_SerializeDiscreteFourierTransform(&serializeBufferSize,captureSampleLength,dft_output,&params);
                    SND_ASSERT(serializeBuffer && serializeBufferSize>0);
                    //------------------
                    check_size = snd_DeserializeDiscreteFourierTransform(dft_output2,dft_size_pot,serializeBuffer,serializeBufferSize);

                    SND_ASSERT(check_size==(size_t)dft_size_pot);
                    SND_FREE(serializeBuffer);serializeBuffer=NULL;

                    // IDFT:
                    snd_DiscreteFourierTransform(dft_input,dft_output2,dft_size_pot,dft_log2size,1,0);          // Note that output 'dft_input' should be real (we don't check)
                    for (i=0;i<captureSampleLength;i++)    {captureBufferReconstructed[i] = dft_input[i][0];}  // fill captureBufferReconstructed

                    //-------------------------------------------------------------------------------
                    do  {
                        mustRedisplay3 = 0;
                        clrscr();
                        //printf("dft_log2size: %d     dft_size_pot = %d    serializeBufferSize = %lu\n",dft_log2size,dft_size_pot,serializeBufferSize);
                        printf("=================================\n");
                        printf("(C) PRELISTEN TO COMPRESSED SOUND\n");
                        printf("=================================\n");
                        printf("Sound duration: %d ms (%d samples)\n",detectedSoundDurationMs,captureSampleLength);
                        printf("Raw size: %s\n",snd_GetFileSizeString(serializeBufferSize));
                        printf("(1) Tune Parameters: {HF_trim:%u;F_trim=%u;QZ_enhance:%u;QZ_worsen:%u}\n",(unsigned)(params.high_frequency_trimmer*1000.f),(unsigned)(params.regular_frequency_trimmer*1000.f),(unsigned)(params.quantization_enhancer*100.f),(unsigned)(params.quantization_worsening*1000.f));
                        printf("(2) SERIALIZE to header file\n");
#                       ifdef ADD_DEBUG_OPTIONS
                        printf("(3) DEBUG: Serialize this compressed sound as raw .bin file\n");
                        printf("(4) DEBUG: Save this compressed sound as .wav file\n");
#                       endif
                        printf("(ESC) or any other key to go BACK TO MENU B.\n");

                        //sndcontext_feedPlaybackData(c,&captureBuffer[startCaptureSample],captureSampleLength*sizeof(snoat));
                        sndcontext_feedPlaybackData(c,&captureBufferReconstructed[0],captureSampleLength*sizeof(snoat));
                        sndcontext_playbackPlay(c);

                        // This is simply a 'ch=async_getch()' that repeats the playback of the capture buffer and displays a simple volume meter
                        printf("\n");displayValue((snoat)0);
                        while ((ch4=async_getch())=='\0') {
                            int pos;
                            sleep_ms(sleepAmountForPlayback);    // ?
                            if (sndcontext_getPlaybackState(c)!=SND_PLAYBACK_PLAYING) sndcontext_playbackPlay(c);
                            pos = sndcontext_getPlaybackPosition(c);
                            SND_ASSERT(pos>=0 && pos<captureSampleLength);
                            //displayValueUpdated(captureBuffer[pos+startCaptureSample]);
                            displayValueUpdated(captureBufferReconstructed[pos]);
                            fflush(stdout);
                        };
                        printf("\n");

                        if (ch4=='1')   {                            
                            static int tune_choice = 1;
                            int mustRedisplay4 = 0, mustUpdatePlayBack = 0;
                            static const char paramNamesTenChars[4][11] = {"HF_trim   ","F_trim    ","QZ_enhance","QZ_worsen "};
                            char ch5 = '\0';
                            mustRedisplay3 = 1;
                            do  {
                                snoat* v;unsigned vu;
                                mustRedisplay4 = mustUpdatePlayBack = 0;
                                clrscr();
                                printf("==============================\n");
                                printf("(C1) TUNE SERIALIZATION PARAMS\n");
                                printf("==============================\n");
                                printf("Sound duration: %d ms (%d samples)\n",detectedSoundDurationMs,captureSampleLength);
                                printf("Raw size: %s\n",snd_GetFileSizeString(serializeBufferSize));
                                if (tune_choice==1) printf("(1) [x] TUNING \"HF_trim\" (high-frequency trimming) in [0,1000]\n");
                                else                printf("(1) [ ] tune   \"HF_trim = %u\" (high-frequency trimming) in [0,1000]\n",(unsigned)(params.high_frequency_trimmer*1000.f));
                                if (tune_choice==2) printf("(2) [x] TUNING \" F_trim\" (regular-frequency trimming) in [0,%u]\n",(unsigned)(params.high_frequency_trimmer*1000.f));
                                else                printf("(2) [ ] tune   \" F_trim = %u\" (regular-frequency trimming) in [0,\"HF_trim\"]\n",(unsigned)(params.regular_frequency_trimmer*1000.f));
                                if (tune_choice==3) printf("(3) [x] TUNING \"QZ_enhance\" (quantization enhancing) in [0,...]\n");
                                else                printf("(3) [ ] tune   \"QZ_enhance = %u\" (quantization enhancing) in [0,...]\n",(unsigned)(params.quantization_enhancer*100.f));
                                if (tune_choice==4) printf("(4) [x] TUNING \"QZ_worsen\" (quantization worsening) in [0,1000]\n");
                                else                printf("(4) [ ] tune   \"QZ_worsen = %u\" (quantization worsening) in [0,1000]\n",(unsigned)(params.quantization_worsening*1000.f));
                                printf("(LEFT) (RIGHT) Tune param with step  1\n");
                                printf("(DOWN)  (UP)   Tune param with step  5\n");
                                printf(" (+)    (-)    Tune param with step 10\n");
                                printf("(D) Reset all params to DEFAULT\n");
                                printf("(ESC) or any other key to go BACK TO MENU C.\n");

                                switch (tune_choice)    {
                                case 1:{v = &params.high_frequency_trimmer;vu = (unsigned)((*v)*(snoat)1000);}break;
                                case 2:{v = &params.regular_frequency_trimmer;vu = (unsigned)((*v)*(snoat)1000);}break;
                                case 3:{v = &params.quantization_enhancer;vu = (unsigned)((*v)*(snoat)100);}break;
                                case 4:{v = &params.quantization_worsening;vu = (unsigned)((*v)*(snoat)1000);}break;
                                default:SND_ASSERT(0);break;
                                }


                                // This is simply a 'ch=async_getch()' that repeats the playback of the capture buffer
                                printf("\n");printf("\"%s\": %.5u",&paramNamesTenChars[tune_choice][0],vu);fflush(stdout);
                                while (1)   {
                                    static char last_adder = 0;
                                    int adder = 0;  // a positive or negative number if user tunes the value
                                    ch5=async_getch();
                                    if (ch5=='+')       adder=10;       // plus key
                                    else if (ch5=='-')  adder=-10;      // minus key
                                    if (ch5 == '\033' && async_getch()=='[') {
                                        switch(async_getch()) {
                                            case 'A':adder =  5;break;   // arrow up
                                            case 'B':adder = -5;break;   // arrow down
                                            case 'C':adder =  1;break;   // arrow right
                                            case 'D':adder = -1;break;   // arrow left
                                        }
                                    }
                                    if (adder!=0)   {
                                        if (tune_choice!=3) {
                                            if (adder>0) vu = (vu+adder<1000)?(vu+adder):1000;
                                            else         vu = (vu>(unsigned)(-adder))?(vu+adder):0;
                                            *v = ((snoat) vu)/(snoat)1000;
                                        }
                                        else {
                                            if (adder>0) vu = (vu+adder<10000)?(vu+adder):10000;
                                            else         vu = (vu>(unsigned)(-adder))?(vu+adder):0;
                                            *v = ((snoat) vu)/(snoat)100;
                                        }
                                    }
                                    if (adder!=0) mustUpdatePlayBack = 0;
                                    if (ch5=='\0' && mustUpdatePlayBack) ++mustUpdatePlayBack;
                                    if (last_adder!=adder)    {
                                        if (adder==0 && last_adder!=0)   {
                                            ++mustUpdatePlayBack;
                                        }
                                        //-----------
                                        last_adder = adder;
                                    }

                                    if (ch5=='\0' || adder!=0) {
                                        int pos;
                                        if (ch5=='\0') sleep_ms(sleepAmountForPlayback);    // ?
                                        if (sndcontext_getPlaybackState(c)!=SND_PLAYBACK_PLAYING) sndcontext_playbackPlay(c);
                                        pos = sndcontext_getPlaybackPosition(c);
                                        SND_ASSERT(pos>=0 && pos<captureSampleLength);
                                        if (ch5 == '\033')  {
                                            printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b"
                                                   "                       "
                                                   "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b"
                                                   "\"%s\": %.5u",&paramNamesTenChars[tune_choice-1][0],vu);
                                        }
                                        else {
                                            printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b"
                                                   "                    "
                                                   "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b"
                                                   "\"%s\": %.5u",&paramNamesTenChars[tune_choice-1][0],vu);
                                        }
                                        fflush(stdout);
                                    }
                                    else break;
                                    if (mustUpdatePlayBack>=20) {                                        
                                        mustUpdatePlayBack = 1;
                                        mustRedisplay4 = 1;
                                        break;
                                    }

                                }
                                printf("\n");

                                if (ch5>='1' && ch5<='4')   {
                                    mustRedisplay4 = 1;
                                    if (tune_choice!=(int)(ch5-'0')) {
                                        tune_choice=(int)(ch5-'0');
                                        mustUpdatePlayBack = 1; // (not necessary...)
                                    }
                                    SND_ASSERT(tune_choice>0 && tune_choice<=4);
                                }
                                else if (ch5=='d')  {
                                    const sndDFTCompressionParams_t tmp = SNDDFTCOMPRESSIONPARAMS_DEFAULT_INIT;
                                    mustRedisplay4 = 1;
                                    params = tmp;
                                    mustUpdatePlayBack = 1;
                                }

                                if (mustUpdatePlayBack) {
                                    // update playback (better refactor)
                                    sndcontext_playbackStop(c);
                                    SND_ASSERT(serializeBuffer==NULL);
                                    serializeBuffer = snd_SerializeDiscreteFourierTransform(&serializeBufferSize,captureSampleLength,dft_output,&params);
                                    SND_ASSERT(serializeBuffer && serializeBufferSize>0);
                                    check_size = snd_DeserializeDiscreteFourierTransform(dft_output2,dft_size_pot,serializeBuffer,serializeBufferSize);
                                    SND_ASSERT(check_size==(size_t)dft_size_pot);
                                    SND_FREE(serializeBuffer);serializeBuffer=NULL;
                                    snd_DiscreteFourierTransform(dft_input,dft_output2,dft_size_pot,dft_log2size,1,0);          // Note that output 'dft_input' should be real (we don't check)
                                    for (i=0;i<captureSampleLength;i++)    {captureBufferReconstructed[i] = dft_input[i][0];}  // fill captureBufferReconstructed
                                    sndcontext_feedPlaybackData(c,&captureBufferReconstructed[0],captureSampleLength*sizeof(snoat));
                                    sndcontext_playbackPlay(c);
                                    // ------------------------------
                                }

                                if (!mustRedisplay4) {
                                    break;
                                }
                            }
                            while (1);
                        }
                        else if (ch4=='2'
#                           ifdef ADD_DEBUG_OPTIONS
                                 || ch4=='3' || ch4=='4'
#                           endif
                                 )
                        {
                            char filename[2048]="";size_t len = 0;
                            mustRedisplay3 = 1;

                            printf("\n\nPlease enter file name (e.g. mysound): ");fflush(stdout);
                            scanf("%s",filename);
                            len = strlen(filename);
                            //if (len==0) strcpy(filename,"mysound");
                            if (len>0)  {
                                if (ch4=='2' || ch4=='3')   {
                                    SND_ASSERT(!serializeBuffer);
                                    serializeBufferSize = 0;
                                    serializeBuffer = snd_SerializeDiscreteFourierTransform(&serializeBufferSize,captureSampleLength,dft_output,&params);
                                    SND_ASSERT(serializeBuffer && serializeBufferSize>0);
                                    if (ch4=='2')   {
                                        char* buff = NULL; size_t buff_size=0,size=0;FILE* f = NULL;
                                        strcat(filename,".inl");
                                        size = snd_Base85Encode((const char*)serializeBuffer,serializeBufferSize,&buff,&buff_size,1,280,1);
                                        SND_ASSERT(buff);
                                        f = fopen(filename,"wt");
                                        if (f)  {
                                            fprintf(f,"\t// num_samples = %d, saved with samplerate = %d and num_channels = %d (length in ms: %d)\n",captureSampleLength,SND_SAMPLERATE,1,detectedSoundDurationMs);
                                            fwrite(buff,size,1,f);
                                            fclose(f);
                                            printf("\nFile: \"%s\" saved successfully.\n",filename);
                                        }
                                        SND_FREE(buff);buff_size=0;
                                    }
                                    else if (ch4=='3')  {
                                        FILE* f = NULL;
                                        strcat(filename,".bin");
                                        f = fopen(filename,"wt");
                                        if (f)  {
                                            fwrite(serializeBuffer,serializeBufferSize,1,f);
                                            fclose(f);
                                            printf("\nFile: \"%s\" saved successfully.\n",filename);
                                        }
                                    }
                                    SND_FREE(serializeBuffer);serializeBuffer=NULL;
                                }
                                else if (ch4=='4')  {
                                    strcat(filename,".wav");
                                    if (snd_SaveAsWavFile(filename,&captureBufferReconstructed[0],captureSampleLength,SND_SAMPLERATE,1)>0)
                                        printf("\nFile: \"%s\" saved successfully.\n",filename);
                                }
                                else {SND_ASSERT(0);}
                                printf("Press a key to continue and go back.");
                                getch();
                            }
                        }

                        if (!mustRedisplay3) {
                            sndcontext_playbackStop(c);
                            break;
                        }
                    } while (1);
                    // ==============================================================================
                }
#               ifdef ADD_DEBUG_OPTIONS
                else if (ch3=='3')  {
                    char filename[2048]="";size_t len = 0;
                    mustRedisplay2 = 1;

                    printf("\n\nPlease enter file name (e.g. mysound): ");fflush(stdout);
                    scanf("%s",filename);
                    len = strlen(filename);
                    //if (len==0) strcpy(filename,"mysound");
                    if (len>0)  {
                        strcat(filename,".wav");
                        if (snd_SaveAsWavFile(filename,&captureBuffer[startCaptureSample],captureSampleLength,SND_SAMPLERATE,1)>0)
                            printf("\nFile: \"%s\" saved successfully.\n",filename);
                    }
                    printf("Press a key to continue and go back.");
                    getch();
                }
#               endif

                if (!mustRedisplay2)    {
                    sndcontext_playbackStop(c);
                    if (sndcontext_isOpenPlaybackDeviceStarted(c))  sndcontext_stopOpenPlaybackDevice(c);
                    if (!sndcontext_isOpenCaptureDeviceStarted(c))  sndcontext_startOpenCaptureDevice(c);
                    sndcontext_updateCaptureBuffer(c,captureBuffer,CAPTURE_BUFFER_SIZE);    // flush capture buffer
                    memset(captureBuffer,0,CAPTURE_BUFFER_SIZE*sizeof(snoat));  // reset capture buffer
                    break;
                }
            }
            while (1);
        }

        //-------------------------
        if (!mustRedisplay) break;
    }
    while (1);

    fflush(stdout);
    sndcontext_destroy(c);c=NULL;
    return 0;
}

