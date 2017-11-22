//////////////////////////////////
/*
P12 LED display driver for AR9331 SoC
With ATMega328 videoframe driver chip

(c) Nyakotronics ^_^//, aderod
*/
//////////////////////////////////

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include "timer.h"
#include "gpio.h"
#include "render.h"



#include <ft2build.h>
#include FT_FREETYPE_H

#define BUFLEN 512


static char subbuf[64];

static enum isp_t {P10,P12,P08} isp;

static enum textstyle_t {NORMAL,BLINK} textstyle;


/* origin is the upper left corner */

static volatile uint8_t visible;


void timer_handler(void);


void diep(char *s)
{
    perror(s);
    exit(1);
}



void timer_handler(void)
{
	int i;
	visible++;
	if (textstyle==BLINK)
	{
		if (visible%2==1)
		{
			render_text((char *)&subbuf);
		}else{
			render_text(" ");
	
		}
	}
}




int main(int argc, char *argv[])
{
    struct sockaddr_in si_me, si_other;
    int s, i, slen=sizeof(si_other);
    int datalen,packet_length;
    char buf[BUFLEN];
    int port=0;
    char hostid[256];
    char serverip[256];
	
    visible=0;    
	
    if(start_timer(1000, &timer_handler))
    {
      printf("\n timer error\n");
      return(1);
    }
   
  if (argc > 1)
    {
      for (int count = 1; count < argc; count++)
	{
	  printf("argv[%d] = %s\n", count, argv[count]);
	  if (strcmp(argv[count],"-p")==0)
		{
			if (count<(argc-1))
			{
				port=atoi(argv[count+1]);
			}
		}
   	  if (strcmp(argv[count],"-h")==0)
		{
			if (count<(argc-1))
			{
				memset(hostid, 0, sizeof(hostid));
			   	strcpy(hostid, argv[count+1]);
			}

		}
	
	  if (strcmp(argv[count],"-s")==0)
		{
			if (count<(argc-1))
			{
				memset(serverip, 0, sizeof(serverip));
			   	strcpy(serverip, argv[count+1]);
			}
		}

	  if (strcmp(argv[count],"-i")==0)
		{
			if (count<(argc-1))
			{
				if (strcmp(argv[count+1],"P10")==0)
				{
					isp=P10;
				}else
				if (strcmp(argv[count+1],"P12")==0)
				{
					isp=P12;
				}else 
				if (strcmp(argv[count+1],"P08")==0)
				{
					isp=P08;
				}

			}
		}

	}
    }
  else
    {
        printf("The command had no other arguments.\n");
	return 0;
    }
	
    if (port==0)
	{
        printf("Invalid port\n");
	return 0;
	}

    //GPIO SETUP->


  
    //<-GPIO SETUP

    //OPEN UDP SERVER -->
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
      diep("socket");

    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(port);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s, (struct sockaddr*)&si_me, sizeof(si_me))==-1)
        diep("bind");
    //Send to display empty frame
	clear_buffer();
	normalize_buffer();
	send_buffer_spi();

    render_text("TSTЩ123");
    printf("Wait packets...\n");	
    //WAIT PACKETS -->
    while (1) {

	
      if ((packet_length=recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)&si_other, &slen))<0)
	{	
	if (errno == EINTR) continue;
        diep("recvfrom()");
	
	}
      
      if ((buf[0]=='V') && (buf[1]=='F'))  //videoframe ^_^//
	{
		printf("VF PACKET GOTCHA!\n");
		if (buf[2]=='D') //data frame!
			{
				printf("DATA FRAME:\n");
				memcpy(subbuf, &buf[3], 4);
				subbuf[4]='\0';
				datalen=atoi((char *)&subbuf);
				if (datalen<257) //ugly check
				{
					
					//memcpy(databuffer,(uint8_t *)&buf[7],datalen);
					normalize_buffer();
					send_buffer_spi();

				}
			}
		if (buf[2]=='E') //echo frame
			{
				sprintf(buf,"VF ECHO:%s\n",hostid);
				if (sendto(s, buf, strlen(buf), 0, (struct sockaddr*) &si_other, slen) == -1)
  				      {
				            diep("sendto()");
				      }
			
			}
		if (buf[2]=='T') //text frame frame
			{
				memcpy(subbuf, &buf[3], packet_length-4);
				subbuf[packet_length-4]='\0';
				sprintf(buf,"VF TEXT@%s:%s\n",hostid,subbuf);
				if (sendto(s, buf, strlen(buf), 0, (struct sockaddr*) &si_other, slen) == -1)
  				      {
				            diep("sendto()");
				      }
				render_text((char *)&subbuf);
				textstyle=NORMAL;
			}
		if (buf[2]=='I') //text frame frame blink
			{
				memcpy(subbuf, &buf[3], packet_length-4);
				subbuf[packet_length-4]='\0';
				sprintf(buf,"VF TEXT@%s:%s\n",hostid,subbuf);
				if (sendto(s, buf, strlen(buf), 0, (struct sockaddr*) &si_other, slen) == -1)
  				      {
				            diep("sendto()");
				      }
				render_text((char *)&subbuf);
				textstyle=BLINK;
			}
		if (buf[2]=='C') //clear
			{
				memcpy(subbuf, &buf[3], packet_length-4);
				subbuf[packet_length-4]='\0';
				sprintf(buf,"VF TEXT@%s:%s\n",hostid,subbuf);
				if (sendto(s, buf, strlen(buf), 0, (struct sockaddr*) &si_other, slen) == -1)
  				      {
				            diep("sendto()");
				      }
				render_text(" ");
				textstyle=NORMAL;
			}
	}
      
      printf("Received packet from %s:%d\nData: %s\n\n", 
             inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), buf);
    }

    close(s);
    return 0;
}
