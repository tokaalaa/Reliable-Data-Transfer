/*
 * Parser.cpp
 *
 *  Created on: Dec 18, 2020
 *      Author: toka
 */

#include "Parser.h"
#include <vector>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <algorithm>

//check if string is a number
bool is_number(const string& s)
{
    return !s.empty() && find_if(s.begin(),
        s.end(), [](unsigned char c) { return !isdigit(c); }) == s.end();
}
//check if string is float & in range [0-1]
bool isValidProb( string myString ) {
    std::istringstream iss(myString);
    float f;
    iss >> noskipws >> f;
    if (iss.eof() && !iss.fail()){
    	float prob = stof(myString);
    	return	!(prob < 0 || prob > 1);
    }
    return false;
}

// remove leading and trailing spaces from a string
string trim(const string &s)
{
    auto start = s.begin();
    while (start != s.end() && std::isspace(*start)) {
        start++;
    }

    auto end = s.end();
    do {
        end--;
    } while (std::distance(start, end) > 0 && std::isspace(*end));

    return std::string(start, end + 1);
}
// parse client input file
struct clientargs parseClientArgs(string file, bool *error){
	struct clientargs c;
	vector<string> lines = vector<string>();
		ifstream in(file);
		if (in.is_open()) {
			cout<<"Open file"<<endl;
			string line;
			while (getline(in, line)) {
				lines.push_back(line);
			}
		}else{
			cout<<"Could not open input file"<<endl;
		}
		if (lines.size() == 3) {
				if (is_number(lines[1].c_str())) {
					hostent * record;
					record = gethostbyname(lines[0].c_str());
					c.inAdd = (struct in_addr *)record->h_addr_list[0];
					c.servPort = stoi(lines[1]);
					c.filename = trim(lines[2]);
					*error = false;
				}
		}
 return c;
}

struct serverargs parseServerArgs(string file, bool *error){
	struct serverargs s;
		vector<string> lines = vector<string>();
			ifstream in(file);
			if (in.is_open()) {
				cout<<"Open file"<<endl;
				string line;
				while (getline(in, line)) {
					lines.push_back(line);
				}
			}else{
				cout<<"Could not open input file"<<endl;
			}
			if (lines.size() == 3) {
					if (is_number(lines[0].c_str()) && is_number(lines[1].c_str()) && isValidProb(lines[2].c_str())) {
						s.servPort = stoi(lines[0]);
						s.rand_seed = stoi(lines[1]);
						s.loss_prob = stof(lines[2]);
						*error = false;
					}
			}
	 return s;
}
