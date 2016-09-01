Install portaudio, apply patch from C++ example

Then run the following
``` bash
swig -c++ -javascript -node -DV8_VERSION=0x0478025 -I../../ snowboy-detect-swig.i
```