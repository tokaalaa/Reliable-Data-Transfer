/*
 * Server.cpp
 *
 *  Created on: Dec 19, 2020
 *      Author: toka
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <cstdlib>
#include <signal.h>
#include <sys/time.h>

#include "Server.h"
#include "Parser.h"
#include "DataPacket.h"
#include "DataSender.h"

void Server::start(string file){
	bool error = true;
		struct serverargs args = parseServerArgs(file,&error);
		if(error){
			cout<<"Error in parsing input file"<<endl;
			return;
		}

		cout << "Parse input file successfully" <<endl;
		int serverSocket;
		if((serverSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			cout << "Failed to create server socket!" << endl;
			return;
		}
		struct sockaddr_in serverAddr;
		memset(&serverAddr, 0 , sizeof(sockaddr_in));
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		serverAddr.sin_port = htons(args.servPort);

		if (bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
				cout << "Could not bind listen port ... close program" << endl;
				close(serverSocket);
				return;
		}

		cout << "Start listening on port : " << args.servPort << endl;
		struct sockaddr_in clientAddr;
		srand(args.rand_seed);
		while(1){
			DataPacket pck;
			unsigned char buff[PCK_SIZE];
			memset(&buff, 0, PCK_SIZE);
			socklen_t addrlen = sizeof(struct sockaddr_in);
			if (recvfrom(serverSocket, buff,PCK_SIZE, 0, (struct sockaddr *)&clientAddr, &addrlen) < 0) {
				cout << "ERROR in receiving request from client" << endl;
				exit(1);

			}
			pid_t pid = fork();
				if (pid == 0) {
					pid = getpid();
					cout << pid << " : Child Process Handling Request : " << inet_ntoa(clientAddr.sin_addr) << ":"
											<< ntohs(clientAddr.sin_port) << endl;
					int childSocket;
					if((childSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
								cout << "Failed to create child server socket... close program!" << endl;
								return;
					}
					DataSender sender;
					pck.create_packet(buff);
					char filePath[PCK_DATA_SIZE + 1];
					memset(filePath, 0, PCK_DATA_SIZE + 1);
					memcpy(filePath, pck.data, pck.len - HEADERS_SIZE);
					if (!sender.fileExist(string(filePath))) {
							cout << pid << " : Invalid file " << filePath << " ...child closing" << endl;
							close(childSocket);
							exit(1);
					}

					cout << pid << " : Sending file : " << filePath <<" ..."<< endl;

					struct timeval startTime;
					gettimeofday(&startTime, NULL);
					sender.send_file(string(filePath),args.loss_prob, childSocket,(sockaddr *)&clientAddr);
					struct timeval endTime;
					gettimeofday(&endTime, NULL);
					long int timeTaken = ((endTime.tv_sec - startTime.tv_sec)*1000000L
						           +endTime.tv_usec) - startTime.tv_usec;

					close(childSocket);
					cout << pid << " : Finished Transferring file." << endl;
					printf("%u transmission time :\n\tsec : %ld sec\n\tmsec : %ld msec\n\tMsec : %ld Msec\n",
							pid, (timeTaken / 1000000L), ((timeTaken / 1000L) % 1000L), (timeTaken % 1000L)
							);
						exit(0);
					} else if (pid < 0) {
						cout << "Failed to create child to handle request from : " << inet_ntoa(clientAddr.sin_addr) << ":"
								<< ntohs(clientAddr.sin_port) << endl;
					}
					signal(SIGCHLD,SIG_IGN);
				}
				close(serverSocket);
				cout << "Connection closed" << endl;
}
