/*
 * Client.cpp
 *
 *  Created on: Dec 18, 2020
 *      Author: toka
 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <sys/time.h>

#include "Client.h"
#include "Parser.h"
#include "DataPacket.h"
#include "DataReceiver.h"

#define TimeOut 10

void Client::start(string file){
	bool error = true;
	struct clientargs args = parseClientArgs(file,&error);
	if(error){
		cout<<"Error in parsing input file"<<endl;
		return;
	}

	cout << "Parse input file successfully" <<endl;
	int clientSocket;
	if((clientSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		cout << "Failed to create client socket!" << endl;
		return;
	}

	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0 , sizeof(sockaddr_in));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = args.inAdd->s_addr;
	serverAddr.sin_port = htons(args.servPort);

	DataPacket pck;
	unsigned char* buff =pck.create_packet(args.filename.c_str(), (uint16_t)args.filename.size(), 0);

	struct timeval tv;
	tv.tv_sec = TimeOut;
	tv.tv_usec = 0;
	setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
	int ntry = 0;
	error = true;
	while (error && ntry < 3) {
		ntry++;
		int trysend = 0;
		while (error && trysend < 3) {
			trysend++;
			if(sendto(clientSocket, buff, PCK_SIZE, 0, (const struct sockaddr *)&serverAddr, sizeof(struct sockaddr_in)) > 0 ){
				error = false;
			}else{
				cout << "Failed to send request. Retrying again for " << trysend << " time of 3 retries !" << endl;
			}
		}
		if (error) {
			cout << "Could not send request...Close program!" << endl;
			close(clientSocket);
			return;
		}
		char c;
		if (recv(clientSocket, &c, 1, MSG_PEEK) == -1) {
			error = true;
			if ((errno != EAGAIN) && (errno != EWOULDBLOCK)) {
				cout << "Response timeout, retransmitting. Retrying again for " << ntry << " time of 3 retries !" << endl;
			} else {
				cout << "Failed to receive response. Retrying again for " << ntry << " time of 3 retries !" << endl;
			}

		} else {
		   error = false;
		}
	}
	if (error) {
		cout << "Could not send data request to server ... Close program" << endl;
		close(clientSocket);
		return;
	}

	cout << "Send data request successfully ... Waiting for receiving from server ... " << endl;

	struct timeval startTime;
	gettimeofday(&startTime, NULL);

	DataReceiver receiver = DataReceiver(args.filename);
	receiver.receive_message(clientSocket); // rand_sink

	struct timeval endTime;
	gettimeofday(&endTime, NULL);
	long int timeTaken = ((endTime.tv_sec - startTime.tv_sec)*1000000L
				   +endTime.tv_usec) - startTime.tv_usec;
	cout <<"Transferring file process is completed!" << endl;
	printf("transmission time :\n\tsec : %ld sec\n\tmsec : %ld msec\n\tMsec : %ld Msec\n",
		 (timeTaken / 1000000L), ((timeTaken / 1000L) % 1000L), (timeTaken % 1000L)
								);

	close(clientSocket);
	cout << "Connection closed" << endl;

}

