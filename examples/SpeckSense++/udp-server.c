/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

#include "contiki.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"

#include "sys/log.h"
#include "dev/radio.h"
#include "net/netstack.h"

// #include "nrf52840_bitfields.h"
// #include "nrf52840.h"

#include "dev/leds.h"
#include "dev/etc/rgb-led/rgb-led.h"

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  0
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

#define INIT_MSG 1337


static struct simple_udp_connection udp_conn;

/**/
typedef struct {
    const uip_ipaddr_t* address;//[25];
    int ID;
} tTuple;
/**/


static char str[32];
static tTuple IDs[5];
static int IDs_length = 5;
static int ID_counter =0;



PROCESS(udp_server_process, "UDP server");
AUTOSTART_PROCESSES(&udp_server_process);
/*---------------------------------------------------------------------------*/
static void 
check_address(const uip_ipaddr_t* rec_address)  //fd00::f6ce:3699:46c9:dcc4   25
{
    LOG_INFO("START FUNC \n");

  for(int i = 0; i < IDs_length; i++)
  {
    if(IDs[i].address == rec_address) /*Incase a UDP sends a message twice, don't add same address twice*/
    {
      break;
    }
    if(IDs[i].ID == -1)  /*Check if the array has an empty spot*/
    {
      //snprintf(rec_address, sizeof(rec_address), "Dobido %d", 1);
      IDs[i].address = rec_address;
      LOG_INFO("Added: ");
      LOG_INFO_6ADDR(IDs[i].address);
      LOG_INFO("\n");

      IDs[i].ID = ID_counter;
      ID_counter +=1;
      // printf(IDs[i].address);
      // printf("\n");
      break;
    }
    else
    {
      LOG_INFO("NOPE");
    }
    //IDs[i]. =
  }
    LOG_INFO("END FUNC \n");

}
/*---------------------------------------------------------------------------*/
static void
udp_rx_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
rgb_led_set(RGB_LED_GREEN);
  LOG_INFO("YEAH\n");
  LOG_INFO("Received request '%.*s' from ", datalen, (char *) data);
  LOG_INFO_6ADDR(sender_addr);
  LOG_INFO_("\n");
  LOG_INFO("This is the Server! \n");
  
  //if((char *) data)
  // printf(data);
 
  // char* test = (char *) data;
  int int_msg;
  sscanf((char *) data, "%d", &int_msg);
  printf("What is this: ");
  printf("%d", int_msg);
  printf("\n");
  if(int_msg == INIT_MSG) // Add the node information to the server.
  {
    LOG_INFO("YES! \n");
    check_address(sender_addr);
  }
  else
  {
    LOG_INFO("NO! \n");

  }

  snprintf(str, sizeof(str), "From Server %d", 222);
  simple_udp_sendto(&udp_conn, str, strlen(str), sender_addr);

  /*
    if address doesn't exist in the ID array
    add it.
    otherwise don't;
  */
rgb_led_off();
#if WITH_SERVER_REPLY
  /* send back the same string to the client as an echo reply */
  LOG_INFO("Sending response.\n");
  simple_udp_sendto(&udp_conn, data, datalen, sender_addr);
#endif /* WITH_SERVER_REPLY */
}

/*---------------------------------------------------------------------------*/
// static void
// server_init_ID(int iter,const uip_ipaddr_t *sender_addr)
// {
//   snprintf(str, sizeof(str), "ID %d", IDs[iter].ID);
//   simple_udp_sendto(&udp_conn, str, strlen(str), sender_addr);

// }

//static int 


/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server_process, ev, data)
{
  static struct etimer et;
  PROCESS_BEGIN();

  etimer_set(&et, CLOCK_SECOND);

  NETSTACK_MAC.off();
  NETSTACK_RADIO.on();
  if (NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, 26) != RADIO_RESULT_OK)
  {
    printf("ERROR: failed to change radio channel\n");
    break;
  }
  LOG_INFO("\nStart of process!");
  //Init ID arr
  for(int i = 0; i < IDs_length; i++)
  {
    IDs[i].ID = -1;

  }
  /* Initialize DAG root */
  if(NETSTACK_ROUTING.root_start() == 0)
  {
    LOG_INFO("DAG root initialize");
  }

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL,
                      UDP_CLIENT_PORT, udp_rx_callback);

  

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/


/*
  Client connect to server.

  Server store the client addres in a array. 

  Server got address to all different nodes. 

  Server is now able to connect to a specific node. 





*/

/*
 *  Set up basic udp-rpl connection. 
 *  Created some
 *  Looked at openThread
 *  Looked some at the RSSI things. 
 * 
 */
