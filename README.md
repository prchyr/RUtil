### RUtil

RUtil is a convience namespace for use with ROOT. There are several sub-namespaces for use with different things. One is an FFT namespace that uses the ROOT wrapper of FFTW to do FFTs and make spectrograms and stuff like that. another is an SVD namespace that does SVD type matrix operations and decompositions. It is pretty slow compared to more native implementations like using GSL or something but it works. most importantly, there are lots of utilities for analyzing RF waveforms, mostly using TGraphs. there are utilities for shifting, stretching, aligning, slicing, stacking, averaging TGraphs etc., which are useful for RF analysis. There are also some built in canvas and histogram title settings that make the default plots look less like garbage. 

some stuff might be broken and things might not work right and I add to this regularly. I will do my best to not break backward compatibility if i add stuff, but no promises. 
