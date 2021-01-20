/*
 * AckPacket.h
 *
 *  Created on: Dec 19, 2020
 *      Author: toka
 */

#ifndef ACKPACKET_H_
#define ACKPACKET_H_
#include <stdint.h>
#define HEADERS_SIZE 8


class AckPacket {
public:
	unsigned char* create_packet(uint32_t ack_num);
	bool create_packet(const unsigned char* buff);
	bool verifyChecksum();
	uint16_t check_sum;
	uint16_t len;
	uint32_t ackno;

private:
	unsigned char buffer[HEADERS_SIZE];
	uint16_t computeCheckSum();
};

#endif /* ACKPACKET_H_ */
