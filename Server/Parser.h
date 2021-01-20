/*
 * Parser.h
 *
 *  Created on: Dec 18, 2020
 *      Author: toka
 */

#ifndef PARSER_H_
#define PARSER_H_
#include <iostream>
#include <string>
using namespace std;

 struct clientargs {
  struct in_addr * inAdd;
  int servPort;
  string filename;
 };
 struct serverargs {
   int servPort;
   int rand_seed;
   float loss_prob;
  };

 struct clientargs parseClientArgs(string file, bool *error);
 struct serverargs parseServerArgs(string file, bool *error);

#endif /* PARSER_H_ */
