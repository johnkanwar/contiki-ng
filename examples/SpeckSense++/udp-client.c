#include "contiki.h"
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
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY 1
#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

#define SEND_INTERVAL (60 * CLOCK_SECOND)

#define INIT_MSG a

static struct simple_udp_connection udp_conn;
static uip_ipaddr_t dest_ipaddr;

static int ID;
static int init = 0;
static int cnt = 0;
/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/
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

PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer et;
  static unsigned count;
  static char str[32];

  PROCESS_BEGIN();
  printf("Does this happen multiple times ? ");

  NETSTACK_MAC.off();
  NETSTACK_RADIO.on();
  if (NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, 26) != RADIO_RESULT_OK)
  {
    printf("ERROR: failed to change radio channel\n");
    break;
  }
  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);

  etimer_set(&et, CLOCK_SECOND * 0.5);
  while (1)
  {

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    etimer_reset(&et);

    if(NETSTACK_ROUTING.node_is_reachable())
    {
      printf("First one down\n");
      if(NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr))
      {

        if (!init)
        {
          LOG_INFO("Init function!! \n");

          /*Send init*/
          udp_init_send();
          init = 1;
        }
        else
        {

	/*Flash led light green*/
	  rgb_led_set(RGB_LED_GREEN);
          /* Send to DAG root */
          LOG_INFO("This is the client! \n");
          LOG_INFO("Sending request %u to ", count);
          LOG_INFO_6ADDR(&dest_ipaddr);
          LOG_INFO_("\n");
          snprintf(str, sizeof(str), "5 %d", count);
          simple_udp_sendto(&udp_conn, str, strlen(str), &dest_ipaddr);
          count++;
        }
      }
      else
      {
        LOG_INFO("Didn't get root_ipaddr\n");
      }

      // if()
    }
    else
    { 
      rgb_led_set(RGB_LED_RED);
      LOG_INFO("Not reachable yet: %d\n", cnt);
      cnt++;
    }

    rgb_led_off();
    /* Add some jitter */
    // etimer_set(&periodic_timer, SEND_INTERVAL
    //   - CLOCK_SECOND + (random_rand() % (2 * CLOCK_SECOND)));
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

