#define _CRT_SECURE_NO_WARNINGS
#define _HAS_STD_BYTE 0
#include <iostream>
#include "Blockchain.hpp"
using namespace std;
void GenKeyPair()
{
	// InvertibleRSAFunction is used directly only because the private key
	// won't actually be used to perform any cryptographic operation;
	// otherwise, an appropriate typedef'ed type from rsa.h would have been used.
	AutoSeededRandomPool rng;
	InvertibleRSAFunction privkey;
	privkey.Initialize(rng, 1024);

	// With the current version of Crypto++, MessageEnd() needs to be called
	// explicitly because Base64Encoder doesn't flush its buffer on destruction.
	Base64Encoder privkeysink(new FileSink("privkey.txt"));
	privkey.DEREncode(privkeysink);
	privkeysink.MessageEnd();

	// Suppose we want to store the public key separately,
	// possibly because we will be sending the public key to a third party.
	RSAFunction pubkey(privkey);

	Base64Encoder pubkeysink(new FileSink("pubkey.txt"));
	pubkey.DEREncode(pubkeysink);
	pubkeysink.MessageEnd();

}
void Sign()
{
	string strContents = sha256("Aakash is nigger");
	//FileSource("tobesigned.dat", true, new StringSink(strContents));

	AutoSeededRandomPool rng;

	//Read private key
	CryptoPP::ByteQueue bytes;
	FileSource file("privkey.txt", true, new Base64Decoder);
	file.TransferTo(bytes);
	bytes.MessageEnd();
	RSA::PrivateKey privateKey;
	privateKey.Load(bytes);

	//Sign message
	RSASSA_PKCS1v15_SHA_Signer privkey(privateKey);
	SecByteBlock sbbSignature(privkey.SignatureLength());
	privkey.SignMessage(
		rng,
		(byte const*)strContents.data(),
		strContents.size(),
		sbbSignature);

	//Save result
	FileSink sink("signed.txt");
	sink.Put((byte const*)strContents.data(), strContents.size());
	FileSink sinksig("sig.txt");
	sinksig.Put(sbbSignature, sbbSignature.size());
}

void Verify()
{
	//Read public key
	CryptoPP::ByteQueue bytes;
	FileSource file("pubkey.txt", true, new Base64Decoder);
	file.TransferTo(bytes);
	bytes.MessageEnd();
	RSA::PublicKey pubKey;
	pubKey.Load(bytes);

	RSASSA_PKCS1v15_SHA_Verifier verifier(pubKey);

	//Read signed message
	string signedTxt;
	FileSource("signed.txt", true, new StringSink(signedTxt));
	string sig;
	FileSource("sig.txt", true, new StringSink(sig));

	string combined(signedTxt);
	combined.append(sig);

	//Verify signature
	try
	{
		StringSource(combined, true,
			new SignatureVerificationFilter(
				verifier, NULL,
				SignatureVerificationFilter::THROW_EXCEPTION
			)
		);
		cout << "Signature OK" << endl;
	}
	catch (SignatureVerificationFilter::SignatureVerificationFailed& err)
	{
		cout << err.what() << endl;
	}

}

int main()
{
	/*Blockchain bChain = Blockchain();
	std::cout << "Genesis Block: " << bChain.getLastBlock().getHash() << std::endl;*/
	auto generateKeyPair = []() -> std::pair<std::string, std::string> {
		AutoSeededRandomPool rng;
		RSA::PrivateKey pvtKey;
		pvtKey.GenerateRandomWithKeySize(rng, 512);
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

	cout << pubKey << endl;
	cout << privKey << endl;

	//string strContents = sha256("Aakash is nigger");

	//cout << "Data Hash: " << endl;
	//cout << strContents << endl;

	//AutoSeededRandomPool rng;

	////get private key
	//CryptoPP::ByteQueue bytes;
	//StringSource pString(privKey, true, new Base64Decoder);
	//pString.TransferTo(bytes);
	//bytes.MessageEnd();
	//RSA::PrivateKey privateKey;
	//privateKey.Load(bytes);

	////Sign message
	//RSASSA_PKCS1v15_SHA_Signer privkey(privateKey);
	//SecByteBlock sbbSignature(privkey.SignatureLength());
	//privkey.SignMessage(rng, (byte const*)strContents.data(), strContents.size(), sbbSignature);

	//std::cout << "String Contents" << endl;
	//std::cout << strContents.data() << endl;

	//std::cout << "Content Signature" << endl;
	//std::cout << sbbSignature << endl;

	//////Read public key
	//CryptoPP::ByteQueue pBytes;
	//StringSource pbString(pubKey, true, new Base64Decoder);
	//pbString.TransferTo(pBytes);
	//pBytes.MessageEnd();
	//RSA::PublicKey pk;
	//pk.Load(pBytes);

	//RSASSA_PKCS1v15_SHA_Verifier verifier(pk);

	//stringstream ss;
	//ss << strContents.data() << sbbSignature;

	////Verify signature
	//try
	//{
	//	StringSource(ss.str(), true, new SignatureVerificationFilter(verifier, NULL, SignatureVerificationFilter::THROW_EXCEPTION));
	//	cout << "Signature OK" << endl;
	//}
	//catch (SignatureVerificationFilter::SignatureVerificationFailed& err)
	//{
	//	cout << err.what() << endl;
	//}
	GenKeyPair();
	Sign();
	Verify();
	return 0;
	//print the blocks in the blockchain
}
//confirmed hash using https://emn178.github.io/online-tools/sha256.html 