#include "../src/lib/crypto.h"

#include <openssl/evp.h>

#include <iostream>

using std::cout;
using std::endl;

namespace TCPChat {

bool test_crypto() {
  int key = Crypto::GenerateKey("./");
  cout << key << endl;
  return true;
}

}  // namespace TCPChat