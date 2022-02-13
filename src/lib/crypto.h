#ifndef CRYPTO_H
#define CRYPTO_H

#include <openssl/evp.h>

#include <string>

#ifdef __cplusplus
namespace TCPChat {

namespace Crypto {
#endif

EVP_PKEY *GenerateKey();
bool KeyToFile(EVP_PKEY *key, char *out_file, bool is_private);
unsigned char *KeyToString(EVP_PKEY *privkey, bool is_private);
EVP_PKEY *FileToKey(char *in_file, bool is_private);
EVP_PKEY *StringToKey(unsigned char *key_string, bool is_private);
unsigned char *Sign(char *message, EVP_PKEY *key);
bool Verify(char *message, unsigned char *signature, EVP_PKEY *pubkey);
std::string GenerateNonce();
std::string StripNewLines(std::string key);
std::string ExpandNewLines(std::string key);

#ifdef __cplusplus
}  // namespace Crypto

}  // namespace TCPChat
#endif

#endif /* !CRYPTO_H */