#include "../src/lib/crypto.h"

#include <openssl/evp.h>
#include <string.h>

#include <iostream>

using std::cout;
using std::endl;

namespace TCPChat {

bool test_crypto() {
  EVP_PKEY* privkey = Crypto::GenerateKey();
  unsigned char* privkey_string = Crypto::KeyToString(privkey, true);
  printf("%s\n", privkey_string);
  printf("%d\n", strlen((const char*)privkey_string));

  unsigned char* pubkey_string = Crypto::KeyToString(privkey, false);
  printf("%s\n", pubkey_string);
  printf("%d\n", strlen((const char*)pubkey_string));

  memset(privkey_string, 0, strlen((char*)privkey_string));
  free(privkey_string);

  memset(pubkey_string, 0, strlen((char*)pubkey_string));
  free(pubkey_string);

  EVP_PKEY_free(privkey);

  return true;
}

}  // namespace TCPChat