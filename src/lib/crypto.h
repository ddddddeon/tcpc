#include <crypto++/rsa.h>

#include <string>

namespace TCPChat {

class Crypto {
 public:
  CryptoPP::RSA::PrivateKey GenerateKey();
  void WriteKeyToFile(CryptoPP::RSAFunction &key, char *out);
  std::string PubKeyToString(CryptoPP::RSA::PublicKey pubkey);
  CryptoPP::RSA::PublicKey StringToPubKey(std::string pubkey_string);
};

}  // namespace TCPChat
