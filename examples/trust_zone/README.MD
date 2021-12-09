# TrustZone example with non-secure and secure zones.

The example contains two different projects.
The secure_zone and the non_secure_zone.
And MDK folder that contains the linker scripts, startup files. 


## ~ How to build ~
Run "make tz" to build, merge and upload.
Run "make rm" to remove the secure and non-secure builds


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
