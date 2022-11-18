#define _CRT_SECURE_NO_WARNINGS
#define _HAS_STD_BYTE 0
#include <iostream>
#include "Blockchain.hpp"

int main()
{
	/*B chain;
	std::cout << chain.x << std::endl;
	A l = A(chain);
	std::cout << chain.x << std::endl;*/
	std::vector<Blockchain> chains;
	chains.push_back(Blockchain());
	chains.push_back(Blockchain());
	
	for (size_t i = 0; i < chains.size(); i++)
	{
		Wallet yash = Wallet(chains[i]);
		Wallet rdx = Wallet(chains[i]);
		yash.sendData("hello rudy", rdx.getPublicKey());
		rdx.sendData("hello yash", yash.getPublicKey());
		rdx.sendData("hello STACK OVERFLOW", rdx.getPublicKey());
		chains[i].printChain();
	}

	
	return 0;
	//print the blocks in the blockchain
}
//confirmed hash using https://emn178.github.io/online-tools/sha256.html 