/*
 * ServerMain.cpp
 *
 *  Created on: Dec 19, 2020
 *      Author: toka
 */
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include <iostream>
#include "Server.h"
using namespace std;

int main()
{
    string file;
	cout << "Enter Input File path to read server Arguments: " << endl;
	//cin >> file;
	Server server;
	server.start("./server.in");
	return 0;
}


