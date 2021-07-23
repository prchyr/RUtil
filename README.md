## RUtil

RUtil is a convience namespace for use with ROOT. There are several sub-namespaces for use with different things. One is an FFT namespace that uses the ROOT wrapper of FFTW to do FFTs and make spectrograms and stuff like that. another is an SVD namespace that does SVD type matrix operations and decompositions. It is pretty slow compared to more native implementations like using GSL or something but it works. most importantly, there are lots of utilities for analyzing RF waveforms, mostly using TGraphs. there are utilities for shifting, stretching, aligning, slicing, stacking, averaging TGraphs etc., which are useful for RF analysis. There are also some built in canvas and histogram title settings that make the default plots look less like garbage. 

some stuff might be broken and things might not work right and I add to this regularly. I will do my best to not break backward compatibility if i add stuff, but no promises. 


### installing

basic install, will put things in the appropriate places in /usr/local

```bash
cd /path/to/your/desired/source/dir
git clone https://github.com/prchyr/RUtil.git
cd RUtil
./install.sh
```
or, to install somewhere else

```bash
cd /path/to/your/desired/source/dir
git clone https://github.com/prchyr/RUtil.git
cd RUtil
export RUTIL_INSTALL_DIR=/path/to/your/desired/install/dir
./install.sh
```

### using

To use in your c++ programs, just do 

```c++
#include <RUtil.hh>
```

in your programs, and compile against the library with the flag

```c++
-lRUtil
```

To use inside of the ROOT interpreter (either command line or with macros) add these lines to your rootlogon.C:

```c++
gROOT->ProcessLine(".L RUTIL_INSTALL_DIR/lib/libRUtil.so");
gROOT->ProcessLine("#include <RUtil.hh>");
```

where you have replaced ```RUTIL_INSTALL_DIR``` with the correct path (for the default installation, it would be ```/usr/local/```. 


### documentation

currently, functions are just defined in the header. I'll try to add more.
