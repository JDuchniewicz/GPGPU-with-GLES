# GPGPU with GLES

A library for performing GPGPU (General Purpose GPU) computations on BeagleBone Black using OpenGL ES. 

_Originally concevied during Google Summer of Code 2021 for beagleboard.org_

Although this README contains most of the necessary details, for more in-depth descriptions and development diary please visit [the project blog](https://jduchniewicz.github.io/gsoc2021-blog/).

Motivation for the project was scarcity of heteregenousity on the BBB platform which means that most computations were done either on the CPU or in the PRU or both of them. Meanwhile GPU on the SoC was laying mostly untouched apart from some rare occasions where rendering was required - this is unacceptable and this project aims to change that!

Usually, one would simply use _compute shaders_ and just focus on writing efficient GPU computing code. However, we are limited with OpenGL ES version to 2.0 which does **not** support this kind of shaders. Therefore, we must do some _hacks_ and trick out GPU that we want to render something while we actually do just computations in our shaders.

The targetted chip is SGX540 (on the BBB) and SGX544 (on the BBAI - TODO: yet untested).
## Preparing the environment

## Examples

## Benchmarks
