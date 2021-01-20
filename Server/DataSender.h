/*
 * DataSender.h
 *
 *  Created on: Dec 22, 2020
 *      Author: toka
 */

#ifndef DATASENDER_H_
#define DATASENDER_H_
#include <iostream>
#include <vector>
#include <sys/time.h>
#include "DataPacket.h"

#define MAX_DUP_ACKS 3

using namespace std;
class DataSender {
public:
	bool fileExist(string filePath);
	void send_file(string filePath,float loss_prob, int serverSocket,
			const struct sockaddr * clientAddr);
private:
	float loss_prob;
	int windowSize;
	int miniWindow = 0;
	unsigned int sentData = 0;
	uint32_t seqNo = 0;
	uint32_t base = 0;
	int ssthresh = 32;
	int dupAckCount = 0;
	vector<pair<uint32_t, double>> sent_time;
	vector<DataPacket>unacked_packets;
	vector<unsigned char*>corrupted;
	vector<int>cwnd_for_analysis;
	void setFileSize(string filePath);
	void setWindowSize(int ws);
	bool hasPackets(int sock);
	bool receive_acks(int sock, const struct sockaddr *clientAddr);
	void send_packet(uint32_t seqNo, unsigned char *buffer,int serverSocket, const struct sockaddr *clientAddr);
	void check_timeout(int serverSocket, const struct sockaddr *clientAddr);
	unsigned char* corrupt_packet(unsigned char *buffer,uint32_t seqNo);
};

#endif /* DATASENDER_H_ */
