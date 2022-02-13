#ifndef CRYPTO_H
#define CRYPTO_H

#include <openssl/evp.h>

#include <string>

namespace TCPChat {

namespace Crypto {

EVP_PKEY *GenerateKey();
void KeyToFile(EVP_PKEY *key, char *out_file, bool is_private);
unsigned char *KeyToString(EVP_PKEY *privkey, bool is_private);
EVP_PKEY *FileToKey(char *in_file, bool is_private);
EVP_PKEY *StringToKey(unsigned char *key_string, bool is_private);
std::string GenerateNonce();
unsigned char *Sign(char *message, EVP_PKEY *key);
bool Verify(char *message, unsigned char *signature, EVP_PKEY *pubkey);
std::string StripNewLines(std::string key);
std::string ExpandNewLines(std::string key);

};  // namespace Crypto

}  // namespace TCPChat

#endif /* !CRYPTO_H */