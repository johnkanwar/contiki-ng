/**
 * \file
 *          A Contiki application that detects multiple 
 *          concurrent channel activity in the 2.4 GHz band.
 *          The application works on low-power sensor node platforms 
 *          that feature the cc2420 radio from Texas Instruments.
 *          
 * \author
 *          Venkatraman Iyer <iyer.venkat9@gmail.com>
 */

#include "contiki.h"
#include <math.h>
#include <stdio.h> /* For printf() */
#include <stdlib.h>
#include "dev/eeprom.h"
#include "string.h"
#include "dev/radio.h"
#include "net/netstack.h"
#include "net/packetbuf.h"
#include "lib/random.h"
#include "sys/pt.h"
#include "sys/timer.h"
#include "dev/watchdog.h"
#include "dev/leds.h"
#include "sys/rtimer.h"
#include "dev/serial-line.h"
#include "kmeans.h"

#include "cfs/cfs.h"

/*RPL-udp CLIENT includes*/
#include "net/routing/routing.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "os/lib/assert.h"

#include "sys/log.h"
#include "dev/radio.h"
#include "net/netstack.h"
// #include <stdio.h>
#include "nrf52840_bitfields.h"
#include "nrf52840.h"
#include "dev/leds.h"
#include "dev/etc/rgb-led/rgb-led.h"

/*---------------------------------------------------------------------------*/

//fonts color
#define FBLACK "\033[30;"
#define FRED "\033[31;"
#define FGREEN "\033[32;"
#define FYELLOW "\033[33;"
#define FBLUE "\033[34;"
#define FPURPLE "\033[35;"
#define D_FGREEN "\033[6;"
#define FWHITE "\033[7;"
#define FCYAN "\x1b[36m"
//background color
#define BBLACK "40m"
#define BRED "41m"
#define BGREEN "42m"
#define BYELLOW "43m"
#define BBLUE "44m"
#define BPURPLE "45m"
#define D_BGREEN "46m"
#define BWHITE "47m"
//end color
#define NONEX "\033[0m"

#define MAX_DURATION 1000

/*RPL-UDP + SpeckSense++ variables*/
/*Could be in a sperate file*/
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO
#define WITH_SERVER_REPLY 1
#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678
#define SEND_INTERVAL (60 * CLOCK_SECOND)
#define INIT_MSG a


/*---------------------------------------------------------------------------*/
#if PROCESS_ID == 0
PROCESS(specksense, "SpeckSense");
// AUTOSTART_PROCESSES(&specksense);
PROCESS(udp_client_process, "UDP client");
AUTOSTART_PROCESSES(&udp_client_process);
#elif PROCESS_ID == 1
PROCESS(specksense, "SpeckSense");
AUTOSTART_PROCESSES(&specksense);
#elif PROCESS_ID == 2
PROCESS(channel_allocation, "Spectrum monitoring");
AUTOSTART_PROCESSES(&channel_allocation);
#else
#error "Choose a valid process 1. specksense on RADIO_CHANNEL, 2. scanning all channels."
#endif
/*---------------------------------------------------------------------------*/

//! Global variables

static int print_counter = 0;

static uint16_t n_clusters;

//! Global variables for RSSI scan
static int rssi_val, rssi_valB, rle_ptr = -1, rle_ptrB = -1,
					 step_count = 1, cond,
					 itr,
					 n_samples, max_samples, rssi_val_mod,rssi_val_modB;
static unsigned rssi_levels[140]; //original 120 test 140

static rtimer_clock_t sample_st, sample_end;
static struct record record;
static struct record recordB;

// static unsigned rssi_values_arr[]

#if CHANNEL_METRIC == 2
static uint16_t cidx;
static int itr_j;
static int current_channel = RADIO_CHANNEL;
#endif


/*RPL-UDP + SpeckSense++ funcs*/
/*Could be in a sperate file*/

/*---------------------------------------------------------------------------*/
#if PROCESS_ID == 0
static struct etimer et;

static struct simple_udp_connection udp_conn;
static uip_ipaddr_t dest_ipaddr;

static int ID;
static int init = 0;
static int cnt = 0;

static void /*The arguments are basically what the clients recives. */
udp_rx_callback(struct simple_udp_connection *c,
				const uip_ipaddr_t *sender_addr,
				uint16_t sender_port,
				const uip_ipaddr_t *receiver_addr,
				uint16_t receiver_port,
				const uint8_t *data,
				uint16_t datalen)
{

	if (*data == 1) /*Init*/
	{
		//Init
		printf("INIT\n");
		ID = *data;
	}
	LOG_INFO("Received response '%.*s' from ", datalen, (char *)data);
	LOG_INFO_6ADDR(sender_addr);
#if LLSEC802154_CONF_ENABLED
	LOG_INFO_(" LLSEC LV:%d", uipbuf_get_attr(UIPBUF_ATTR_LLSEC_LEVEL));
#endif
	LOG_INFO_("\n");
}
/*---------------------------------------------------------------------------*/
static void
udp_init_send()
{
	char str[32];
	/*Send first init message to server so that the server can add my address*/
	snprintf(str, sizeof(str), "%d", 1337);
	//  snprintf(str, sizeof(str), "hello %d", count);

	simple_udp_sendto(&udp_conn, str, strlen(str), &dest_ipaddr);
}
/*---------------------------------------------------------------------------*/

#endif

/*---------------------------------------------------------------------------*/
static void rssi_sampler_combined(int time_window_ms)
{
	sample_st = RTIMER_NOW();
	max_samples = time_window_ms * 20;
	step_count = 1;
	rle_ptr = -1;
	rle_ptrB = -1;
	record.sequence_num = 0;
	int globalCounter = 0;
	print_counter = 0;

	printf("\n START combined\n");

	// for (int i = 0; i < (RUN_LENGTH); i++)
	// {
	// 	// printf("Old values: [0]: %d  [1]: %d \n", record.rssi_rle[i][0],record.rssi_rle[i][1]);

	// 	record.rssi_rle[i][0] = 0;
	// 	record.rssi_rle[i][1] = 0;
	// }

	record.sequence_num++;
	rle_ptr = -1;
	record.rssi_rle[0][1] = 0;
	record.rssi_rle[0][0] = 0;
	rle_ptrB = -1;
	recordB.rssi_rle[0][1] = 0;
	recordB.rssi_rle[0][0] = 0;
	n_samples = max_samples * 10;
	//   watchdog_stop();
	print_counter = 0;
	int times = 0;
	// watchdog_periodic();
	while ((rle_ptr < RUN_LENGTH)){
		times ++;
		/*Get RSSI value*/
		if (NETSTACK_RADIO.get_value(RADIO_PARAM_RSSI, &rssi_val) != RADIO_RESULT_OK){
			printf(" ff");
		}

		rssi_val -= 45; /* compensation offset */
		rssi_valB = rssi_val;
		int16_t debug_rssi = rssi_val;



		/*This should be incremented*/
		/*Loading for a potential Bluetooth, WiFi interference check. */
		if(rle_ptrB < RUN_LENGTH){
			n_samples = n_samples - 1;
			
			rssi_valB = ((signed char) rssi_valB);
			cond = 0x01 & (( recordB.rssi_rle[rle_ptrB][0] != rssi_levels[-rssi_valB-1]) | (recordB.rssi_rle[rle_ptrB][1] == 32767));
			
			/*----------------*/
			/*Max_duration achieved, move to next value*/
			if (recordB.rssi_rle[rle_ptrB][1] >= MAX_DURATION){
				cond = 1; /*Jump to next value*/
			}
			/*----------------*/
			rle_ptrB = rle_ptrB + cond;
			rssi_val_modB = -rssi_valB - 1;

			/*Check out of bounds*/
			if (rssi_val_modB >= 140){
				rssi_val_modB = 139;
				// printf("out of loop I guess\n");
			}
			/*----------------*/

		//rle_ptrB = rle_ptrB + cond;
			recordB.rssi_rle[rle_ptrB][0] = rssi_levels[rssi_val_modB];
			recordB.rssi_rle[rle_ptrB][1] = (record.rssi_rle[rle_ptrB][1]) * (1-cond) + 1 ;
		}







		/*If power level is <= 2 set it to power level 1*/
		if (debug_rssi <= -132){
			debug_rssi = -139;
		}

		/*Power level most be higher than one */
		if (rssi_levels[-debug_rssi - 1] > 1){
			n_samples = n_samples - 1;
			cond = 0x01 & ((record.rssi_rle[rle_ptr][0] != rssi_levels[-rssi_val - 1]) | (record.rssi_rle[rle_ptr][1] == 32767));

			/*Max_duration achieved, move to next value*/
			if (record.rssi_rle[rle_ptr][1] >= MAX_DURATION){
				cond = 1; /*Jump to next value*/
			}

			/*Increase rle_ptr when new powerlevel starts recording.*/
			rle_ptr = rle_ptr + cond;
			rssi_val_mod = -rssi_val - 1;

			/*Check out of bounds*/
			if (rssi_val_mod >= 140){
				rssi_val_mod = 139;
				// printf("out of loop I guess\n");
			}

			/*Create 2D vector*/
			record.rssi_rle[rle_ptr][0] = rssi_levels[rssi_val_mod];
			record.rssi_rle[rle_ptr][1] = (record.rssi_rle[rle_ptr][1]) * (1 - cond) + 1;

			if (0){ //Can be removed
				if (record.rssi_rle[rle_ptr][0] >= 10){
					//This print is important for the SFD indentification ?
					printf("%d,%d:", record.rssi_rle[rle_ptr][0], record.rssi_rle[rle_ptr][1]);
					// print_counter ++;
					
				}
				globalCounter++;
			}
		}
		else{					
			if(rle_ptr == RUN_LENGTH -2)
			{
				 printf("Debug"); // was a \n before
			}
		}
	}
	// printf("This is how many times the loop looped: %d \n", times);

	// for(int i =0; i < 500; i++)
	// {
	// 	for(int k = 0; k < 2; k++)
	// 	{
	// 		printf("recordB: [%d][%d] = (%d,%d) \n", i,k,recordB.rssi_rle[i][0],recordB.rssi_rle[i][1]);
	// 	}
	// }
	watchdog_start();

	sample_end = RTIMER_NOW();
	if (rle_ptrB < RUN_LENGTH)
		rle_ptrB++;
	
	if (rle_ptr < RUN_LENGTH)
		rle_ptr++;

	printf("\nNumber of sampels %d : rle_ptr %d\n", globalCounter, rle_ptr);
	printf(" \n");
	printf(" \n");
}
/*---------------------------------------------------------------------------*/
static void rssi_sampler(int time_window_ms)
{
	sample_st = RTIMER_NOW();
	max_samples = time_window_ms * 20;
	step_count = 1;
	rle_ptr = -1;
	record.sequence_num = 0;
	int globalCounter = 0;
	print_counter = 0;

	printf("\n START \n");

	// for (int i = 0; i < (RUN_LENGTH); i++)
	// {
	// 	// printf("Old values: [0]: %d  [1]: %d \n", record.rssi_rle[i][0],record.rssi_rle[i][1]);

	// 	record.rssi_rle[i][0] = 0;
	// 	record.rssi_rle[i][1] = 0;
	// }

	record.sequence_num++;
	rle_ptr = -1;
	record.rssi_rle[0][1] = 0;
	record.rssi_rle[0][0] = 0;
	n_samples = max_samples * 10;
	//   watchdog_stop();
	print_counter = 0;
	int times = 0;
	watchdog_periodic();
	while ((rle_ptr < RUN_LENGTH)){
		times ++;
		/*Get RSSI value*/
		if (NETSTACK_RADIO.get_value(RADIO_PARAM_RSSI, &rssi_val) != RADIO_RESULT_OK){
			printf(" ff");
		}

		rssi_val -= 45; /* compensation offset */
		int16_t debug_rssi = rssi_val;

		/*If power level is <= 2 set it to power level 1*/
		if (debug_rssi <= -132){
			debug_rssi = -139;
		}

		/*Power level most be higher than one */
		if (rssi_levels[-debug_rssi - 1] > 1)	{
			n_samples = n_samples - 1;
			cond = 0x01 & ((record.rssi_rle[rle_ptr][0] != rssi_levels[-rssi_val - 1]) | (record.rssi_rle[rle_ptr][1] == 32767));

			/*Max_duration achieved, move to next value*/
			if (record.rssi_rle[rle_ptr][1] >= MAX_DURATION){
				cond = 1; /*Jump to next value*/
			}

			/*Increase rle_ptr when new powerlevel starts recording.*/
			rle_ptr = rle_ptr + cond;
			rssi_val_mod = -rssi_val - 1;

			/*Check out of bounds*/
			if (rssi_val_mod >= 140){
				rssi_val_mod = 139;
				// printf("out of loop I guess\n");
			}

			/*Create 2D vector*/
			record.rssi_rle[rle_ptr][0] = rssi_levels[rssi_val_mod];
			record.rssi_rle[rle_ptr][1] = (record.rssi_rle[rle_ptr][1]) * (1 - cond) + 1;

			if (0){ //Can be removed
				if (record.rssi_rle[rle_ptr][0] >= 10){
					//This print is important for the SFD indentification ?
					printf("%d,%d:", record.rssi_rle[rle_ptr][0], record.rssi_rle[rle_ptr][1]);
					// print_counter ++;
					
				}
				globalCounter++;
			}
		}
		else{					/*I think a problem might be that it loops here without printing anything for a very long amount of time. */
			if(rle_ptr == 498)
			{
				printf("BIRDO"); // was a \n before
			}
		}
	}
	printf("This is how many times the loop looped: %d \n", times);
	watchdog_start();

	sample_end = RTIMER_NOW();
	if (rle_ptr < RUN_LENGTH)
		rle_ptr++;

	printf("\nNumber of sampels %d : rle_ptr %d\n", globalCounter, rle_ptr);
	printf(" \n");
	printf(" \n");
}
/*---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/
static void rssi_sampler_old(int time_window_ms){
	// sample_st = RTIMER_NOW();
	max_samples = time_window_ms * 20;
	step_count = 1; rle_ptr = -1;
	record.sequence_num = 0;
     
     while (step_count <= NUM_RECORDS) {

	  record.sequence_num++;
	  rle_ptr = -1;
	  record.rssi_rle[0][1] = 0;
      record.rssi_rle[0][0] = 0;
	  n_samples = max_samples;

	//   watchdog_periodic();
 

	  while ((rle_ptr < RUN_LENGTH) && (n_samples)) {
			if (NETSTACK_RADIO.get_value(RADIO_PARAM_RSSI, &rssi_val) != RADIO_RESULT_OK){
				printf(" ff");
			}
	    //    rssi_val -= 45; /* compensation offset */

	       n_samples = n_samples - 1;
	       
	       rssi_val = ((signed char) rssi_val);
	       cond = 0x01 & (( record.rssi_rle[rle_ptr][0] != rssi_levels[-rssi_val-1]) | (record.rssi_rle[rle_ptr][1] == 32767));
	       
	       rle_ptr = rle_ptr + cond;
	       record.rssi_rle[rle_ptr][0] = rssi_levels[-rssi_val-1];
	       record.rssi_rle[rle_ptr][1] = (record.rssi_rle[rle_ptr][1]) * (1-cond) + 1 ;
	
	  }
		
	  watchdog_start();

	  step_count++;
     }

     if (rle_ptr < RUN_LENGTH) rle_ptr++;
}

/*---------------------------------------------------------------------------*/

static void init_power_levels()
{
	printf("POWER_LEVELS: %d", POWER_LEVELS);
#if POWER_LEVELS == 2
	for (itr = 0; itr < 120; itr++)
		if (itr < 90)
			rssi_levels[itr] = 2;
		else
			rssi_levels[itr] = 1;
#elif POWER_LEVELS == 4
	for (itr = 0; itr < 120; itr++)
		if (itr < 30)
			rssi_levels[itr] = 4;
		else if (itr >= 30 && itr < 60)
			rssi_levels[itr] = 3;
		else if (itr >= 60 && itr < 90)
			rssi_levels[itr] = 2;
		else
			rssi_levels[itr] = 1;
#elif POWER_LEVELS == 8
	for (itr = 0; itr < 120; itr++)
		if (itr < 14)
			rssi_levels[itr] = 8;
		else if (itr >= 12 && itr < 25)
			rssi_levels[itr] = 7;
		else if (itr >= 25 && itr < 38)
			rssi_levels[itr] = 6;
		else if (itr >= 38 && itr < 51)
			rssi_levels[itr] = 5;
		else if (itr >= 51 && itr < 64)
			rssi_levels[itr] = 4;
		else if (itr >= 64 && itr < 77)
			rssi_levels[itr] = 3;
		else if (itr >= 77 && itr < 90)
			rssi_levels[itr] = 2;
		else
			rssi_levels[itr] = 1;
#elif POWER_LEVELS == 16
	for (itr = 0; itr < 120; itr++)
		if (itr < 6)
			rssi_levels[itr] = 16;
		else if (itr >= 6 && itr < 12)
			rssi_levels[itr] = 15;
		else if (itr >= 12 && itr < 18)
			rssi_levels[itr] = 14;
		else if (itr >= 18 && itr < 24)
			rssi_levels[itr] = 13;
		else if (itr >= 24 && itr < 30)
			rssi_levels[itr] = 12;
		else if (itr >= 30 && itr < 36)
			rssi_levels[itr] = 11;
		else if (itr >= 36 && itr < 42)
			rssi_levels[itr] = 10;
		else if (itr >= 42 && itr < 48)
			rssi_levels[itr] = 9;
		else if (itr >= 48 && itr < 54)
			rssi_levels[itr] = 8;
		else if (itr >= 54 && itr < 60)
			rssi_levels[itr] = 7;
		else if (itr >= 60 && itr < 66)
			rssi_levels[itr] = 6;
		else if (itr >= 66 && itr < 72)
			rssi_levels[itr] = 5;
		else if (itr >= 72 && itr < 78)
			rssi_levels[itr] = 4;
		else if (itr >= 78 && itr < 84)
			rssi_levels[itr] = 3;
		else if (itr >= 84 && itr < 90)
			rssi_levels[itr] = 2;
		else
			rssi_levels[itr] = 1;
#elif POWER_LEVELS == 120
	int i_c = 140 - 1;
	for (itr = 1; itr <= 140; itr++)
	{
		rssi_levels[i_c] = itr;
		i_c--;
	}
#elif POWER_LEVELS == 20
	for (itr = 0; itr < 140; itr++)
		if (/*itr >= 60 &&*/ itr < 64) 
			rssi_levels[itr] = 20;
		else if (itr >= 64 && itr < 68)
			rssi_levels[itr] = 19;
		else if (itr >= 68 && itr < 72)
			rssi_levels[itr] = 18;
		else if (itr >= 72 && itr < 76)
			rssi_levels[itr] = 17;
		else if (itr >= 76 && itr < 80)
			rssi_levels[itr] = 16;
		else if (itr >= 80 && itr < 84)
			rssi_levels[itr] = 15;
		else if (itr >= 84 && itr < 88)
			rssi_levels[itr] = 14;
		else if (itr >= 88 && itr < 92)
			rssi_levels[itr] = 13;
		else if (itr >= 92 && itr < 96)
			rssi_levels[itr] = 12;
		else if (itr >= 96 && itr < 100)
			rssi_levels[itr] = 11;
		else if (itr >= 100 && itr < 104) 
			rssi_levels[itr] = 10;
		else if (itr >= 104 && itr < 108) 
			rssi_levels[itr] = 9;
		else if (itr >= 108 && itr < 112) 
			rssi_levels[itr] = 8;
		else if (itr >= 112 && itr < 116) 
			rssi_levels[itr] = 7;
		else if (itr >= 116 && itr < 120) 
			rssi_levels[itr] = 6;
		else if (itr >= 120 && itr < 124) 
			rssi_levels[itr] = 5;
		else if (itr >= 124 && itr < 128) 
			rssi_levels[itr] = 4;
		else if (itr >= 128 && itr < 132) 
			rssi_levels[itr] = 3;
		else if (itr >= 132 && itr < 136) 
			rssi_levels[itr] = 2;
		else if (itr >= 136 && itr < 140)
			rssi_levels[itr] = 1;
		else
			rssi_levels[itr] = 1337; // Will never happend

#else
#error "Power levels should be one of the following values: 2, 4, 8, 16 or 120"
#endif
}

/*---------------------------------------------------------------------------*/
#if PROCESS_ID == 0
PROCESS_THREAD(udp_client_process, ev, data)
{
	static struct etimer et;
	static unsigned count;
	static char str[32];
	static int nr_failures = 0;

	PROCESS_BEGIN();

	NETSTACK_MAC.off();
	NETSTACK_RADIO.on();
	if (NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, 26) != RADIO_RESULT_OK){
		printf("ERROR: failed to change radio channel\n");
		break;
	}
	/* Initialize UDP connection */
	simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
						UDP_SERVER_PORT, udp_rx_callback);

	etimer_set(&et, CLOCK_SECOND * 0.5);
	while (1){

		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		etimer_reset(&et);

		if (NETSTACK_ROUTING.node_is_reachable()){
			printf("First one down\n");
			if (NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)){
				nr_failures = 0;

				if (!init){
					LOG_INFO("Init function!! \n");

					/*Send init*/
					udp_init_send();
					init = 1;
				}else{

					/*Flash led light green*/
					rgb_led_set(RGB_LED_GREEN);
					LOG_INFO("This is the client! \n");
					LOG_INFO("Sending request %u to ", count);
					LOG_INFO_6ADDR(&dest_ipaddr);
					LOG_INFO_("\n");
					snprintf(str, sizeof(str), "5 %d", count);
					simple_udp_sendto(&udp_conn, str, strlen(str), &dest_ipaddr);
					count++;
				}
			}else{
				LOG_INFO("Didn't get root_ipaddr\n");
			}
		}
		else{
			rgb_led_set(RGB_LED_RED);
			LOG_INFO("Not reachable yet: %d\n", cnt);
			if (nr_failures > 20){
				nr_failures = 0;
				process_start(&specksense, NULL);
				PROCESS_EXIT();
			}
			nr_failures++;
			cnt++;
		}

		rgb_led_off();
	}

	PROCESS_END();
}
PROCESS_THREAD(specksense, ev, data)
{
	static int nr_times = 0;
	PROCESS_BEGIN();
	NETSTACK_MAC.off();
	NETSTACK_RADIO.on();

	watchdog_start();
	init_power_levels();

	if (NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, RADIO_CHANNEL) != RADIO_RESULT_OK){
		printf("ERROR: failed to change radio channel\n");
		break;
	}
	etimer_set(&et, CLOCK_SECOND * 2);

	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

	while (nr_times < 10){
		record.sequence_num = 0;

		rssi_sampler(TIME_WINDOW);
		//  leds_off(LEDS_GREEN);
		//  leds_off(LEDS_RED);

		n_clusters = kmeans(&record, rle_ptr);
		check_similarity(PROFILING);
		nr_times++;
		PROCESS_PAUSE();
	}
	nr_times = 0;
	process_start(&udp_client_process, NULL);
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
#elif PROCESS_ID == 1
PROCESS_THREAD(specksense, ev, data)
{

	PROCESS_BEGIN();
	NETSTACK_MAC.off();
	NETSTACK_RADIO.on();

	watchdog_start();
	init_power_levels(); //Populate the rssi quantization levels.

	if (NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, 26) != RADIO_RESULT_OK){
		printf("ERROR: failed to change radio channel\n");
		break;
	}

	while (1){

		record.sequence_num = 0;

		//  leds_on(LEDS_RED);
		//  leds_on(LEDS_GREEN);

		if(0)
		{
			rssi_valB = 0;
		}
	/*
		They need to have the same format after the RSSI sampler
	*/

		if(0){
			
			rssi_sampler_combined(TIME_WINDOW);

			if(0){
				/* TODO: Basically what I need to do is to send in a bool, and change it to true if unintentionall interference seems to be the case.
				then run kmeans_old. */
				n_clusters = kmeans(&record, rle_ptr);
				check_similarity(PROFILING);
			}
			else
			{
				n_clusters = kmeans_old(&recordB, rle_ptrB);
				print_interarrival(RADIO_CHANNEL, n_clusters);
			}
			/*If a jamming attack return*/
			/*else*/
			/*Run K-means_old check for WiFi and Bluetooth interference*/
		}




		if(0){
			rssi_sampler(TIME_WINDOW);
		}

		if(1){
			rssi_sampler_old(TIME_WINDOW);
			
			printf("done sampling on channel 26\n");
			printf("RSSI");
			for (int i = 0; i <= rle_ptr; i++)
				printf(":%d,%d",record.rssi_rle[i][0],record.rssi_rle[i][1]);
			printf("\nrle_ptr:%d,%d,%d\n",rle_ptr, max_samples, n_samples);
		}
		/*First check for Bluetooth or WiFi*/
		
		if(0){
			n_clusters = kmeans(&record, rle_ptr);
			check_similarity(PROFILING);
		}
		if(1){
			n_clusters = kmeans_old(&record, rle_ptr);
			print_interarrival(RADIO_CHANNEL, n_clusters);
		}

		PROCESS_PAUSE();
	}
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
#elif PROCESS_ID == 2
PROCESS_THREAD(channel_allocation, ev, data)
{
	PROCESS_BEGIN();
	NETSTACK_MAC.off();
	NETSTACK_RADIO.on();
	init_power_levels();

	for (itr = 0; itr < 16; itr++)
		channel_metric[itr] = 0;

	cidx = 11;
	// cc2420_set_channel(cidx);
	if (NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, cidx) != RADIO_RESULT_OK)
	{
		printf("ERROR: failed to change radio channel\n");
		break;
	}
	itr = 0;
	while (itr < 3)
	{

		record.sequence_num = 0;

		leds_on(LEDS_RED);
		rssi_sampler(TIME_WINDOW);
#if DEBUG_RSSI == 1
		print_rssi_rle();
#endif
		//   watchdog_stop();
#if CHANNEL_METRIC == 1

		printf("Channel %d:", cidx); /*TODO get the radio channel.*/

		n_clusters = kmeans(&record, rle_ptr);
		print_interarrival(cidx, n_clusters);
#elif CHANNEL_METRIC == 2
		channel_metric[cidx - 11] = channel_metric[cidx - 11] +
									channel_metric_rssi_threshold(&record, rle_ptr);
#endif
		channel_rate(&record, n_clusters);
		watchdog_start();

		//   leds_off(LEDS_RED);
		NETSTACK_RADIO.get_value(RADIO_PARAM_CHANNEL, &current_channel)
			cidx = (current_channel == 26) ? 11 : current_channel + 1;
		NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, cidx)
			//   cc2420_set_channel(cidx);
			if (cidx == 11)
				itr++;
	}
#if CHANNEL_METRIC == 2
	for (itr = 0; itr < 16; itr++)
	{
		channel_metric[itr] = channel_metric[itr] / 3.0;
		printf("Channel %d: Channel metric: %ld.%03u\n",
			   itr + 11, (long)channel_metric[itr],
			   (unsigned)((channel_metric[itr] -
						   floor(channel_metric[itr])) *
						  1000));
	}

	for (itr = 0; itr < 15; itr++)
		for (itr_j = itr + 1; itr_j < 16; itr_j++)
			if (channel_metric[itr] > channel_metric[itr_j])
			{
				int tmp_channel;
				float tmp_val = channel_metric[itr];

				channel_metric[itr] = channel_metric[itr_j];
				channel_metric[itr_j] = tmp_val;

				tmp_channel = channel_arr[itr];
				channel_arr[itr] = channel_arr[itr_j];
				channel_arr[itr_j] = tmp_channel;
			}

	printf("Channel ordering:");
	for (itr = 15; itr >= 0; itr--)
	{
		printf(" %d", channel_arr[itr]);
		if (itr)
			printf(",");
	}

#endif
	PROCESS_END();
}
#endif
/*---------------------------------------------------------------------------*/
/*
	TODO: Understand how I can read the Bluetooth values and the WiFi values. 
		  I think I need to profile the values. 
		  So basically I need run SpeckSense when it is Bluetooth interference, and WiFi interference. 
		  And calculate the average clusters created and inter-bursts. 

		  Then I run it again and measure if it is possible to detect the Bluetooth interference and WiFi interference. 

	TODO: Experiments to carry out. 
		  Run old SpeckSense and check if it identifies WiFi and Bluetooth.
		  Understand how SpeckSense is understanding Bluetooth
		  Run and check if old SpeckSense identifies Bluetooth and WiFi.
		  Run and check if old SpeckSense is triggering on unintentionall attacks. 
		  Run and check if SpeckSense++ triggers on Bluetooth and WiFi.
		  Should be about it .

	TODO: Whichlist:
		  Is it possible to identify both Bluetooth or WiFi somultenously as reading for jamming attacks. 
		  Because I have stored both values should it absolutley be a possibility. 
		  If old SpeckSense doesn't identify jamming attacks as bluetooth or WiFi
		  
		  Then
		  Just run both K-means and test if it is a possibility.  

	TODO: It is a possibility to just remove the Bluetooth and WiFi identification.
*/