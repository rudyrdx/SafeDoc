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

class Wallet {
private:
	std::string pubKey;
	std::string privKey;

public:
	//Usually the constructor will be called with a private key 
	//but for now, lets generate
	Wallet()
	{
		auto generateKeyPair = []() -> std::pair<std::string, std::string> {
			AutoSeededRandomPool rng;
			RSA::PrivateKey pvtKey;
			pvtKey.GenerateRandomWithKeySize(rng, 128);
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

		auto [pubKey, privKey] = generateKeyPair();

		this->pubKey = pubKey;
		this->privKey = privKey;
	}
	//void Sign()
	//{
	//	string strContents = "Aakash is nigger";
	//	//FileSource("tobesigned.dat", true, new StringSink(strContents));

	//	AutoSeededRandomPool rng;

	//	//Read private key
	//	CryptoPP::ByteQueue bytes;
	//	FileSource file("privkey.txt", true, new Base64Decoder);
	//	file.TransferTo(bytes);
	//	bytes.MessageEnd();
	//	RSA::PrivateKey privateKey;
	//	privateKey.Load(bytes);

	//	//Sign message
	//	RSASSA_PKCS1v15_SHA_Signer privkey(privateKey);
	//	SecByteBlock sbbSignature(privkey.SignatureLength());
	//	privkey.SignMessage(
	//		rng,
	//		(byte const*)strContents.data(),
	//		strContents.size(),
	//		sbbSignature);

	//	//Save result
	//	FileSink sink("signed.txt");
	//	sink.Put((byte const*)strContents.data(), strContents.size());
	//	FileSink sinksig("sig.txt");
	//	sinksig.Put(sbbSignature, sbbSignature.size());
	//}
	void sendData(std::string data, std::string reciever)
	{
		const std::string dataHash = sha256(data);

		AutoSeededRandomPool rng;
		CryptoPP::ByteQueue bytes;
		StringSource file(this->privKey, true);
		file.TransferTo(bytes);
		bytes.MessageEnd();

		RSA::PrivateKey privateKey;
		privateKey.Load(bytes);

		RSASSA_PKCS1v15_SHA_Signer privkey(privateKey);
		SecByteBlock sbbSignature(privkey.SignatureLength());
		privkey.SignMessage(rng, (byte const*)dataHash.data(), dataHash.size(), sbbSignature);
		
		std::cout << sbbSignature;
	}
};