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
    int err, last, addr, len, source;
	char type;

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
		char data_s[FRAME_PAYLOAD_SIZE];
	    char buf_s[FRAME_PAYLOAD_SIZE + 1];		
		char header[13]; 	    	
		char sender = 'i';

		char *payload = (char*) malloc(144);
		char *pre = (char*) malloc(20);
		char *magickey = (char*) malloc(8);
		char *hex_sum = (char*) malloc(8);
		char *hex_id = (char*) malloc(1);
		char *hex_flag = (char*) malloc(1);
        char *ph = (char*) malloc(1);
		int checksum, id, flag, flag_rec, flag_data;
		while(1){

			switch(sender){
				case 'i':  // Idle
					printf("case (send): idle\n");
					memset((char *) &data_s, 0 , FRAME_PAYLOAD_SIZE);

					// Payload

			        printf("Type the payload (integer):\n");
			        scanf("%s",payload); 
			        // printf("payload: %s\n", payload);

					sender = 's';
					break;

				case 's':  // Send
					printf("case (send): start\n");


					// preamble for the data_sframe

				    pre = "AAAAAAAAAAAAAAAAAAAA";
				    strcpy(data_s, pre);
			   

			        magickey = "1337F542";
			        strcat(data_s, magickey);
			   
			        // Header

			        sprintf(header, "%02lx", strlen(payload));
			        // printf("header: %s\n", header);
			   

			        checksum = 0;
			        sprintf(hex_sum, "%08x", checksum);
			        strcat(header, hex_sum);
			        // printf("checksum: %s\n", header);
			   
			        id = 1;
			        sprintf(hex_id, "%01x", id);
			        strcat(header, hex_id);
			        // printf("id: %s\n", header);

			   
			        flag = 8;
			        sprintf(hex_flag, "%01x", flag);
			        strcat(header, hex_flag);
			        // printf("flag: %s\n", header);
			   
			        strcat(data_s, header);
			   
			        if ( (err=radio_send(rcv_addr, data_s, FRAME_PAYLOAD_SIZE)) != ERR_OK) {
			            printf("radio_send failed with %d\n", err);
						return 1;
	     		    }
	     		    printf("send data: %s\n", data_s);
	
			        sender = 'w';
			        break;
/*	     		    
		            if ( (len=radio_recv(&source, buf_s, TIMEOUT_SEC * 1000)) < 0) {
			            if (len == ERR_TIMEOUT) {
			                printf("radio_recv timed out\n");
			                continue;
			            }
		            printf("radio_recv failed with %d\n", len);
		            return 1;
	       			}			 		

					printf("Buf_s: %s \n", buf_s);

			        strncpy(ph, buf_s+39, 1);  
			        flag = atoi(ph);      		 

			        if (flag == 4){
			        	sender ='d';
			        	printf("Ack recieved sending data\n");
			        }
			        else{
			        	break;
			        }
*/
				case 'd':
					printf("case (send): send payload\n");

					memset((char *) &data_s, 0 , FRAME_PAYLOAD_SIZE);
					memset((char *) &header, 0 , 13);
					// preamble for the data_sframe

				    strcpy(data_s, pre);
			        strcat(data_s, magickey);	

			        sprintf(header, "%02lx", strlen(payload));
	     		    
			        checksum = 0;
			        sprintf(hex_sum, "%08x", checksum);
			        strcat(header, hex_sum);
			        // printf("checksum: %s\n", header);
	     		    
			        id = 1;
			        // sprintf(hex_id, "%01x", id);
			        strcat(header, hex_id);
			        // printf("id: %s\n", header);
	     		    
			        flag = 0;
			        sprintf(hex_flag, "%01x", flag);
			        strcat(header, hex_flag);
			        // printf("flag: %s\n", header);
	     		    
			        strcat(data_s, header);
			        strcat(data_s, payload);
			        //printf("data_s: %s\n", data_s);

			        // Send pakage
			        if ( (err=radio_send(source, data_s, FRAME_PAYLOAD_SIZE)) != ERR_OK) {
			            printf("radio_send failed with %d\n", err);
						return 1;
	     		    }

					printf("sending data: %s\n", data_s);

					sender = 'w';  // Going back to idle			        
					break;

				case 'w':
					printf("case (send): waiting for ack\n");
		            if ( (len=radio_recv(&source, buf_s, TIMEOUT_SEC * 1000)) < 0) {
			            if (len == ERR_TIMEOUT) {
			                printf("radio_recv timed out\n");
			                continue;
			            }
		            printf("radio_recv failed with %d\n", len);
		            return 1;
	       			}			 		

					printf("Buf_s: %s \n", buf_s);

			        strncpy(ph, buf_s+39, 1);  
			        flag_rec = atoi(ph);      		 

			        printf("flag: %d, flag_rec: %d, flag_data: %d \n", flag, flag_rec, flag_data);
			        if (flag == 8 && flag_rec == 4){
			        	sender ='d';
			        	printf("Ack recieved sending data\n");
			        }
			        else if (flag == 0 && flag_rec == 4){
			     		sender = 't';   	
			     		printf("Ack recieved terminating\n");
			        }
			        else if (flag == 2 && flag_rec == 4){
			        	sender = 'i'; 					
			        }	
			        else{
			        	printf("%s\n", "Should not happen");
			        	sender = 'd';
			        }		        
					break;

				case 't':
					printf("case (send): terminating, waiting for ack\n");
					memset((char *) &data_s, 0 , FRAME_PAYLOAD_SIZE);
					// preamble for the data_sframe

				    pre = "AAAAAAAAAAAAAAAAAAAA";
				    strcpy(data_s, pre);
			   

			        magickey = "1337F542";
			        strcat(data_s, magickey);
			   
			        // Header

			        sprintf(header, "%02lx", strlen(payload));
			        // printf("header: %s\n", header);
			   

			        checksum = 0;
			        sprintf(hex_sum, "%08x", checksum);
			        strcat(header, hex_sum);
			        // printf("checksum: %s\n", header);
			   
			        id = 1;
			        sprintf(hex_id, "%01x", id);
			        strcat(header, hex_id);
			        // printf("id: %s\n", header);

			   
			        flag = 2;
			        sprintf(hex_flag, "%01x", flag);
			        strcat(header, hex_flag);
			        // printf("flag: %s\n", header);
			   
			        strcat(data_s, header);
			   
			        if ( (err=radio_send(rcv_addr, data_s, FRAME_PAYLOAD_SIZE)) != ERR_OK) {
			            printf("radio_send failed with %d\n", err);
						return 1;
	     		    }

	     		    sender = 'i';
					break;
			}
		}
	}

	// FSM as reciever
	if (type == 'r'){
		char reciever = 'r';
	    char buf_r[FRAME_PAYLOAD_SIZE + 1];
 		char data_r[FRAME_PAYLOAD_SIZE];
 		int err, len, source, flag_int, flag_rec, checksum_int, id_int;
        char *preamble = (char*) malloc(20); 
        char *magickey = (char*) malloc(8);
		char *magickey_data = (char*) malloc(8);  
        char *header = (char*) malloc(12); 
        char *pay_len = (char*) malloc(2);
        char *checksum = (char*) malloc(8);
        char *id = (char*) malloc(1);
		char *flag = (char*) malloc(1);
		char *pakage = (char*) malloc(144);
		char *hex_sum = (char*) malloc(8);
		char *hex_id = (char*) malloc(1);
		char *hex_flag = (char*) malloc(1);

		while(1){
			switch(reciever){
				case 'r':  // Ready
					memset((char *) buf_r, 0, FRAME_PAYLOAD_SIZE+1);
					printf("case (rec): ready\n");
		            if ( (len=radio_recv(&source, buf_r, TIMEOUT_SEC * 1000)) < 0) {
			            if (len == ERR_TIMEOUT) {
			                printf("radio_recv timed out\n");
			                continue;
			            }
		            printf("radio_recv failed with %d\n", len);
		            return 1;
	       			}

	       			printf("Received message: %s\n", buf_r);


			        strncpy(preamble, buf_r, 20);
			        // printf("Received preamble (hex): %s\n", preamble);


			        strncpy(magickey, buf_r+20, 8);
			        // printf("Received magickey (hex): %s\n", magickey);


			        strncpy(header, buf_r+28, 12);
			        // printf("Received header (hex): %s\n", header);


			        strncpy(pay_len, header, 2);
			        // printf("Received pay_len : %s\n", pay_len);


			        strncpy(checksum, header+2, 8);
			        // printf("Received checksum (hex): %s\n", checksum);


			        strncpy(id, header+10, 1);
			        // printf("Received id: %s\n", id);


			        strncpy(flag, header+11, 1);        
			        // printf("Received flag: %s\n", flag);        

			        flag_rec = atoi(flag);

			        if (flag_rec == 8 || flag_rec == 2){
			        	reciever = 'a';
			        }
			        else{
			        	reciever = 'r';
			        } 

			        break;
/*
			        char *pakage = (char*) malloc(144); 
			        strncpy(pakage, buf_r+40, 144); */
			        //printf("Received pakage (int): %s\n", pakage);
				case 'a': // Acknowledgement
					printf("case (rec): acknowledgement\n");
					// preamble for the data_sframe

				    strcpy(data_r, preamble);

			        magickey = "1337F541";
			        strcat(data_r, magickey);	

			        // Header 
			        sprintf(header, "%02x", 0);
			        // printf("header: %s\n", header);

			        checksum_int = 0;
			        sprintf(hex_sum, "%08x", checksum_int);
			        strcat(header, hex_sum);
			        // printf("checksum: %s\n", header);


			        id_int = 0;
			        sprintf(hex_id, "%01x", id_int);
			        strcat(header, hex_id);
			        // printf("id: %s\n", header);
			        printf("flag_rec %d\n", flag_rec);
			        if (flag_rec == 8){
	     		   		reciever = 'd';
			        }
			        else if (flag_rec == 2){
			        	reciever = 't';
			        }
			        else if (flag_rec == 0){
			        	reciever = 'w';
			        }
			        else{
			        	reciever = 'r';
			        }

			        flag_int = 4;
			        sprintf(hex_flag, "%01x", flag_int);
			        strcat(header, hex_flag);
			        // printf("flag: %s\n", header);

			        strcat(data_r, header);

			        if ( (err=radio_send(source, data_r, FRAME_PAYLOAD_SIZE)) != ERR_OK) {
			            printf("radio_send failed with %d\n", err);
						return 1;
	     		    }		
	     		    printf("Sending ack: %s\n", data_r);
	     		    break;
				case 'd':

					memset((char *) buf_r, 0, FRAME_PAYLOAD_SIZE+1);
					printf("case (rec): recieve data\n");
		            if ( (len=radio_recv(&source, buf_r, TIMEOUT_SEC * 1000)) < 0) {
			            if (len == ERR_TIMEOUT) {
			                printf("radio_recv timed out\n");
			                continue;
			            }
		            printf("radio_recv failed with %d\n", len);
		            return 1;
	       			}

	       			printf("Received data: %s\n", buf_r);


			        strncpy(preamble, buf_r, 20);
			        // printf("Received preamble (hex): %s\n", preamble);

      				strncpy(magickey_data, buf_r+20, 8);

			        // printf("Received magickey (hex): %s\n", magickey);

			        strncpy(header, buf_r+28, 12);
			        // printf("Received header (hex): %s\n", header);

			        strncpy(pay_len, header, 2);
			        // printf("Received pay_len : %s\n", pay_len);

			        strncpy(checksum, header+2, 8);
			        // printf("Received checksum (hex): %s\n", checksum);

			        strncpy(id, header+10, 1);
			        // printf("Received id: %s\n", id);

			        strncpy(flag, header+11, 1);        
			        // printf("Received flag: %s\n", flag);       
			        flag_rec = atoi(flag);

					strncpy(pakage, buf_r+40, 144);
					printf("flag_rec: %d\n", flag_rec);
					if (atoi(pay_len) == strlen(pakage) && flag_rec == 0){
						printf("All data recieved: %s\n", pakage);
						reciever = 'a';
					}
					else if (atoi(pay_len) == strlen(pakage) && flag_rec == 2){
						printf("All data recieved: %s\n", pakage);
						reciever = 't';
					}

					break;
				case 'w':
					printf("%s\n", "Waiting for termination");

					memset((char *) buf_r, 0, FRAME_PAYLOAD_SIZE+1);
					printf("case (rec): recieve data\n");
		            if ( (len=radio_recv(&source, buf_r, TIMEOUT_SEC * 1000)) < 0) {
			            if (len == ERR_TIMEOUT) {
			                printf("radio_recv timed out\n");
			                continue;
			            }
		            printf("radio_recv failed with %d\n", len);
		            return 1;
	       			}					

			        strncpy(preamble, buf_r, 20);
			        // printf("Received preamble (hex): %s\n", preamble);

      				strncpy(magickey_data, buf_r+20, 8);

			        // printf("Received magickey (hex): %s\n", magickey);

			        strncpy(header, buf_r+28, 12);
			        // printf("Received header (hex): %s\n", header);

			        strncpy(pay_len, header, 2);
			        // printf("Received pay_len : %s\n", pay_len);

			        strncpy(checksum, header+2, 8);
			        // printf("Received checksum (hex): %s\n", checksum);

			        strncpy(id, header+10, 1);
			        // printf("Received id: %s\n", id);

			        strncpy(flag, header+11, 1);
			        flag_rec = atoi(flag);        
			        // printf("Received flag: %s\n", flag); 

			        if (flag_rec == 2){
			        	reciever = 'a';
			        }
			        else{
			        	printf("It did not terminate! flag_rec should be 2 but is %d\n", flag_rec);
			        }

				case 't':
					printf("%s\n", "Det var det ");
					reciever = 'r';
					break;

	
			}
		}
	}
}