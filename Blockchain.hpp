#pragma once
#include <cryptopp/rsa.h>
#include <cryptopp/base64.h>
#include <cryptopp/osrng.h>
#include <string>
#include <iomanip>
#include "json.hpp"
#include "sha256.hpp"
#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/VariadicBind.h>
using namespace std;

/*
	Description: Stores Data of a transaction
	Structure:
		-hash
		-prevHash
		-data
		-timestamp

	Functions:
*/

class Transaction {
private:
	string sender;
	string receiver;
	string data;
public:
	Transaction(nlohmann::json sender, nlohmann::json reciever, string data) {
		
		this->sender = sender;
		this->receiver = reciever;
		this->data = data;
	}

	nlohmann::json to_json() {
		nlohmann::json j;
		j["sender"] = sender;
		j["receiver"] = receiver;
		j["data"] = data;
		return j;
	}
	string getSender() {
		return sender;
	}

	string getReciever() {
		return receiver;
	}

	string getData() {
		return data;
	}
};

class Block {
private:
	int index;
	std::string prevHash;
	Transaction data;
	time_t timestamp;
	std::string currHash;
public:
	//int nonce;
	Block(int idx, std::string pHash, Transaction cData, time_t ts = std::time(nullptr)) : index(idx), prevHash(pHash), data(cData), timestamp(ts) {

		currHash = getHash();
	};

	//string operator
	nlohmann::json to_json() {
		nlohmann::json j;
		j["index"] = index;
		j["transaction"] = data.to_json();
		j["timestamp"] = timestamp;
		j["prevHash"] = prevHash;
		j["currHash"] = currHash;
		return j;
	}

	std::string getHash()
	{
		std::stringstream ss;
		ss << prevHash << data.to_json() << timestamp;
		return sha256(ss.str());
	}

	std::string getPrevHash()
	{
		return prevHash;
	}
	std::string getCurrHash()
	{
		return currHash;
	}

};

class Blockchain
{
private:
	int index;
	std::vector<Block> chain;
public:

	Blockchain() {
		index = 0;
		chain.push_back(Block(index, "0", Transaction(std::string("0").c_str(), std::string("0").c_str(), "Genesis block")));
	}

	//function to update the chain with previously saved json data
	void updateChain(nlohmann::json j) {
		chain.clear();
		for (auto& element : j["chain"]) {
			chain.push_back(Block(element["index"], element["prevHash"], Transaction(element["transaction"]["sender"], element["transaction"]["receiver"], element["transaction"]["data"]), element["timestamp"]));
		}
	}
	std::vector<Block> getChain() {
		return chain;
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
			std::string hash = sha256(std::to_string(solution) + getLastBlock().getCurrHash());
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

	void addBlock(Transaction data, std::string pubKey, std::string signature) {

		auto verifySignature = [](std::string pubKey, std::string data, std::string signature) -> bool {
			try
			{
				CryptoPP::RSASSA_PKCS1v15_SHA_Verifier verifier;
				verifier.AccessKey().Load(CryptoPP::StringSource(pubKey, true, new CryptoPP::Base64Decoder).Ref());
				CryptoPP::StringSource(data + signature, true,
					new CryptoPP::SignatureVerificationFilter(
						verifier, NULL,
						CryptoPP::SignatureVerificationFilter::THROW_EXCEPTION
					)
				);
				return true;

			}
			catch (CryptoPP::SignatureVerificationFilter::SignatureVerificationFailed& err)
			{
				return false;
			}
			return false;
		};
		/*std::cout << signature << std::endl;
		std::cout << data << std::endl;*/
		
		//data recieved is already in hash
		const std::string dataHash = data.getData();
		bool isValid = verifySignature(pubKey, dataHash, signature);
		if (isValid) {
			index++;
			Block newBlock(index, getLastBlock().getHash(), data);
			this->mine(2);
			chain.push_back(newBlock);
		}
		else {
			std::cout << "Invalid Signature" << std::endl;
		}
	}

	nlohmann::json to_json()
	{
		nlohmann::json j;
		for (int i = 0; i < chain.size(); i++)
		{
			j["chain"].push_back(chain[i].to_json());
		}
		return j;
	}

	void printChain() {

		nlohmann::json j;
		for (int i = 0; i < chain.size(); i++)
		{
			j["chain"].push_back(chain[i].to_json());
		}
		std::cout << j.dump(4) << std::endl;
	}
};

class Wallet {
private:
	std::string uid;
	std::string pubKey;
	std::string privKey;
	Blockchain* cc;
	SQLite::Database* db;
public:
	//Usually the constructor will be called with a private key 
	//but for now, lets generate
	Wallet(Blockchain& chain, std::string uid, SQLite::Database& db) : uid(uid)
	{
		this->db = &db;

		SQLite::Statement selectUser(*this->db, "SELECT * FROM user WHERE uid = ?");
		selectUser.bind(1, uid);

		if (selectUser.executeStep()) {
			
			std::cout << "Getting data from db" << std::endl;

			this->uid = selectUser.getColumn(0).getString();
			this->pubKey = selectUser.getColumn(1).getString();
			this->privKey = selectUser.getColumn(2).getString();
		}
		else {
			std::cout << "Adding a user" << std::endl;

			auto generateKeyPair = [](int mod) -> std::pair<std::string, std::string> {
				CryptoPP::AutoSeededRandomPool rng;
				CryptoPP::RSA::PrivateKey pvtKey;
				pvtKey.GenerateRandomWithKeySize(rng, mod);
				std::string pvt;
				CryptoPP::Base64Encoder privkeysink(new CryptoPP::StringSink(pvt));
				pvtKey.DEREncode(privkeysink);
				privkeysink.MessageEnd();
				CryptoPP::RSA::PublicKey pubKey(pvtKey);
				std::string pub;
				CryptoPP::Base64Encoder pubkeysink(new CryptoPP::StringSink(pub));
				pubKey.DEREncode(pubkeysink);
				pubkeysink.MessageEnd();
				return std::make_pair(pub, pvt);
			};

			auto [pubKey, privKey] = generateKeyPair(512);

			this->pubKey = pubKey;
			this->privKey = privKey;

			SQLite::Statement query(*this->db, "INSERT INTO user (uid, public_key, private_key) VALUES (?, ?, ?)");
			query.bind(1, uid);
			query.bind(2, pubKey);
			query.bind(3, privKey);

			query.exec();

			std::cout << "User added" << std::endl;
		}

		this->cc = &chain;

		/*std::cout << this->privKey << std::endl;
		std::cout << this->pubKey << std::endl;*/
	}

	void sendData(Transaction data, std::string recieverPubKey)
	{
		//std::cout << data << std::endl;
		//std::cout << dataHash << std::endl;

		auto generateSignature = [](std::string privKey, std::string data) -> std::string
		{
			std::string signature;
			CryptoPP::AutoSeededRandomPool rng;
			CryptoPP::RSASSA_PKCS1v15_SHA_Signer privkey;
			privkey.AccessKey().Load(CryptoPP::StringSource(privKey, true, new CryptoPP::Base64Decoder).Ref());
			CryptoPP::StringSource(data, true,
				new CryptoPP::SignerFilter(rng, privkey,
					new CryptoPP::StringSink(signature)
				)
			);
			return signature;
		};

		std::string signature = generateSignature(this->privKey, data.getData());
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