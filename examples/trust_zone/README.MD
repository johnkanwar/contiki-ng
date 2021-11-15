# TrustZone example with non-secure and secure zones.

The example contains two different projects.
The secure_zone and the non_secure_zone.
And MDK folder that contains the linker scripts, startup files and a Makefile.def file that configures build for secure or non-secure. 


## ~ Focus ~
The main focus is to initialize the data in the non_secure build when we are jumping from non-secure to secure. 
Everything is setup through the linker scripts and the core_init function that is located 
in secure_zone/example_s.c.
The configure_ns_code function in the secure_zone/examplse_s.c file setups the necessary
in order to jump from secure to non-secure.
It follows how TF-M does it. 

## ~ Problem ~
When jumping from secure to non-secure does it freze right after the jump is called. 


## ~ Things tested ~

1. Tested the SPM code that Zephyr used for jumping. (That they used Before switching to TF-M.) 
2. Tested using Sparrows boot_image functions to switch and initalize non-secure build.
3. Tested using the TF-M functions for configuration and jumping. (This is in the current push)
4. Tested the original linker script and gcc_starup files. 
5. Changed the linker scripts and gcc_startup files to a more TF-M like way. (This is in the current push)
6. Setuped a Zephyr project from a SDK and ran SPM project, closley followed how they configured and did the same.
7.





## ~ Things removed ~
Removed all the secure and non-secure partitioning configuration from example_s.c
The functions do however exist in spu.h/spu/c

## ~ How to build ~
In the folder mdk does it exist a Makefile.def file. It contains a variable that is called secure_build.
Set secure_build = 1 for building for secure or secure_build = 0 for a non-secure build. 

If secure_build = 1. 
cd to secure_zone, build the project and flash it. 

make TARGET=nrf BOARD=nrf5340/dk/application (Builds the project)
sudo make TARGET=nrf BOARD=nrf5340/dk/application example_s.upload PORT=/dev/ttyACM2 (Upload the project to nrf5340)
make TARGET=nrf BOARD=nrf5340/dk/application login PORT=/dev/ttyACM2 (Read the output)

If secure_build = 0.
cd to non_secure_zone, build the project and flash it. 
sudo make TARGET=nrf BOARD=nrf5340/dk/application example_s.upload PORT=/dev/ttyACM2 (Upload the project to nrf5340)

## ~ Merge hex files ~
Currently there is no script for merging the hex files automatically. 
So either go into the respective builds and merge the hex files manually, or use srec. 
