/*
 * DataSender.cpp
 *
 *  Created on: Dec 22, 2020
 *      Author: toka
 */

#include "DataSender.h"
#include "AckPacket.h"

#include <sys/socket.h>
#include <sys/ioctl.h>

#include <string>
#include <fstream>
#include <streambuf>
#include <string.h>
#include <cmath>

#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <cstdlib>
#include <signal.h>
#include <sys/time.h>

#define TimeOut 0.1
#define CORRUPT_PROB 0.05

bool DataSender::fileExist(string filePath){
	ifstream in(filePath);
		if (!in) {
			return false;
		}
	return true;
}
void DataSender::setFileSize(string filePath){
	ifstream in(filePath, ifstream::ate | ifstream::binary);
		if (!in) {
			sentData = 0;
		}
		sentData = in.tellg();
		cout << "Length of data to be sent in bytes: " << sentData <<endl;
	in.close();
}
void DataSender::setWindowSize(int ws){
	windowSize = ws;
}
bool DataSender::hasPackets(int sock) {
	int count;
	ioctl(sock, FIONREAD, &count);
	return count > 0;
}
void DataSender::send_file(string filePath,float loss_prob,
		int serverSocket,const struct sockaddr * clientAddr){
	cout<<"============================ start ==================================="<<endl;
	this->loss_prob = loss_prob;
	setFileSize(filePath);
	setWindowSize(1);
	cwnd_for_analysis.push_back(windowSize);
	ifstream *in = new ifstream(filePath);
	bool finished = false;
	while(sentData > 0 || !finished){
		if(seqNo < base + windowSize){
			char buff[PCK_SIZE];
			memset(&buff, 0, PCK_SIZE);
			unsigned char* buffer;
			DataPacket pck;
			if (sentData > sizeof(char[PCK_DATA_SIZE])) {
				in->read(buff,sizeof(char[PCK_DATA_SIZE]));
				sentData -= sizeof(char[PCK_DATA_SIZE]);
				buffer = pck.create_packet(buff, (uint16_t) sizeof(char[PCK_DATA_SIZE]), seqNo);

			} else if (sentData != 0){
				in->read(buff,sentData);
				buffer = pck.create_packet(buff, (uint16_t) sentData, seqNo);
				sentData = 0;
			} else {
				finished = true;
				buffer = pck.create_packet(buff, (uint16_t) sentData, seqNo);
			}

			send_packet(seqNo, buffer,serverSocket, clientAddr);

			unacked_packets.push_back(pck);
			clock_t start = clock() / CLOCKS_PER_SEC;
			sent_time.push_back({seqNo, (double)start});
			seqNo++;
			/**bool error = false;
			while (!error && hasPackets(serverSocket)) {
					error = receive_acks(serverSocket,clientAddr);
			}
			if(error){
				cout << "Error occurred ... close program" << endl;
					return;
			}*/

		}else{
			//if seqNo >= base + windowSize
			bool error = false;
			bool b = false;
			while (!error && hasPackets(serverSocket)) {
				error = receive_acks(serverSocket,clientAddr);
				b = true;
			}
			if(error){
				cout << "Error occurred ... close program" << endl;
				return;
			}
			if(b){
			cwnd_for_analysis.push_back(windowSize);
			}

		}

		check_timeout(serverSocket, clientAddr);

	}
	while (unacked_packets.size() != 0) {
		bool error = false;
		while (!error && hasPackets(serverSocket)) {
			error = receive_acks(serverSocket,clientAddr);
		}
		if(error){
			cout << "Error occurred ... close program" << endl;
			return;
		}

		check_timeout(serverSocket, clientAddr);
	}

	//printing analysis data to file
	    ofstream cwnd_file;
	    cwnd_file.open("cwnd1.txt");
	    for (int i=0; i<cwnd_for_analysis.size(); i++)
	    {
	        cwnd_file << cwnd_for_analysis[i] << '\n';
	    }
	    cwnd_file.close();


}
bool DataSender::receive_acks(int sock, const struct sockaddr *clientAddr){
	cout<<"**cwnd was: "<<windowSize<<", ssthresh was: "<< ssthresh <<endl;
	bool error = true;
	cout << "socket has packets: receiving acks" << endl;
	struct timeval tv;
	tv.tv_sec = TimeOut;
	tv.tv_usec = 0;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

	socklen_t addrlen = sizeof(struct sockaddr_in);
	unsigned char buff[HEADERS_SIZE];
	memset(&buff, 0, HEADERS_SIZE);
	AckPacket ack_pck;
	memset(&ack_pck, 0, sizeof(AckPacket));
	if (recvfrom(sock, buff, HEADERS_SIZE, 0, (struct sockaddr *)clientAddr, &addrlen) > 0) {
		error = false;
		ack_pck.create_packet(buff);
		cout << "receiving ack" << ack_pck.ackno << endl;
	} else {
		if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
				error = true;
				//not time out
			} else {
				error = true;
				// time out
			}
	}

	if(base-1 == ack_pck.ackno){//duplicate ack is received
		dupAckCount++;
		if(dupAckCount == MAX_DUP_ACKS){//fast recovery
			cout << "max duplicate acks is received .. retransmit first unacked packet" << unacked_packets[0].seqno << endl;
			ssthresh = windowSize / 2;
			if(ssthresh == 0){
				ssthresh = 1;
			}
			windowSize = ssthresh + MAX_DUP_ACKS;
			//Resends first unacked packet
			send_packet(unacked_packets[0].seqno,unacked_packets[0].buffer,sock, clientAddr);

		} else if (dupAckCount > MAX_DUP_ACKS){
			windowSize+=1;
		}
	}else{// new ack is received

	   if(dupAckCount > MAX_DUP_ACKS){//fast recovery
		   windowSize = ssthresh;
	   }
	   dupAckCount = 0;
	   if(windowSize >= ssthresh){// congestion avoidance

		   while (base <= ack_pck.ackno) {
				unacked_packets.erase(unacked_packets.begin());
				sent_time.erase(sent_time.begin());
				base++;
				miniWindow++;
				if(miniWindow >= windowSize){
					windowSize+=1;
					miniWindow = 0;
				}
			}

		}else {//slow start

			while (base <= ack_pck.ackno) {
				unacked_packets.erase(unacked_packets.begin());
				sent_time.erase(sent_time.begin());
				base++;
				windowSize++;
			}

		}

	}
	cout<<"**cwnd becomes: "<<windowSize<<", ssthresh becomes: "<< ssthresh  <<endl;

	return error;
}

void DataSender::send_packet(uint32_t seqNo, unsigned char *buffer,int serverSocket, const struct sockaddr *clientAddr){
	float rd = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		if (rd < loss_prob) {
			cout << "Packet " << seqNo << " dropped" << endl;
		} else {
			unsigned char *buff = corrupt_packet(buffer, seqNo);
			if (sendto(serverSocket, buff, PCK_SIZE, 0, clientAddr, sizeof(struct sockaddr_in))> 0) {
				cout << "Packet " << seqNo << " sent" << endl;
			}else{
				cout << "Packet " << seqNo << " dropped due to error in sending" << endl;
			}
		}
}
unsigned char* DataSender::corrupt_packet(unsigned char *buffer,uint32_t seqNo){
	float rd = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	if (rd < CORRUPT_PROB) {
		cout << "Packet " << seqNo << " is corrupted" << endl;
		unsigned char buff[PCK_SIZE];
		corrupted.push_back(buff);
		memcpy(buff, buffer, PCK_SIZE);
		buff[int(rd * PCK_DATA_SIZE)+HEADERS_SIZE]= '@';
		return corrupted.back();
	} else {

		return buffer;
	}

}
void DataSender::check_timeout(int serverSocket, const struct sockaddr *clientAddr){
	for(unsigned int i = 0; i < sent_time.size(); i++){
		double current = clock() / CLOCKS_PER_SEC;
	    if((current - sent_time[i].second) > TimeOut){
	    	cout << "Time out: Try to resend: ";
	    	send_packet(unacked_packets[i].seqno, unacked_packets[i].buffer,serverSocket, clientAddr);
	    	ssthresh = windowSize/2;
	    	windowSize = 1;
	    	if(ssthresh == 0){
	    		ssthresh = 1;

	    	}
	    	sent_time[i].second = current;
	    }
	}
}


