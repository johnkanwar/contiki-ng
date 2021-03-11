/*
*	Proactive Jammer with support of direct test mode(DTM)
*/


#include "contiki.h"

#include <stdio.h>
#include "etimer.h"
#include "ble_dtm.h"
 
/*---------------------------------------------------------------------------*/
PROCESS(reactive_process, "reactive process");
AUTOSTART_PROCESSES(&reactive_process);
/*---------------------------------------------------------------------------*/

static uint32_t
dtm_cmd_put(void)
{

	dtm_cmd_t command_code = LE_TRANSMITTER_TEST;
	dtm_freq_t freq = 39; //This is channel not frequency
	uint32_t length = 255;
	dtm_pkt_type_t payload = DTM_PKT_PRBS9;
	return dtm_cmd(command_code, freq, length, payload);
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(reactive_process, ev, data)
{
	static struct etimer et;
	PROCESS_BEGIN(); // Start process // Only variables above this line.
	etimer_set(&et, CLOCK_SECOND * 0.01);

	if (dtm_init() != DTM_SUCCESS)	// <-- dtm_init causes the restarts, but is neccessary in order to active dtm. 	
	{
		// If DTM cannot be correctly initialized, then we just return.
		return -1;
	}
	
	while (1)
	{
		dtm_cmd_put();
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		etimer_reset(&et);
		PROCESS_PAUSE();
	}
	PROCESS_END();
}
