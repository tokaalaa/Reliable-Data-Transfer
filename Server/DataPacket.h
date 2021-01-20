/*
 * DataPacket.h
 *
 *  Created on: Dec 19, 2020
 *      Author: toka
 */

#ifndef DATAPACKET_H_
#define DATAPACKET_H_

#include <stdint.h>
#define PCK_DATA_SIZE 512
#define HEADERS_SIZE 8
#define PCK_SIZE PCK_DATA_SIZE + HEADERS_SIZE

class DataPacket {
public:
	unsigned char* create_packet(const char * data, uint16_t size, uint32_t seq_num);
	bool create_packet(const unsigned char* buff);
	bool verifyChecksum();

	uint16_t check_sum;
	uint16_t len;
	uint32_t seqno;
	char data[PCK_DATA_SIZE];
	unsigned char buffer[PCK_SIZE];
private:
	uint16_t computeCheckSum();
};

#endif /* DATAPACKET_H_ */
