#include "radio.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include<arpa/inet.h>
#include<sys/socket.h>

#define TIMEOUT_SEC   10


int snd_addr = 2132;
int rcv_addr = 2135;

int checksum_fail = 0;
int payload_len_fail = 0;
int no_packages = 2;

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
		char *hex_payload = (char*) malloc(144);
		char *pre = (char*) malloc(20);
		char *magickey = (char*) malloc(8);
		char *rec_magic = (char*) malloc(8);
		char *hex_sum = (char*) malloc(8);
		char *sum = (char*) malloc(8);
        char *com_checksum = (char*) malloc(8);		
		char *hex_id = (char*) malloc(1);
		char *hex_flag = (char*) malloc(1);
        char *ph = (char*) malloc(1);
		int checksum, rec_checksum, id, flag, flag_rec, int_pay, gen_magic, magic_check, ack_count;
		while(1){

			switch(sender){
				case 'i':  // Idle
					printf("case (send): idle\n");
					ack_count = 0;
					memset((char *) &data_s, 0 , FRAME_PAYLOAD_SIZE);

					// Payload
			        printf("Type the payload (integer):\n");
			        scanf("%s",payload); 
			        printf("payload: %s\n", payload);
			        int_pay = strtol(payload,NULL,10);
			        sprintf(hex_payload, "%x", int_pay);
			        //printf("THIS IS HEX: %s\n", hex_payload);		

					gen_magic = magickey_gen();
					sprintf(magickey, "%08x", gen_magic);
					sender = 's';
					break;

				case 's':  // Send
					printf("case (send): start\n");


					// preamble for the data_sframe
					strcpy(pre, "AAAAAAAAAAAAAAAAAAAA");
				    //pre = "AAAAAAAAAAAAAAAAAAAA";
				    strcpy(data_s, pre);
			   	
			        strcat(data_s, magickey);
			   
			        // Header

			        sprintf(header, "%02lx", strlen(hex_payload));
			        // printf("header: %s\n", header);

			        sprintf(hex_sum, "%08x", 0);
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
			        sprintf(data_s+40, "%02x", no_packages);
				    // CHECKSUM TESTING //
			   		printf("%s\n", data_s);
			        checksum = 0;
				    for(int i = 0; i < strlen(data_s); i++){
				    	checksum = checksum + data_s[i];
				    }

				    memset((char *) &data_s, 0 , FRAME_PAYLOAD_SIZE);
				    memset((char *) &header, 0 , 12);
				    strcpy(data_s, pre);
				    strcat(data_s, magickey);
				    if (payload_len_fail == 1){
				    	sprintf(header, "%02x", 200);
				    }else{
				  		sprintf(header, "%02lx", strlen(hex_payload));
			        }
			        sprintf(hex_sum, "%08x", checksum);
			        strcat(header, hex_sum);
			        strcat(header, hex_id);		   
			        strcat(header, hex_flag);
			        strcat(data_s, header);
			        sprintf(data_s+40, "%02x", no_packages);
			   	
			   		//////////////////////

			   
			        if ( (err=radio_send(rcv_addr, data_s, strlen(data_s))) != ERR_OK) {
			            printf("radio_send failed with %d\n", err);
						return 1;
	     		    }
	     		    printf("send data: %s\n", data_s);
	
			        sender = 'w';
			        break;

				case 'd':
					printf("case (send): send payload\n");

					memset((char *) &data_s, 0 , FRAME_PAYLOAD_SIZE);
					memset((char *) &header, 0 , 13);
					// preamble for the data_sframe

				    strcpy(data_s, pre);
			        strcat(data_s, magickey);	

			        sprintf(header, "%02lx", strlen(hex_payload));
	     		    
			        sprintf(hex_sum, "%08x", 0);
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

			        strcat(data_s, hex_payload);
			        //printf("data_s: %s\n", data_s);

				    // CHECKSUM TESTING //
			
			        checksum = 0;
				    for(int i = 0; i < strlen(data_s); i++){
				    	checksum = checksum + data_s[i];
				    }

				    memset((char *) &data_s, 0 , FRAME_PAYLOAD_SIZE);
				    memset((char *) &header, 0 , 12);
				    strcpy(data_s, pre);
				    strcat(data_s, magickey);
				  	sprintf(header, "%02lx", strlen(hex_payload));
			        sprintf(hex_sum, "%08x", checksum);
			        strcat(header, hex_sum);
			        strcat(header, hex_id);		   
			        strcat(header, hex_flag);
			        strcat(data_s, header);
			        strcat(data_s, hex_payload);
		
			   		//////////////////////

			        // Send pakage
			        if ( (err=radio_send(source, data_s, strlen(data_s))) != ERR_OK) {
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

					// Read magic key, should be equal to its own
			        strncpy(rec_magic, buf_s+20, 8);  
			        strncpy(header, buf_s+28, 12);
			        strncpy(com_checksum, header+2, 8);

			        // check checksum
			        checksum = 0;
			        printf("%s\n", buf_s);
				    for(int i = 0; i < 30; i++){
				    	checksum = checksum + buf_s[i];
				    }
				    for(int i = 38; i < strlen(buf_s); i++){
				    	checksum = checksum + buf_s[i];
				    }				    
				    checksum = checksum + (8 * 48);

					sprintf(sum, "%08x", checksum);

				    if (strcmp(sum, com_checksum) == 0){
				    	printf("%s\n", "Checksum match\n");
				    }
				    else{
				    	printf("Checksum did not match! Resend last pakage\n");
				    }


			        magic_check = strcmp(rec_magic, magickey);
			        if (magic_check != 0 && ack_count <= 5){
			        	printf("Magickey is wrong, trying again\n");
			        	sender = 'w';
			        	ack_count++;
			        	break;
			        }
			        else if (magic_check != 0 && ack_count > 5){
			        	printf("Could not find server. Returning to Idle state!\n");
			        	sender = 'i';
			        	break;
			        }

			        // Read checksum 

					// Read flags
			        strncpy(ph, buf_s+39, 1);  
			        flag_rec = atoi(ph);      		 

			        //printf("flag: %d, flag_rec: %d\n", flag, flag_rec);
			        // Acknowledge commands
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
		       	 	else if (flag == 8 && flag_rec == 1){
			        	sender ='s';
			        	printf("Resending SEND\n");
			        }
			        else if (flag == 0 && flag_rec == 1){
			     		sender = 'd';   	
			     		printf("Resending DATA\n");
			        }
			        else if (flag == 2 && flag_rec == 1){
			        	printf("Resending TERMINATE\n");
			        	sender = 't'; 					
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

					strcpy(pre, "AAAAAAAAAAAAAAAAAAAA");
				    //pre = "AAAAAAAAAAAAAAAAAAAA";
				    strcpy(data_s, pre);

				    //strcpy(magickey, gen_magic);
			        //magickey = "13379542";
			        sprintf(magickey, "%08x", gen_magic);
			        strcat(data_s, magickey);
			   
			        // Header
			        sprintf(header, "%02lx", strlen(hex_payload));
			        sprintf(hex_sum, "%08x", 0);
			        strcat(header, hex_sum);
			        id = 1;
			        sprintf(hex_id, "%01x", id);
			        strcat(header, hex_id);		   
			        flag = 2;
			        sprintf(hex_flag, "%01x", flag);
			        strcat(header, hex_flag);
			        strcat(data_s, header);
				    
				    // CHECKSUM TESTING //
			    

			        checksum = 0;
				    for(int i = 0; i < strlen(data_s); i++){
				    	checksum = checksum + data_s[i];
				    }

				    memset((char *) &data_s, 0 , FRAME_PAYLOAD_SIZE);
				    memset((char *) &header, 0 , 12);
				    strcpy(data_s, pre);
				    strcat(data_s, magickey);
				  	sprintf(header, "%02lx", strlen(hex_payload));
			        sprintf(hex_sum, "%08x", checksum);
			        strcat(header, hex_sum);
			        strcat(header, hex_id);		   
			        strcat(header, hex_flag);
			        strcat(data_s, header);
			   	
			   		//////////////////////			   
			        
			        if ( (err=radio_send(rcv_addr, data_s, strlen(data_s))) != ERR_OK) {
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
 		int err, len, source, flag_int, flag_rec, id_int, number, magic_check, ack_count, checksum, error_resend, pakage_nr_int;
        char *preamble = (char*) malloc(20); 
        char *magickey = (char*) malloc(8);
		char *magickey_data = (char*) malloc(8);  
        char *header = (char*) malloc(12); 
        char *pay_len = (char*) malloc(2);
        char *buf_len = (char*) malloc(2);
        char *com_checksum = (char*) malloc(8);
        char *id = (char*) malloc(1);
		char *flag = (char*) malloc(1);
		char *pakage = (char*) malloc(144);
		char *hex_sum = (char*) malloc(8);
		char *hex_id = (char*) malloc(1);
		char *hex_flag = (char*) malloc(1);
		char *sum = (char*) malloc(8);
		char *pakage_nr = (char*) malloc(2);		
		while(1){
			switch(reciever){
				case 'r':  // Ready
					printf("case (rec): ready\n");	
		            if ( (len=radio_recv(&source, buf_r, 0)) < 0) {
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
			        sprintf(buf_len, "%02x", FRAME_PAYLOAD_SIZE-40);
			        printf("%s %d\n", buf_len, FRAME_PAYLOAD_SIZE-40);
			        printf("GIVE ME NUMBER: %d\n", strcmp(pay_len, buf_len));
			        if (strcmp(pay_len, buf_len) > 0){
			        	printf("%s\n", "ERROR, message size is larger than buffer!");
			        	reciever = 't';
			        	break;
			        }

			        strncpy(com_checksum, header+2, 8);
			        // printf("Received checksum (hex): %s\n", checksum);


			        strncpy(id, header+10, 1);
			        // printf("Received id: %s\n", id);


			        strncpy(flag, header+11, 1);        
			        // printf("Received flag: %s\n", flag);        

			        strncpy(pakage_nr, buf_r+40, 2);
			        pakage_nr_int = atoi(pakage_nr);

			        // check checksum
			        checksum = 0;
			        printf("%s\n", buf_r);
				    for(int i = 0; i < 30; i++){
				    	checksum = checksum + buf_r[i];
				    }
				    for(int i = 38; i < strlen(buf_r); i++){
				    	checksum = checksum + buf_r[i];
				    }				    
				    checksum = checksum + (8 * 48);

					sprintf(sum, "%08x", checksum);
					error_resend = strcmp(sum,com_checksum);
				    if (error_resend == 0){
				    	printf("%s\n", "Checksum match\n");
				    }
				    else{
				    	printf("Checksum did not match! Resend last pakage\n");
				    	reciever = 'a';
				    	break;
				    }

			        flag_rec = atoi(flag);
			        printf("flag_rec: %d\n", flag_rec);
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
				case 'a': // Acknowledgemenpakage_nr_intt
					printf("case (rec): acknowledgement\n");
					// preamble for the data_sframe

				    strcpy(data_r, preamble);

				    //strcpy(magickey, "13372541");
			        //magickey = "13372541";
			        strcat(data_r, magickey);	

			        // Header 
			        sprintf(header, "%02x", 0);
			        // printf("header: %s\n", header);

			        sprintf(hex_sum, "%08x", 0);
			        strcat(header, hex_sum);
			        // printf("checksum: %s\n", header);


			        id_int = 0;
			        sprintf(hex_id, "%01x", id_int);
			        strcat(header, hex_id);

			        // printf("id: %s\n", header);
			        //printf("flag_rec %d\n", flag_rec);
			        //printf("%d\n", (flag_rec == 0));
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

			        if (error_resend != 0){
			        	flag_int = 1;
			        }
			        else{
			        	flag_int = 4;
			        }
			        
			        sprintf(hex_flag, "%01x", flag_int);
			        strcat(header, hex_flag);
			        // printf("flag: %s\n", header);

			        strcat(data_r, header);

				    // CHECKSUM TESTING //
			        checksum = 0;
				    for(int i = 0; i < strlen(data_r); i++){
				    	checksum = checksum + data_r[i];
				    }
				    memset((char *) &data_r, 0 , FRAME_PAYLOAD_SIZE);

				    strcpy(data_r, preamble);

				    strcat(data_r, magickey);

				  	sprintf(header, "%02x", 0);  // strlen(hex_payload)
			        sprintf(hex_sum, "%08x", checksum);

			        strcat(header, hex_sum);
			        strcat(header, hex_id);		   
			        strcat(header, hex_flag);
			        strcat(data_r, header);
			   		//////////////////////	

			        if ( (err=radio_send(source, data_r, strlen(data_r))) != ERR_OK) {
			            printf("radio_send failed with %d\n", err);
						return 1;
	     		    }		
	     		    printf("Sending ack: %s\n", data_r);
	     		    break;

				case 'd':
					ack_count = 0;
					memset((char *) buf_r, 0, FRAME_PAYLOAD_SIZE+1);
			        for(int i = 1; i <= pakage_nr_int; i++){
			        	printf("HEEY: %d\n", i);
				        
						printf("case (rec): recieve data\n");
			            if ( (len=radio_recv(&source, buf_r, TIMEOUT_SEC * 1000)) < 0) {
				            if (len == ERR_TIMEOUT){
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
				        
				        magic_check = strcmp(magickey_data, magickey);
				        if (magic_check != 0 && ack_count <= 5){
				        	printf("Wrong magickey! Trying to connect again.\n");
				        	reciever = 'd';
				        	ack_count++;
				        	break;
				        }
				        else if (magic_check != 0 && ack_count > 5){
				        	printf("Could not find cleint. Returning to Idle state!\n");
				        	reciever = 't';
				        	break;
				        }

				        // printf("Received magickey (hex): %s\n", magickey);

				        strncpy(header, buf_r+28, 12);
				        // printf("Received header (hex): %s\n", header);

				        strncpy(pay_len, header, 2);
				        // printf("Received pay_len : %s\n", pay_len);

				        strncpy(com_checksum, header+2, 8);
				        // printf("Received checksum (hex): %s\n", checksum);

				        strncpy(id, header+10, 1);
				        // printf("Received id: %s\n", id);

				        strncpy(flag, header+11, 1);        
				        // printf("Received flag: %s\n", flag);       
				        
				        // check checksum
				        checksum = 0;
				        printf("%s\n", buf_r);
					    for(int i = 0; i < 30; i++){
					    	checksum = checksum + buf_r[i];
					    }
					    for(int i = 38; i < strlen(buf_r); i++){
					    	checksum = checksum + buf_r[i];
					    }			

					    if (checksum_fail == 1){
					     	checksum = 9;
					     	checksum_fail++;
					    }
					    else{
					    	checksum = checksum + (8 * 48); 
					    }
					    //fail++;
					}
					sprintf(sum, "%08x", checksum);

					error_resend = strcmp(sum, com_checksum);
				    if (error_resend == 0){
				    	printf("%s\n", "Checksum match\n");
				    }
				    else{
				    	printf("Checksum did not match! Resend last pakage\n");
				    	reciever = 'a';
				    	break;				    	
				    }

			        flag_rec = atoi(flag);

					number = (int)strtol(pay_len, NULL, 16);
					//number = (int)strtol(pay_len, NULL, 16);

					strncpy(pakage, buf_r+40, 144);
					printf("flag_rec: %d\n", flag_rec);
					if (number == strlen(pakage) && flag_rec == 0){
						printf("All data recieved: %s\n", pakage);
						reciever = 'a';
					}

					else if (number == strlen(pakage) && flag_rec == 2){
						printf("All data recieved: %s\n", pakage);
						reciever = 't';
					}
					//printf("len: %d %ld\n", number, strlen(pakage));
					break;

				case 'w':
					printf("%s\n", "Waiting for termination");
					ack_count = 0;
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

			        magic_check = strcmp(magickey_data, magickey);
			        if (magic_check != 0 && ack_count <= 5){
			        	printf("Wrong magickey! Trying to connect again.\n");
			        	reciever = 'w';
			        	ack_count++;
			        	break;
			        }
			        else if (magic_check != 0 && ack_count > 5){
			        	printf("Could not find cleint. Terminating!\n");
			        	reciever = 't';
			        	break;
			        }
			        // printf("Received magickey (hex): %s\n", magickey);

			        strncpy(header, buf_r+28, 12);
			        // printf("Received header (hex): %s\n", header);

			        strncpy(pay_len, header, 2);
			        // printf("Received pay_len : %s\n", pay_len);

			        strncpy(com_checksum, header+2, 8);
			        // printf("Received checksum (hex): %s\n", checksum);

			        strncpy(id, header+10, 1);
			        // printf("Received id: %s\n", id);

			        strncpy(flag, header+11, 1);

			        // check checksum
			        checksum = 0;
			        printf("%s\n", buf_r);
				    for(int i = 0; i < 30; i++){
				    	checksum = checksum + buf_r[i];
				    }
				    for(int i = 38; i < strlen(buf_r); i++){
				    	checksum = checksum + buf_r[i];
				    }				    
				    checksum = checksum + (8 * 48);

					sprintf(sum, "%08x", checksum);

				    if (strcmp(sum, com_checksum) == 0){
				    	printf("%s\n", "Checksum match\n");
				    }
				    else{
				    	printf("Checksum did not match! Resend last pakage\n");
				    }

			        flag_rec = atoi(flag);        
			        // printf("Received flag: %s\n", flag); 

			        if (flag_rec == 2){
			        	reciever = 'a';
			        }
			        else{
			        	printf("It did not terminate! flag_rec should be 2 but is %d\n", flag_rec);
			        }

				case 't':
					printf("case (term): terminating\n");
					
					memset((char *) buf_r, 0, FRAME_PAYLOAD_SIZE+1);
					memset((char *) data_r, 0, FRAME_PAYLOAD_SIZE);
					memset((char *) preamble, 0, 20);
					memset((char *) magickey, 0, 8);
					memset((char *) header, 0, 12);
					memset((char *) pay_len, 0, 2);
					memset((char *) com_checksum, 0, 8);
					memset((char *) id, 0, 1);
					memset((char *) flag, 0, 1);
					
					printf("%s\n", magickey);
					reciever = 'r';
					break;

	
			}
		}
	}
}