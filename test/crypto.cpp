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

  EVP_PKEY* pubkey2 = Crypto::StringToKey(pubkey_string, false);
  unsigned char* pubkey_string2 = Crypto::KeyToString(pubkey2, false);

  printf("%s\n", pubkey_string2);

  Crypto::KeyToFile(privkey, "id_rsa", true);
  Crypto::KeyToFile(privkey, "id_rsa.pub", false);

  EVP_PKEY* opened_privkey = Crypto::FileToKey("./id_rsa", true);
  EVP_PKEY* opened_pubkey = Crypto::FileToKey("./id_rsa.pub", false);

  unsigned char* opened_privkey_string =
      Crypto::KeyToString(opened_privkey, true);
  unsigned char* opened_pubkey_string =
      Crypto::KeyToString(opened_pubkey, false);

  printf("%s\n", opened_privkey_string);
  printf("%s\n", opened_pubkey_string);

  char* message = "chris is cool";

  unsigned char* sig = Crypto::Sign(message, privkey);
  printf("%s\n", sig);

  free(sig);
  free(privkey_string);
  free(pubkey_string);
  free(pubkey_string2);
  EVP_PKEY_free(privkey);

  return true;
}

}  // namespace TCPChat