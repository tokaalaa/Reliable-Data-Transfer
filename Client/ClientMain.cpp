//============================================================================
// Name        : C.cpp
// Author      : toka
// Version     :
// Copyright   : Your copyright notice
//============================================================================

#include <iostream>
#include "Client.h"
using namespace std;

int main()
{
    string file;
	cout << "Enter Input File path to read client Arguments: " << endl;
	//cin >> file;
	Client client;
	client.start("./client.in");
	return 0;
}
