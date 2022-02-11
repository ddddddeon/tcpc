#ifndef CRYPTO_H
#define CRYPTO_H

#include <string>

namespace TCPChat {

namespace Crypto {

int GenerateKey(std::string path);
void WriteKeyToFile(int key, char *out);
int LoadKeyFromFile(std::string path);
std::string PubKeyToString(int pubkey);
int StringToPubKey(std::string pubkey_string);
std::string GenerateNonce();
std::string Sign(std::string message, int privkey);
bool Verify(std::string signature, std::string message, int pubkey);
std::string StripNewLines(std::string key);
std::string ExpandNewLines(std::string key);

};  // namespace Crypto

}  // namespace TCPChat

#endif /* !CRYPTO_H */