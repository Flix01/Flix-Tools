# sndEmbedder
sndEmbedder is the command-line tool I use to embed **short mono audio sounds (max about 5 seconds)** into a C/C++ file.


# HOW TO COMPILE
**sndEmbedder.c** is a single C89 file that can be easily compiled this way:

```gcc -O3 -no-pie -fno-pie sndEmbedder.c -o sndEmbedder -lopenal -lm```

(or in a similar way using other compilers: compilation instructions are at the top of [**sndEmbedder.c**](./sndEmbedder.c))

## DEPENDENCIES
* **sndEmbedder.c** requires a version of openal with float samples support (**AL_FORMAT_MONO_FLOAT32**)

# GETTING STARTED
**sndEmbedder** is an interactive console application.

It can capture sound from any supported capture device (e.g. microphone or any PC output, if an audio monitor capture device has been correctly configured).

At the end of the process it should create an **.inl** file (like [bells.inl](./Tests/sounds/bells.inl)) that can be included into user code.

# HOW TO USE .INL FILES
In the **Test** subfolder there are two demos:

* [**test_openal.c**](./Tests/test_openal.c) (depends on openal)
* [**test_sokol.c**](./Tests/test_sokol.c) (depends on -lasound on Linux)

Compilation instructions for Linux, Windows and Emscripten are at the top of each .c file.


