/*
*	Jammer with support of direct test mode(DTM)
*/


/*
*	-- Good to know modifications -- 
*	I have tried to do it the same way as the original dtm_example that could be find in the file "ble_dtm_example_original"
*	Changed the dtm_cmd function so it takes in an struct instead of a bitmask in order for easier debuging.
*	Added various prints in ble_dtm.c
*	Comment out the function timer_init() in ble_dtm.c as it made the program froze. (Might be an important function) 
*/

#include "contiki.h"

#include <stdio.h>
#include "leds.h"
#include "etimer.h"
#include "dev/radio.h"
#include "net/netstack.h"
#include "packetbuf.h"
#include "ble_dtm.h"

#include "uart0.h"
/*---------------------------------------------------------------------------*/
PROCESS(reactive_process, "reactive process");
AUTOSTART_PROCESSES(&reactive_process);

/*---------------------------------------------------------------------------*/
static radio_value_t channel_c = 26;
static struct ble_cmd input;
static uint8_t debug_switch = 0;

/*---------------------------------------------------------------------------*/
static void
set_channel(void)
{
	printf("Set channel\n");
	if (NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, channel_c) != RADIO_RESULT_OK)
	{
		printf("ERROR: failed to change radio channel\n");
	}
}
/*---------------------------------------------------------------------------*/
static void
set_cmd_setup(uint8_t cmd, uint8_t freq, uint32_t length, uint8_t payload)
{
	// struct ble_cmd input;
	input.cmd = cmd;
	input.frequency = freq;
	input.length = length;
	input.payload = payload;
}
/*---------------------------------------------------------------------------*/
static void
activate_dtm(void)
{
	if (dtm_init() != DTM_SUCCESS)
	{
		// If DTM cannot be correctly initialized, then we just return.
		printf("ERROR: failed to init dtm_init()\n");
	}
	if (dtm_cmd(input) != DTM_SUCCESS)
	{
		printf("ERROR: failed to use dtm_cmd()");
		//ASSERT("ERROR");
		// Extended error handling may be put here.
		// Default behavior is to return the event on the UART (see below);
		// the event report will reflect any lack of success.
	}
}
/*---------------------------------------------------------------------------*/
static void
run_transmit(void)
{
	NETSTACK_RADIO.prepare(packetbuf_hdrptr(), packetbuf_totlen()); // 0 == OK
	NETSTACK_RADIO.transmit(120);									// 0 == OK
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(reactive_process, ev, data)
{
	PROCESS_BEGIN(); // Start process // Only variables above this line.

	set_channel();
	/*RSSI:-49-49-91-49-49-93-49-49-92-49-49-92-49-49-92-49*/
	printf("%d netstack_mac_off\n", NETSTACK_MAC.off());
	printf("%d netstack_radio_on\n", NETSTACK_RADIO.on());

//	NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, RADIO_CONST_TXPOWER_MAX); //0 enum OK

	/*Set function*/
	switch (debug_switch)
	{
	case 0:
		set_cmd_setup(
			LE_TRANSMITTER_TEST,
			39, //frequency which is actually channel
			254,
			DTM_PKT_PRBS9);
		break;
	case 1:
		set_cmd_setup(
			LE_TRANSMITTER_TEST,
			0,
			CARRIER_TEST,
			DTM_PKT_TYPE_VENDORSPECIFIC);
		break;
	default:
		set_cmd_setup(
			LE_TEST_SETUP,
			0,
			0,
			0);
	}

	while (1)
	{
		activate_dtm();
		run_transmit();
		PROCESS_PAUSE();
	}
	PROCESS_END();
}

/*
		Function argument flow :: If activate_dtm() is outside the while-loop.
		* dtm_cmd(input)
			#case LE_TRANSMITTER_TEST
			- on_test_transmit_cmd()
				#case  DTM_PKT_TYPE_VENDORSPECIFIC
				- dtm_vendor_specific_pkt()
					#case CARRIER_TEST_STUDIO
					-radio_prepare(TX_MODE);
						dtm_constant_carrier();		<---- STUCK

		* dtm_cmd(input)
			#case LE_TRANSMITTER_TEST
			- on_test_transmit_cmd()
				#case  DTM_PKT_PRBS9
				- dmemcpy
					-radio_prepare(TX_MODE);
		
		run_transmit();		<---- STUCK

*/



/*Reactive jammer*/

/*
		1. Scan for activity.
			a. Find channel activity
				- Start transmitting on that channel
				- After n seconds, check if activiy is still happening on that channel.
					. If yes, go to step  1.a.-
					. If no, go to step 1.a
			b. Don't find channel activity 
				- Go to step 1.	
*/

/*What the reference does*/

/*
		1. Scan for activity.
			a. Find channel activity
				- Notice SFD is recived
					. Turn on radio test mode for n seconds and interfere.
						_ Go to step 1.a
	
*/

/*
	 Essentially it sets up an |interrupt| when the |IEEE 802.15.4| 
	 |start-frame-delimiter| (SFD) is |received| and use that to turn 
	 on |radio test mode| for a |short time| to |interfere| with |any| 
	 |packet in the air|.
	
	frame single network package digital data


	Ethernet package consist of:
		* Preamble
			Giving the device reciving incoming frame. A heads up a frame is comming. 

		* Start frame delimeter (SFD)
			One byte long and starts ends in a 11
			It is the real heads up for the reciving device that a ethernet frame is incoming

	[Pre-amble, SFD, Mac-address, source,etc]
	
*/
