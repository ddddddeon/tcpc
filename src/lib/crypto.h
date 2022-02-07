#include <crypto++/rsa.h>

class Crypto {
 public:
  void GenerateKey();
  void WriteKeyToFile(CryptoPP::RSAFunction &key, char *out);
};