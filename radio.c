/*
 * radio.c
 *
 * Emulation of radio node using UDP (skeleton)
 */

// Implements
#include "radio.h"

// Uses
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

int sock, recv_len, s_len;    // UDP Socket used by this node

void die(char *s)
{
        perror(s);
        exit(1);
}

int radio_init(int addr) {

    struct sockaddr_in sa;   // Structure to set own address
    s_len=sizeof(sa);

    // Check validity of address
    if(1024 > addr || addr > 49151){
        printf("%s\n", "Wrong address, must be 1024 to 49151!");
        exit(0);
    }

    // Create UDP socket
    
    if((sock =socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
        die("Socket");
    }

    // Prepare address structure
    memset((char *) &sa, 0, s_len);
    sa.sin_family = AF_INET;
    sa.sin_port = htons(addr);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind socket to port
    if((bind(sock , (struct sockaddr*)&sa, s_len)) == -1){
        die("Bind");
    }
    
    return ERR_OK;
}

int radio_send(int  dst, char* data, int len) {

    struct sockaddr_in sa;   // Structure to hold destination address

    // Check that port and len are valid
    if(1024 > dst || dst > 49151){
        printf("Wrong address, must be 1024 to 49151! Address was: %d\n", dst);
        exit(0);
    }
    if(FRAME_PAYLOAD_SIZE < len){
        printf("Data size is too big, must be below %d!\n", FRAME_PAYLOAD_SIZE);
        exit(1);
    }

    // Emulate transmission time

    // Prepare address structure
    memset((char *) &sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(dst);

    /*if (inet_aton(SERVER , &sa.sin_addr) == 0) 
    {
        printf("ERROR!!!!\n");
    }*/
    //sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    // Send the message
    if(sendto(sock, data, len , 0 , (struct sockaddr *) &sa, sizeof(sa)) == -1){
        die("sendto");
    }
    
    // Check if fully sent

    return ERR_OK;
}

int radio_recv(int* src, char* data, int to_ms) {

    //exit(-1);
    struct sockaddr_in sa;   // Structure to receive source address

    int len = -1;            // Size of received packet (or error code)


    // First poll/select with timeout (may be skipped at first)

    // Receive packet/data
    if(len = recvfrom(sock, data, FRAME_PAYLOAD_SIZE, 0, (struct sockaddr *) &sa, &s_len) == -1){
        die("recvfrom");
    }
    // Set source from address structure
    *src = ntohs(sa.sin_port);



    return len;
}

