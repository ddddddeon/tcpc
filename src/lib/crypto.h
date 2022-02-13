#ifndef CRYPTO_H
#define CRYPTO_H

#include <openssl/evp.h>

#include <string>

namespace TCPChat {

namespace Crypto {

EVP_PKEY *GenerateKey();
void WriteKeyToFile(int key, char *out);
int LoadKeyFromFile(std::string path);
unsigned char *KeyToString(EVP_PKEY *privkey, bool priv);
int StringToPubKey(std::string pubkey_string);
std::string GenerateNonce();
std::string Sign(std::string message, int privkey);
bool Verify(std::string signature, std::string message, int pubkey);
std::string StripNewLines(std::string key);
std::string ExpandNewLines(std::string key);

};  // namespace Crypto

}  // namespace TCPChat

#endif /* !CRYPTO_H */