#ifndef CRYPTO_H
#define CRYPTO_H

#include <crypto++/base64.h>
#include <crypto++/files.h>
#include <crypto++/osrng.h>
#include <crypto++/rsa.h>

#include <string>

namespace TCPChat {

namespace Crypto {

CryptoPP::RSA::PrivateKey GenerateKey(std::string path);
void WriteKeyToFile(CryptoPP::RSAFunction &key, char *out);
CryptoPP::ByteQueue LoadKeyFromFile(std::string path);
CryptoPP::RSA::PublicKey StringToPubKey(std::string pubkey_string);
std::string PubKeyToString(CryptoPP::RSA::PublicKey pubkey);
std::string StripNewLines(std::string key);
std::string ExpandNewLines(std::string key);
bool Verify(std::string signature, CryptoPP::RSA::PublicKey pubkey);

};  // namespace Crypto

}  // namespace TCPChat

#endif /* !CRYPTO_H */