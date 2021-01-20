/*
 * DataReceiver.h
 *
 *  Created on: Dec 23, 2020
 *      Author: toka
 */

#ifndef DATARECEIVER_H_
#define DATARECEIVER_H_

#include "DataPacket.h"

#include <iostream>
#include <string>
#include <string>
#include <fstream>
#include <streambuf>

using namespace std;

class DataReceiver {
public:
	DataReceiver(string filename);
	void receive_message(int clientSocket);

private:
	ofstream *ofs;
	string filename;
	void write_data(DataPacket pack);
};

#endif /* DATARECEIVER_H_ */
