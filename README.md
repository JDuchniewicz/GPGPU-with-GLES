# GPGPU with GLES

A library for performing GPGPU (General Purpose GPU) computations on BeagleBone Black using OpenGL ES. 

_Originally concevied during Google Summer of Code 2021 for beagleboard.org_

Although this README contains most of the necessary details, for more in-depth descriptions and development diary please visit [the project blog](https://jduchniewicz.github.io/gsoc2021-blog/).

### **TL;DR:**

### _With this library you can accelerate your computations using built-in SGX GPU onboard the BBB. Beware! GPU <=> CPU transfers induce an overhead so choose your computations wisely!_

Motivation for the project was scarcity of heteregenousity on the BBB platform which means that most computations were done either on the CPU or in the PRU or both of them. Meanwhile GPU on the SoC was laying mostly untouched apart from some rare occasions where rendering was required - this is unacceptable and this project aims to change that!

Usually, one would simply use _compute shaders_ and just focus on writing efficient GPU computing code. However, we are limited with OpenGL ES version to 2.0 which does **not** support this kind of shaders. Therefore, we must do some _hacks_ and trick out GPU that we want to render something while we actually do just computations in our shaders.

The targetted chip is SGX540 (on the BBB) and SGX544 (on the BBAI - TODO: yet untested).

Support for other platforms should follow quite easily (BBB was target of this project therefore it is the preferred platform), assuming the system has some OpenGL ES and EGL libraries preinstalled and has the appropriate video devices (_/dev/dri/render*_).

**Caveat**:

For the time being you require a headless dummy plug (similar to [this one]()) to simulate having a display device connected. This is being worked on with Imagination engineers [here]().

## Preparing the environment
### As mentioned earlier, this will guide you through the BBB-specific steps (stating which steps are common).
#### Download and flash the image
In order to prepare your environment, you first need to install the BBB [image](https://debian.beagleboard.org/images/bone-debian-9.12-imgtec-armhf-2020-04-06-4gb.img.xz) containing the necessary libraries. Flashing is described in detail [here](https://beagleboard.org/getting-started). 

You can also follow the steps listed [here](https://elinux.org/Beagleboard:BeagleBoneBlack_Debian#Flashing_eMMC) to move the system to on-board eMMC.

#### Install prerequisites
`sudo apt update && sudo apt install cmake`

#### Clone and build the repository
Clone the repository:
`git clone https://github.com/JDuchniewicz/GPGPU-with-GLES`

Create the build folder and enter it:
`mkdir GPGPU-with-GLES/lib/build && cd GPGPU-with-GLES/lib/build`

Run the cmake command:
`cmake ..`

Finally, build it:
`make`

## Examples

## Benchmarks
Refer to benchmarks to assess whether your
