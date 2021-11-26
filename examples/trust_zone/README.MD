# TrustZone example with non-secure and secure zones.

The example contains two different projects.
The secure_zone and the non_secure_zone.
And MDK folder that contains the linker scripts, startup files and a Makefile.def file that configures build for secure or non-secure. 

## ~ Secure example ~
The secure example contains two different examples, one that follow configuration like Zehpyr's SPM and another one that uses TF-M in order to configure. 


## ~ Focus ~
The main focus is to initialize the data in the non_secure build when we are jumping from non-secure to secure. 

## ~ Problem ~
When jumping from secure to non-secure does it freze right after the jump is called. 


## ~ Things tested ~

1. Tested the SPM code that Zephyr used for jumping. (That they used Before switching to TF-M.) 
2. Tested using Sparrows boot_image functions to switch and initalize non-secure build.
3. Tested using the TF-M functions for configuration and jumping.
4. Tested the original linker script and gcc_starup files. 
5. Changed the linker scripts and gcc_startup files to a more TF-M like way.
6. Setuped a Zephyr project from a SDK and ran SPM project, closley followed how they configured and did the same.
7.


## ~ How to build ~
In the folder mdk does it exist a Makefile.def file. It contains a variable that is called secure_build.
Set secure_build = 1 for building for secure or secure_build = 0 for a non-secure build. 

If secure_build = 1. 
cd to secure_zone, build the project and flash it. 

make TARGET=nrf BOARD=nrf5340/dk/application (Builds the project)
sudo make TARGET=nrf BOARD=nrf5340/dk/application example_s.upload PORT=/dev/ttyACM2 (Upload the project to nRF5340)
make TARGET=nrf BOARD=nrf5340/dk/application login PORT=/dev/ttyACM2 (Read the output)

If secure_build = 0.
cd to non_secure_zone, build the project and flash it. 
sudo make TARGET=nrf BOARD=nrf5340/dk/application example_s.upload PORT=/dev/ttyACM2 (Upload the project to nRF5340)

The example_1 variable chooses which example to build. Setting example_1 to 1 will build the SPM_example setting example_1 to 0 will build the TFM_example.

## ~ Merge hex files ~
Currently there is no script for merging the hex files automatically. 
So either go into the respective builds and merge the hex files manually, or use srec. 
