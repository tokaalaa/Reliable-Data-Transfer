/*
 * DataReceiver.cpp
 *
 *  Created on: Dec 23, 2020
 *      Author: toka
 */

#include "DataReceiver.h"
#include "AckPacket.h"

#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>

DataReceiver::DataReceiver(string filename){
	this->filename = filename;
	ofs = new ofstream(filename);
}
void DataReceiver::receive_message(int clientSocket){
	bool finished = false;
	uint32_t base = 0;
	struct sockaddr_in src_addr;
	memset(&src_addr, 0 , sizeof(struct sockaddr_in));
	bool error;

	while (!finished) {
			struct timeval tv;
			tv.tv_sec = 0;
			tv.tv_usec = 0;
			socklen_t addrlen = sizeof(struct sockaddr_in);
			unsigned char buff[PCK_SIZE];
			memset(&buff, 0, PCK_SIZE);
			DataPacket pck;
			memset(&pck, 0, sizeof(DataPacket));
			if (recvfrom(clientSocket, buff, PCK_SIZE, 0, (struct sockaddr *)&src_addr, &addrlen) > 0) {
					error = false;
					pck.create_packet(buff);
					cout << "received packet: "<<pck.seqno << " of length=  "<< pck.len <<endl;
				} else {
					if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
							error = true;
							//not time out
						} else {
							error = true;
							// time out
						}
					}
			if (error) {// || timeout
				cout << "error occurred receiving packet" << endl;
			}
			else if (pck.seqno == base && pck.verifyChecksum()) {// accept packet only if it has the expected seqno
				if (pck.len != HEADERS_SIZE) {// still not the last packet
					base++;
					write_data(pck);
					//send ack packet
					AckPacket ack_packet;
					unsigned char* buff = ack_packet.create_packet(pck.seqno);
					sendto(clientSocket, buff, HEADERS_SIZE, 0, (struct sockaddr *)&src_addr, sizeof(struct sockaddr_in));
					cout << "Receiving packet " << ack_packet.ackno << endl;
				} else {
					base++;
					AckPacket ack_packet;
					unsigned char* buff = ack_packet.create_packet(pck.seqno);
					sendto(clientSocket, buff, HEADERS_SIZE, 0, (struct sockaddr *)&src_addr, sizeof(struct sockaddr_in));
					cout << "Receiving last packet " << ack_packet.ackno << endl;
					finished = true;
				}

	      }else if (pck.seqno != base){
	    	  // send duplicate ack --> ack last received packet
	    	  AckPacket ack_packet;
	    	  unsigned char* buff = ack_packet.create_packet(base - 1);
	    	  sendto(clientSocket, buff, HEADERS_SIZE, 0, (struct sockaddr *)&src_addr, sizeof(struct sockaddr_in));
	    	  cout << "Receiving out of order packet " << pck.seqno << " ,ack last packet: " << base-1 <<endl;
	      }else{//corrupted packet was received
	    	  cout << "Receiving inorder corrupted packet " << pck.seqno << endl;
	      }
	    }
	this->ofs->close();
	delete this->ofs;
	this->ofs = nullptr;

}

void DataReceiver::write_data(DataPacket pack){
	int size = pack.len - HEADERS_SIZE;
	char data[size];
	memset(&data, 0, sizeof(char [size]));
	memcpy(data, pack.data, size);
	ofs->write(data, size);
}

