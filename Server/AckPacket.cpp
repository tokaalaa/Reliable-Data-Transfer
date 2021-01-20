/*
 * AckPacket.cpp
 *
 *  Created on: Dec 19, 2020
 *      Author: toka
 */

#include "AckPacket.h"
#include <cstring>

unsigned char* AckPacket::create_packet(uint32_t ack_num){
		memset(&buffer, 0, sizeof(buffer));
		len = HEADERS_SIZE;
		ackno = ack_num;
		check_sum = computeCheckSum();

		// add headers fields
		buffer[0] = check_sum >> 8;
		buffer[1] = check_sum;

		buffer[2] = len >> 8;
		buffer[3] = len;

		buffer[4] = ackno >> 24;
		buffer[5] = ackno >> 16;
		buffer[6] = ackno >> 8;
		buffer[7] = ackno;

		return buffer;

}
bool AckPacket::create_packet(const unsigned char* buff){
	memcpy(buffer, buff, HEADERS_SIZE);
	//extract headers fields
	check_sum = ((uint16_t)buffer[0]) << 8;
	check_sum |= ((uint16_t)buffer[1]);

	len = ((uint16_t)buffer[2]) << 8;
	len |= ((uint16_t)buffer[3]);

	ackno = ((uint16_t)buffer[4]) << 24;
	ackno |= ((uint16_t)buffer[5]) << 16;
	ackno |= ((uint16_t)buffer[6]) << 8;
	ackno |= ((uint16_t)buffer[7]);

	return true;

}

uint16_t AckPacket::computeCheckSum(){

		uint32_t sum = (uint32_t)len;
		sum += (ackno >> 16);
		if (sum & 0x00010000) {
			sum &= 0x0000FFFF;
			sum += 1;
		}
		sum += (ackno & 0x0000FFFF);
		if (sum & 0x00010000) {
			sum &= 0x0000FFFF;
			sum += 1;
		}

		return (uint16_t)sum;

}

bool AckPacket::verifyChecksum() {
	return check_sum == computeCheckSum();
}
