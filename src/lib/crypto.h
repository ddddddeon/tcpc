#include <crypto++/rsa.h>

class Crypto {
 public:
  CryptoPP::RSA::PrivateKey GenerateKey();
  void WriteKeyToFile(CryptoPP::RSAFunction &key, char *out);
  std::string PubKeyToString(CryptoPP::RSA::PublicKey pubkey);
};