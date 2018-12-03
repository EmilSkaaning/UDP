#include "radio.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include<arpa/inet.h>
#include<sys/socket.h>

#define TIMEOUT_SEC   10


int snd_addr = 2132;
int rcv_addr = 2135;

int main(){
    int err, last, addr;
	char type;
	char data[FRAME_PAYLOAD_SIZE];

	// User input for sender or reciever
	printf("Type of radio (s or r):\n");
	scanf("%c", &type);	

	// Set address
	if (type == 's'){
		addr = snd_addr;
	}
	else{
		addr = rcv_addr;
	}

	// Initialize node
    if ((err=radio_init(addr)) != ERR_OK) {
        printf("Radio could not be initialized: %d\n", err);
        return 1;
	}

	// FSM as sender
	if (type == 's'){
		char sender = 'i';
		while(1){

			memset((char *) &data, 0 , FRAME_PAYLOAD_SIZE);

			// preamble for the dataframe
			char *pre = (char*) malloc(20);
		    pre = "AAAAAAAAAAAAAAAAAAAA";
		    strcpy(data, pre);

	        char *magickey = (char*) malloc(8);
	        magickey = "1337F542";
	        strcat(data, magickey);


			switch(sender){
				case 'i':  // Idle
					printf("case (send): idle\n");

					// Payload
			        char *payload = (char*) malloc(144);
			        printf("Type the payload (integer):\n");
			        scanf("%s",payload); 

			        printf("payload: %s\n", payload);

			        // Header
			        char *header = (char*) malloc(12); 
			        sprintf(header, "%02x", strlen(payload));
			        printf("header: %s\n", header);


			        int checksum;
			        char *hex_sum = (char*) malloc(8);
			        checksum = 0;
			        sprintf(hex_sum, "%08x", checksum);
			        strcat(header, hex_sum);
			        printf("checksum: %s\n", header);

			        int id;
			        char *hex_id = (char*) malloc(1);
			        id = 1;
			        sprintf(hex_id, "%01x", id);
			        strcat(header, hex_id);
			        printf("id: %s\n", header);

			        int flag;
			        char *hex_flag = (char*) malloc(1);
			        flag = 0;
			        sprintf(hex_flag, "%01x", flag);
			        strcat(header, hex_flag);
			        printf("flag: %s\n", header);

			        strcat(data, header);
			        strcat(data, payload);
			        printf("data: %s\n", data);	

					sender = 's';

				case 's':  // Send
					printf("case (send): start\n");

			         if ( (err=radio_send(rcv_addr, data, FRAME_PAYLOAD_SIZE)) != ERR_OK) {
			            printf("radio_send failed with %d\n", err);
						return 1;
	     		    }
					sender = 'i';
	/*			case 'send_data':
				case 'done':
				case 'end':
					break;
	*/		}
		}
	}

	// FSM as reciever
	if (type == 'r'){
		char reciever = 'r';
	    char buf[FRAME_PAYLOAD_SIZE + 1];
 	
 		int err, len;
    	int source;

		while(1){
			switch(reciever){
				case 'r':  // Ready
					memset((char *) buf, 0, FRAME_PAYLOAD_SIZE+1);
					printf("case (rec): ready\n");
		            if ( (len=radio_recv(&source, buf, TIMEOUT_SEC * 1000)) < 0) {
			            if (len == ERR_TIMEOUT) {
			                printf("radio_recv timed out\n");
			                continue;
			            }
		            printf("radio_recv failed with %d\n", len);
		            return 1;
	       			}

	       			printf("Received message: %s\n", buf);

			        char *preamble = (char*) malloc(20); 
			        strncpy(preamble, buf, 20);
			        printf("Received preamble (hex): %s\n", preamble);

			        char *magickey = (char*) malloc(8); 
			        strncpy(magickey, buf+20, 8);
			        printf("Received magickey (hex): %s\n", magickey);

			        char *header = (char*) malloc(12); 
			        strncpy(header, buf+28, 12);
			        printf("Received header (hex): %s\n", header);

			        char *pay_len = (char*) malloc(2);
			        strncpy(pay_len, header, 2);
			        //int pay_lenint = (int)strtol(&pay_len, NULL, 16);
			        printf("Received pay_len : %s\n", pay_len);
			        //printf("Received pay_lenint : %s\n", pay_lenint);

			        char *checksum = (char*) malloc(8);
			        strncpy(checksum, header+2, 8);
			        printf("Received checksum (hex): %s\n", checksum);

			        char *id = (char*) malloc(1);
			        strncpy(id, header+10, 1);
			        printf("Received id: %s\n", id);

			        char *flag = (char*) malloc(1);
			        strncpy(flag, header+11, 1);        
			        printf("Received flag: %s\n", flag);        

			        char *pakage = (char*) malloc(144); 
			        strncpy(pakage, buf+40, 144);
			        //int pakage_int = (int)strtol(&pakage, NULL, 16);    
			        printf("Received pakage (int): %s\n", pakage);
	/*			case 'recieve_data':
				case 'ack':
					break;
	*/
			}
		}
	}
}