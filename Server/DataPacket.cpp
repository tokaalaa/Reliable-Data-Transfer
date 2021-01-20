/*
 * DataPacket.cpp
 *
 *  Created on: Dec 19, 2020
 *      Author: toka
 */

#include "DataPacket.h"
#include <cstring>
#include <iostream>
using namespace std;

unsigned char* DataPacket::create_packet(const char * data, uint16_t size, uint32_t seq_num){
		memset(&buffer, 0, sizeof(buffer));
		memset(&this->data, 0, sizeof(this->data));
		len = HEADERS_SIZE + size;
		seqno = seq_num;
		memcpy(this->data, data, size);
		check_sum = computeCheckSum();

		// add headers fields
		buffer[0] = check_sum >> 8;
		buffer[1] = check_sum;

		buffer[2] = len >> 8;
		buffer[3] = len;

		buffer[4] = seqno >> 24;
		buffer[5] = seqno >> 16;
		buffer[6] = seqno >> 8;
		buffer[7] = seqno;

		// add data
		memcpy(buffer + 8, data, PCK_SIZE);

		return buffer;

}
bool DataPacket::create_packet(const unsigned char* buff){
	memcpy(buffer, buff, PCK_SIZE);
	//extract headers fields
	check_sum = ((uint16_t)buffer[0]) << 8;
	check_sum |= ((uint16_t)buffer[1]);

	len = ((uint16_t)buffer[2]) << 8;
	len |= ((uint16_t)buffer[3]);

	seqno = ((uint16_t)buffer[4]) << 24;
	seqno |= ((uint16_t)buffer[5]) << 16;
	seqno |= ((uint16_t)buffer[6]) << 8;
	seqno |= ((uint16_t)buffer[7]);

	// extract data
	memset(&data, 0, sizeof(data));
	memcpy(data, buffer + 8, PCK_DATA_SIZE);
	return true;

}
uint16_t DataPacket::computeCheckSum(){

		uint32_t sum = (uint32_t)len;
		sum += (seqno >> 16);
		if (sum & 0x00010000) {
			sum &= 0x0000FFFF;
			sum += 1;
		}
		sum += (seqno & 0x0000FFFF);
		if (sum & 0x00010000) {
			sum &= 0x0000FFFF;
			sum += 1;
		}
		uint16_t *dum_ptr = (uint16_t *)data;
		for (unsigned int i = 0; i < PCK_DATA_SIZE / 16; i++) {
			sum += (uint32_t)(dum_ptr[i]);
			if (sum & 0x00010000) {
				sum &= 0x0000FFFF;
				sum += 1;
			}
		}
		return (uint16_t)sum;

}

bool DataPacket::verifyChecksum() {
	return check_sum == computeCheckSum();
}

