

#include "contiki.h"

#include <stdio.h> /* For printf() not neccessary now*/
#include "boards.h"
#include "nrf52840.h"

/*---------------------------------------------------------------------------*/
PROCESS(test_process, "Test process");
//PROCESS(Aetimer_process, "Aetimer process");
//PROCESS(led_process, "Led process");
AUTOSTART_PROCESSES(&test_process);

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(test_process, ev, data)
{
	static struct etimer et; // Static timer

	PROCESS_BEGIN(); // Start process // Only variables above this line.

	/* Configure board. */
	bsp_board_init(BSP_INIT_LEDS);
	//bsp_board_init(BSP_INIT_BUTTONS);

	etimer_set(&et, CLOCK_SECOND * 2); // Wait for 2 seconds
	
	bsp_board_led_on(0); // Making sure led works
	while (1)
	{

		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		bsp_board_leds_on();
		etimer_reset(&et);
		bsp_board_leds_on();
	}
	PROCESS_END();
}