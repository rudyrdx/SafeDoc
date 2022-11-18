#pragma once
#include <string>
#include <iomanip>
#include <vector>
#include <sstream>
#include "json.hpp"
#include "sha256.hpp"

#include <cryptopp/rsa.h>
#include <cryptopp/osrng.h>
#include <cryptopp/base64.h>
#include <cryptopp/files.h>
using namespace CryptoPP;
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
	std::string currHash;
public:
	//int nonce;
	Block(std::string pHash, std::string cData, time_t ts = std::time(nullptr)) : prevHash(pHash), data(cData), timestamp(ts) {
		
		currHash = getHash();
	};

	//string operator
	std::string to_json() {
		nlohmann::json j;
		j["prevHash"] = prevHash;
		j["data"] = data;
		j["timestamp"] = timestamp;
		j["currHash"] = currHash;
		return j.dump(2);
	}

	std::string getHash()
	{
		std::stringstream ss;
		ss << prevHash << data << timestamp;
		return sha256(ss.str());
	}

	std::string getPrevHash()
	{
		return prevHash;
	}
	std::string getCurrHash()
	{
		return prevHash;
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

	void mine(int dificulty)
	{
		int solution = 1;
		std::cout << "mining..." << std::endl;
		while (true)
		{
			std::string hash = sha256(std::to_string(solution));
			if (hash.substr(0, dificulty) == std::string(dificulty, '0'))
			{
				std::cout << "mined: " << hash << std::endl;
				break;
			}
			solution++;
		}
	}

	bool validateChain()
	{
		for (int i = 1; i < chain.size(); i++)
		{
			Block currentBlock = chain[i];
			Block prevBlock = chain[i - 1];

			if (currentBlock.getCurrHash() != currentBlock.getHash())
			{
				std::cout << "current hash is not equal to the hash of the block" << std::endl;
				return false;
			}

			if (currentBlock.getPrevHash() != prevBlock.getCurrHash())
			{
				std::cout << "previous hash is not equal to the hash of the previous block" << std::endl;
				return false;
			}
		}
		return true;
	}
	
	void addBlock(std::string data, std::string pubKey, std::string signature) {
		auto verifySignature = [](std::string pubKey, std::string data, std::string signature) -> bool {
			try
			{
				RSASSA_PKCS1v15_SHA_Verifier verifier;
				verifier.AccessKey().Load(StringSource(pubKey, true, new Base64Decoder).Ref());
				StringSource(data + signature, true,
					new SignatureVerificationFilter(
						verifier, NULL,
						SignatureVerificationFilter::THROW_EXCEPTION
					)
				);
				return true;

			}
			catch (SignatureVerificationFilter::SignatureVerificationFailed& err)
			{
				return false;
			}
			return false;
		};
		/*std::cout << signature << std::endl;
		std::cout << data << std::endl;*/
		
		const std::string dataHash = sha256(data);
		bool isValid = verifySignature(pubKey, dataHash, signature);
		if (isValid) {
			Block newBlock(getLastBlock().getHash(), data);
			this->mine(2);
			chain.push_back(newBlock);
		}
		else {
			std::cout << "Invalid Signature" << std::endl;
		}
	}

	void printChain() {
		for (auto block : chain) {
			std::cout << block.to_json() << std::endl;
		}
	}
};

class Wallet {
private:
	std::string pubKey;
	std::string privKey;
	Blockchain* cc;
public:
	//Usually the constructor will be called with a private key 
	//but for now, lets generate
	Wallet(Blockchain& chain)
	{
		auto generateKeyPair = [](int mod) -> std::pair<std::string, std::string> {
			AutoSeededRandomPool rng;
			RSA::PrivateKey pvtKey;
			pvtKey.GenerateRandomWithKeySize(rng, mod);
			std::string pvt;
			Base64Encoder privkeysink(new StringSink(pvt));
			pvtKey.DEREncode(privkeysink);
			privkeysink.MessageEnd();
			RSA::PublicKey pubKey(pvtKey);
			std::string pub;
			Base64Encoder pubkeysink(new StringSink(pub));
			pubKey.DEREncode(pubkeysink);
			pubkeysink.MessageEnd();
			return std::make_pair(pub, pvt);
		};

		auto [pubKey, privKey] = generateKeyPair(512);

		this->pubKey = pubKey;
		this->privKey = privKey;
		
		this->cc = &chain;

		/*std::cout << this->privKey << std::endl;
		std::cout << this->pubKey << std::endl;*/
	}

	void sendData(std::string data, std::string recieverPubKey)
	{
		//std::cout << data << std::endl;
		const std::string dataHash = sha256(data);
		//std::cout << dataHash << std::endl;

		auto generateSignature = [](std::string privKey, std::string data) -> std::string
		{
			std::string signature;
			AutoSeededRandomPool rng;
			RSASSA_PKCS1v15_SHA_Signer privkey;
			privkey.AccessKey().Load(StringSource(privKey, true, new Base64Decoder).Ref());
			StringSource(data, true,
				new SignerFilter(rng, privkey,
					new StringSink(signature)
				)
			);
			return signature;
		};

		std::string signature = generateSignature(this->privKey, dataHash);
		//std::cout << signature << std::endl;

		this->cc->addBlock(data, this->pubKey, signature);
	}

	std::string getPrivateKey()
	{
		return this->privKey;
	}

	std::string getPublicKey()
	{
		return this->pubKey;
	}
};