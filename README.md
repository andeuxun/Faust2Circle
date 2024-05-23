# Faust2Circle

The aim of this project was to create an architecture file able to input Faust dsp code in a circle environment using a custom architecture file.
<br><br>

## Current state of the project and usage

__Be warned :__ In its current state, the project is __not able__ to generate a fully operational kernel for the Raspberry PI and requires some tweaking.

The main problem when trying to test the project was its unability to compile using circle.

In the last iteration of the project, we were able to successfully compile a kernel by __totally removing the required libraries__ for the faust program, proving that the problem lies in the importation and usage of libraries.

We determined that __Faust__ tries to use multiple unessential standard __C++ libraries__ by __default__ when compiling any program with faust libraries usage. This is a problem because the circle compilator seems like it does not have any means of access to the mentioned standard libraries.

To bypass this problem, we need to create an expended.dsp file with the Faust compiler (see [next section](#secCompFaust)). <br>
This file will give us the __minimal faust libraries requirements__, and the related paths (These are the library_path0, library_path1, ... variables).

Now, we need to [compile](#secCompFaust) the expended.dsp file with the -fm "def" and our architecture file to generate a circle.cpp file with minimal default imports : ```<math.h> and #include "faust/dsp/fastmath.cpp"``` <br>

An __untested standard math.h__ file copy has been __provided__ in the __dev__ folder, and the fastmath.cpp file exists in the faust libraries (a copy is present in the faust/dsp/fastmath.cpp in the dev folder).

We then tried manually including the libraries in our circle Makefile and then compile, without success. The ```#include "faust/dsp/fastmath.cpp"``` line in our generated circle.cpp file still causes a problem when compiling with circle.

If we remove this include from the file, the project successfuly compiles but sends error messages related to the mydsp class and faust functions. There might be a need to manually implement the .lib files in this file.
<br><br>

## Project usage :

    This section explains how to build a kernel with this project.

<br>

### Compiling Faust dsp with the faust2circle.cpp architecture file :
---
<a name="secCompFaust"> </a>


1. Set up the [Faust compiler](https://github.com/grame-cncm/faust/wiki/BuildingSimple) on your computer

2. Compile your dsp file in a expended dsp file using :
```
faust -a arch/faust2circle.cpp youdspfile.dsp -e -o expended.dsp
```
3. Compile your dsp file in a .cpp file called "circle.cpp" with :
```
faust -fm "def" -a arch/faust2circle.cpp expended.dsp -o circle.cpp
```
<br>

### Compiling the circle project
---
<a name="secCompCircle"> </a>

1. Replace the current circle.cpp file in the dev folder with the one created above

2. Manually add the necessary faust libraries (found inside the expended.dsp file) to the makefile (and maybe the circle.cpp file, we haven't been able to test this but it seems plausible)

3. compile the file using circle using makeall ([official circle github](https://github.com/rsta2/circle))

4. Move the generated files to your Raspberry PI

5. Enjoy !<br> <br>


## Further informations about the project

* This project uses three different projects as a implementation base :<br>
    * ```faust2teensy``` from the Faust libraries
    * ```miniorgan``` from the Circle libraries
    * The simple kernel implementation example in this repository
    
    <br>
* The signal processing happens in the GetChunk function of the [```faust2circle.cpp```](\arch\faust2circle.cpp) file. This function currently does not take inputs into account, to implement this you will need to :
    * **Change** the CPWMSoundBaseDevice **input types** to float values between -1.0 and 1.0.

    * Put these **values** in the two **fInChannel[0][x] and fInChannel[1][x]**.

    <br>*Be mindful that **Circle** creates **one buffer** with **sample values** of multiple channels in **consecutive order** [CH1-1,CH2-1,CH1-2,CH2-2,...] when **faust** has **fInChannel[0]** for all **CH1** values and **fInChannel[1]** for all **CH2** values. This also means that **nChunkSize** for **Circle** is actually **two** times superior to the **Faust chunkSize** (Or x time superior for x input channels)* 
    <br><br>

*NB : I tried to implement this while writing this README file, there is a **commented Input managment** part in the **GetChunk** function of faust2circle.cpp, it should only be **missing the s16 to float conversion** to work properly, but keep in mind it **hasn't been tested at all***

<br>

---
### Best of luck and skill to the ones who will continue this project which should be near completion. It was a fun journey that I hope you will also enjoy.<br> <p align="right">- H.V. aka MadMuses </p>
---
