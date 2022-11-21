#define _CRT_SECURE_NO_WARNINGS
#define _HAS_STD_BYTE 0
#include <iostream>
#include <filesystem>
#include "Blockchain.hpp"
#include "httplib.h"

//rn i am doing smthing i shouldnt do but due to the time restrictions, i have to centralize the data by recieveing it byt the process and 
//broadcasting to all nodes . i will fix this later
//what i have to do is make the threads listen to http req, as soon as they recieve a req, they will add the data to the blockchain and then broadcast the last block's hash
//after which they will be compared to the hash of the last block in the blockchain and if they are not equal, the blockchain will be updated on the basis of the consensus
//hash (chain)
int main()
{
	/*nlohmann::json j;

	if (std::ifstream("consif.json")) {
		j = nlohmann::json::parse(std::ifstream("config.json"));
	}
	else {
		nodes = 5;
		j["nodes"] = nodes;

		std::ofstream("config.json") << j.dump(2);
	}

	nodes = j["nodes"];*/

	//std::vector<Blockchain> chains;
	//chains.push_back(Blockchain());
	//chains.push_back(Blockchain());
	//for (size_t i = 0; i < chains.size(); i++)
	//{
	//	Wallet yash = Wallet(chains[i]);
	//	Wallet rdx = Wallet(chains[i]);
	//	yash.sendData("hello rudy", rdx.getPublicKey());
	//	rdx.sendData("hello yash", yash.getPublicKey());
	//	rdx.sendData("hello STACK OVERFLOW", rdx.getPublicKey());
	//	chains[i].printChain();
	//}


	//create a db 
	SQLite::Database db("blockchain.db", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

	// Test if the 'test' table exists
	const bool bExists = db.tableExists("user");
	std::cout << "SQLite table 'user' exists=" << bExists << std::endl;
	
	if (!bExists) {
		// Create the 'test' table
		db.exec("CREATE TABLE user (uid CHAR PRIMARY KEY, public_key TEXT, private_key TEXT)");
		
		cout << "created user table" << endl;
	}

	int nodes = 3;

	std::vector<Blockchain> chains(nodes);

	std::string nodesFolder = "nodes/";

	if (!std::filesystem::exists(nodesFolder)) {
		std::cout << "couldnt find nodes folder, so making one" << std::endl;
		std::filesystem::create_directory(nodesFolder);

		for (auto i = 0; i < nodes; i++) {

			//create json files
			std::ofstream(nodesFolder + std::to_string(i) + ".json") << chains[i].to_json().dump(2);
		}
		std::cout << "added node json files to the directory" << std::endl;
	}
	else {
		std::cout << "nodes directory exists, reading from it" << std::endl;

		//TODO: check if there are files in the directory

		//load data in the chains from the json files in the nodes directory
		for (auto i = 0; i < nodes; i++) {
			chains[i].updateChain(nlohmann::json::parse(std::ifstream(nodesFolder + std::to_string(i) + ".json")));
			//chains[i].getLastBlock().to_json();
		}

		//validate the chains by comparing all chains last blocks hash and determining a concesus chain
		//the below algo is not the best, but it works for now
		//what i would do is, check if all these chains are valid, remove the bad ones and then compare the hashes of the remaining ones 
		//and select the largest of them all.

		std::vector<int> invalidChains = std::vector<int>();
		int validChain = 0;
		for (auto i = 0; i < nodes; i++) {

			if (chains[0].getLastBlock().getCurrHash() != chains[i].getLastBlock().getCurrHash()) {
				//find a chain which is largest and valid and update all other chains to that chain
				
				std::cout << "chain " + std::to_string(i) + " is not like other chains" << std::endl;
				
				//make it or them like the other chains
				invalidChains.push_back(i);
			}
			else {
				if (chains[i].validateChain()) {
					std::cout << "chain is valid: " + std::to_string(i) << std::endl;
					validChain = i;
				}
				else
				{
					std::cout << "chain is invalid: " + std::to_string(i) << std::endl;
					invalidChains.push_back(i);
				}
			}
		}
		
		if (invalidChains.size() > 0) {
			for (int i = 0; i < invalidChains.size(); i++) {
				
				chains[invalidChains[i]] = chains[validChain];
				
				std::ofstream(nodesFolder + std::to_string(invalidChains[i]) + ".json") << chains[invalidChains[i]].to_json().dump(2);
				
			}
		}

	}

	std::cout << "started the web server" << std::endl;
	
	// HTTP
	httplib::Server svr;

	svr.Post("/transaction", [&](const httplib::Request& req, httplib::Response& res) {

		//rudy.sendData("lol", yash.getPublicKey());
		nlohmann::json json;

		auto body = req.body;
		json = nlohmann::json::parse(body);
		
		if (json["sender"] == nullptr || json["receiver"] == nullptr || json["document"] == nullptr)
		{
			res.status = 400;
			res.set_redirect("/", 400);
			return;
		}

		std::string js  = json["sender"].dump();
		std::string jrs = json["receiver"].dump();
		std::string jd  = json["document"];
		
		try {
			
			std::cout << "sender id: " + js << std::endl;
			std::cout << "receiver id: " + jrs << std::endl;
			std::cout << "doc hash: " + jd << std::endl;
		}
		catch (std::exception e) {
			cout << e.what() << endl;
		}
		
		for (int i = 0; i < chains.size(); i++) {
			
			try {

				Wallet s = Wallet(chains[i], js, db);
				Wallet r = Wallet(chains[i], jrs, db);
				s.sendData(Transaction(json["sender"], json["receiver"], jd), r.getPublicKey());
			}
			catch (std::exception e) {
				cout << e.what() << endl;
			}

			if (chains[i].validateChain()) {
				std::cout << "chain " + std::to_string(i) + " is valid" << std::endl;
				std::ofstream(nodesFolder + std::to_string(i) + ".json") << chains[i].to_json().dump(2);
			}
			
			//todo append the chains in files
			// 
			//have to free up memory on ram
			//what we do is, before reaching this step, clean the chains, then add the last block from json file to chain
			//validate the blocks and once validated append the last block to the chain and free up memory 
			//OOOF b1g algo xD but i wont do it rn heheheheh
		}
		nlohmann::json jr;
		jr["message"] = "transaction successful";
		jr["block"] = chains[0].getLastBlock().to_json();
		
		res.set_content(jr.dump(2), "application/json");

	});

	svr.Get("/", [&](const httplib::Request&, httplib::Response& res) {

		res.set_content(chains[0].to_json().dump(2), "application/json");
	});

	svr.listen("0.0.0.0", 3000);

	return 0;

	//print the blocks in the blockchain
}
//confirmed hash using https://emn178.github.io/online-tools/sha256.html 
// 
//{
//	"sender": {"publicKey": "asdasd", "privateKey" : "asdasdasd"},
//	"receiver" : "asd",
//	"document" : "0x99999"
//}


/*
MFowDQYJKoZIhvcNAQEBBQADSQAwRgJBALIcoSkYUVVNmlgNvusUo2SqdNmDe + 90PO8eubxN
iedP2K9HTtKTw5S / 79QO3PwypyXm7z7W5P2jf9LGjxbwQFMCARE =


MIIBUgIBADANBgkqhkiG9w0BAQEFAASCATwwggE4AgEAAkEAshyhKRhRVU2aWA2 + 6xSjZKp0
2YN773Q87x65vE2J50 / Yr0dO0pPDlL / v1A7c / DKnJebvPtbk / aN / 0saPFvBAUwIBEQJATpQo
+ 4q6eHUPYxUfhdRmM / DZMskRB8JXHjMz0xMmQGY / nC9l1Zoctq298AQDV / AtMyhRal / wMKDm
FQDx3d1oDQIhALrFuJA0ViGpZ6BrL7SWFJQtCAPl9ZohNYdLtsVpqtR9AiEA9CEjT0KcC4B5
KWr4MeMOvZX6 / 5r2xAfOESRA2KVPkQ8CIQCkzJPKiIg70bXJx / zqooqg3HB76QXiWYmVfwqu
MA8z9QIgcuJq + B9YfeIa5lB0zC6dhmSyWirOel4GnqemC5kWYmECIETJhXbQMbjhgOtRBYV5
RV + 6tfDrCI1Gf9o4sXeGdJkR
*/