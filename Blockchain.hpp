#pragma once
#include <string>
#include <iomanip>
#include <vector>
#include <sstream>
#include "json.hpp"
#include "sha256.hpp"


/*
	Description: Stores Data of a transaction
	Structure:
		-hash
		-prevHash
		-data
		-timestamp

	Functions: 
	
*/
class Block {
private:
	std::string prevHash;
	std::string data;
	time_t timestamp;
public:
	Block(std::string pHash, std::string cData, time_t ts = std::time(nullptr)) : prevHash(pHash), data(cData), timestamp(ts) {};
	
	//string operator
	std::string to_json() {
		nlohmann::json j;
		j["prevHash"] = prevHash;
		j["data"] = data;
		j["timestamp"] = timestamp;
		return j.dump(2);
	}
	
	std::string getHash()
	{
		return sha256(this->to_json());
	}
};

class Blockchain
{
private:
	std::vector<Block> chain;

public:
	Blockchain() {
		chain.push_back(Block("0", "Genesis Block"));
	}
	
	//get Last Block
	Block getLastBlock() {
		return chain.back();
	}
	
};