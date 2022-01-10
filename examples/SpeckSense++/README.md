# SpeckSense++
## _Detect and identify radio jamming attacks_


SpeckSense++ is an algorithm built upon [SpeckSense] that makes it possible to identify and classify radio jamming attacks in low-power wireless sensor networks. It Uses [Contiki-NG] in order to work.


# SpeckSense briefly
[SpeckSense][speckSense]
> An algorithm for detecting multiple sources of interference in the 2.4 GHz spectrum.
> It is developed using Contiki OS, and works on the cc2420 radio.

SpeckSense is an algorithm that makes it possible to detect and identify unintentional interference in the 2.4gHz spectrum such as Bluetooth and WiFi beacons. It does also make it a possibility to detect multiple interference sources simultaneously.



## Features
There are three main parts of SpeckSense++:

- RSSI-sampler
- K-means clustering
- Identification part

### RSSI-sampler

It reads in RSSI-values from a specific channel. Those values are quantized and run-length encoded into 2D vectors.
The 2D vectors are sent to the K-means clustering algorithm.

### K-means clustering
The K-means clustering algorithm is weighted towards the power level, which means it emphasizes 2D vectors that have a high power level. It clusters the 2D vectors into clusters which are then compared to the different profiled thresholds in order to identify what type of jamming attack is occurring.

### Interference classification
The interference classification compares the different clusters’ centroids that are created by the K-means clustering algorithm towards the profiled thresholds. In case any of the centroids are inside the boundaries of a threshold is the centroid saved and a suspicion variable is increased. When enough suspicion has risen are multiple suspicious samples stored. The average is calculated between the suspicious samples. That difference between every suspicious sample and the average is calculated. If the difference is high enough, have we established an inconsistent jamming attack which is in this the random jammer. If it is consistent does it depend on what type of threshold it is inside in order to classify it.  

## Differences between SpeckSense and SpeckSense++
The RSSI-sampler SpeckSense++ uses is the same as SpeckSense's RSSI-sampler. It does however not sample power levels that are too low, i.e power levels under 3.

SpeckSense creates bursts of RSSI-vales that consists of a weighted mean power level and the total duration of the subsequence of the RSSI-values. It classifies the interference by comparing the average inter-burst separation and the numbers of clusters created.  

SpeckSense++ creates 2D vectors that consist of power level and duration. In order to identify the different jamming attacks does it compare power level and duration towards pre-profiled characteristics of the different radio jamming attacks. It does also check for consistency by taking samples from the undergoing attack in order to secure what type of radio jamming attack it is.

## Variables

The default variables are configurable.

SpeckSense++ variables | -
--- | --- |
Amount of rssi samples:         |     500
Max duration:                   |     26 ms
Max amount of clusters          |     11
Amount of power levels          |     19
Jammers power level threshold   |     11
PCJ duration threshold          |     4.94 ms
Deceptive duration threshold    |     4.94 ms
SFD duration MIN threshold      |     0.078 ms
SFD duration MAX threshold      |     0.182 ms
BT power level Min threshold    |     4
BT power level MAX threshold    |     5
BT duration MIN threshold       |     0.5 ms
BT duration MAX threshold       |     3.6 ms
WiFI duration MIN threshold     |     2.34 ms
WiFi duration MAX threshold     |     4.16 ms
WiFi power level MIN threshold  |     6
WiFi power level MAX threshold  |     7


## Tech

In order to run SpeckSense++ do you need:

- [Contiki-NG] - Contiki-NG: The OS for Next Generation IoT Devices

## Installation

SpeckSense++ requires [Contiki-NG] to run.
The installation for the Linux toolchain can be found here: [Contiki-NG toolchain]

After Contiki-NG is installed, build and upload SpeckSense.

```sh
cd examples/project
make TARGET=nrf52840 <BOARD>
sudo make TARGET=nrf52840 <BOARD> specksense.dfu-upload <PORT>
make TARGET=nrf52840 <BOARD> login <PORT>
```
For example, building for nRF52840 on port 0, would look like this:

```sh
make TARGET=nrf52840 BOARD=dongle && sudo make TARGET=nrf52840 BOARD=dongle specksense.dfu-upload PORT=/dev/ttyACM0 && make TARGET=nrf52840 BOARD=dongle login PORT=/dev/ttyACM0
```

In order to detect the different attacks is it necessary to profile them.

## Makefile
In the makefile is it possible to set the different thresholds.
The symbol J_D determines if the firmware is going to detect intentional interference or unintentional interference.

## PROCESS_ID
PROCESS_ID 1 activated runs SpeckSense++ normally. 
PROCESS_ID 0 activated runs SpeckSense++ togheter with RPL-UDP client. It will run RPL-UDP client and if the connetion fails 20 times will it turn off the RPL-UDP process and start a SpeckSense++. The SpeckSense++ will run 10 times before trying to get a stable connection again.  

## CSMA settings
The Maximum number of re-transmissions attempts are set to 0, in order to identify the SFD jammer more accurate. 


[//]: # (These are reference links used in the body of this note and get stripped out when the markdown processor does its job. There is no need to format nicely because it shouldn't be seen. Thanks SO - http://stackoverflow.com/questions/4823468/store-comments-in-markdown-syntax)

   [SpeckSense]: <https://github.com/iyervenkat9/SpeckSense>
   [Contiki-NG]: <https://github.com/contiki-ng/contiki-ng>
   [nRF52840]: <https://www.nordicsemi.com/Products/Low-power-short-range-wireless/nRF52840>
   [Contiki-NG toolchain]: <https://github.com/contiki-ng/contiki-ng/wiki/Toolchain-installation-on-Linux>



