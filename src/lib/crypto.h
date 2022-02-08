#ifndef CRYPTO_H
#define CRYPTO_H

#include <crypto++/base64.h>
#include <crypto++/files.h>
#include <crypto++/osrng.h>
#include <crypto++/rsa.h>

#include <string>

namespace TCPChat {

// TODO this probably doesn't need to be a class
class Crypto {
 public:
  CryptoPP::RSA::PrivateKey GenerateKey();
  void WriteKeyToFile(CryptoPP::RSAFunction &key, char *out);
  std::string PubKeyToString(CryptoPP::RSA::PublicKey pubkey);
  CryptoPP::RSA::PublicKey StringToPubKey(std::string pubkey_string);
};

}  // namespace TCPChat

#endif /* !CRYPTO_H */