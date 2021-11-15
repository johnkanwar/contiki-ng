# TrustZone example with non-secure and secure zones.

The example contains two different projects.
The secure_zone and the non_secure_zone.


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

## ~ Merge hex files ~
Currently do I either merge the hex files manually or with srec.

## ~ Change build
