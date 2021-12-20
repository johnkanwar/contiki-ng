# TrustZone example with non-secure and secure zones.

The example contains two different projects.
The secure_zone and the non_secure_zone.
And MDK folder that contains the linker scripts, startup files. 


## ~ How to build ~
Run "make tz_build" to build secure, non-secure and merge the hex files. <br />
Run "make rm" to remove the secure and non-secure builds. <br />

To flash to the nRF5340 cd into secure_zone and run : <br />
sudo make TARGET=nrf BOARD=nrf5340/dk/application example_s.upload PORT=/dev/<PORT>

To login the port run: <br />
make TARGET=nrf BOARD=nrf5340/dk/application login PORT=/dev/<PORT>

## ~ GDB setup for nRF (Linux and Mac) ~
Some of the things required to set up GDB may already be installed. For example nRF Command Line (nrfjprog), SEGGER J-Link, GNU Arm Embedded toolchain etc. These are set by following [contiki-nrf](https://github.com/contiki-ng/contiki-ng/wiki/Platform-nrf52840)
 
1. Install gdb-multiarch. (Which should already be installed with the GNU Arm embedded toolchain)
    ```sh
    sudo apt-get update -y
    ```
    ```sh
    sudo apt-get install gdb-multiarch
    ```
2. Compile the builds with optimization flags to create debug symbols. -g
  
3. Open a JLinkGDBServer to allow connections from the GDB client (In this case do we target the nRF5340).
    ```sh
    JLinkGDBServer -device nrf5340_xxaa -if swd -port 2331
    ```
    -"device" nrfxx_xxaa (What type of nrf version)                                 
    -"if" specifies the debug interface                                            
    -"port" which port to use. 

4. In another terminal start gdb-multiarch  
    ```sh
    gdb-multiarch example.FILE
    ```
    -"file" could for example be .ELF or .out etc. 
    
5. In GDB, connect to the GDB server  
    ```sh
    target remote localhost:2331
    ```


## ~ Secure example ~
The secure example contains two different examples, one that follow configuration like Zehpyr's SPM and another one that uses TF-M in order to configure. 

