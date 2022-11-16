#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include "Blockchain.hpp"
int main()
{
	Blockchain bChain = Blockchain();
	std::cout << "Genesis Block: " << bChain.getLastBlock().getHash() << std::endl;
	return 0;
	//print the blocks in the blockchain
}
//confirmed hash using https://emn178.github.io/online-tools/sha256.html