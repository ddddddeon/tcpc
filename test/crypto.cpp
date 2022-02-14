#include "../src/lib/crypto.h"

#include <assert.h>
#include <openssl/evp.h>
#include <string.h>

#include <iostream>

using std::cout;
using std::endl;

namespace TCPChat {

bool test_crypto() {
  EVP_PKEY* privkey = Crypto::GenerateKey();
  assert(privkey != NULL);

  unsigned char* privkey_string = Crypto::KeyToString(privkey, true);
  assert(privkey_string != NULL);

  unsigned char* pubkey_string = Crypto::KeyToString(privkey, false);
  assert(pubkey_string != NULL);

  EVP_PKEY* pubkey = Crypto::StringToKey(pubkey_string, false);
  assert(pubkey != NULL);

  unsigned char* pubkey_string2 = Crypto::KeyToString(pubkey, false);
  assert(pubkey_string2 != NULL);
  assert(strcmp((char*)pubkey_string, (char*)pubkey_string2) == 0);

  assert(Crypto::KeyToFile(privkey, "id_rsa", true));
  assert(Crypto::KeyToFile(privkey, "id_rsa.pub", false));

  EVP_PKEY* opened_privkey = Crypto::FileToKey("./id_rsa", true);
  EVP_PKEY* opened_pubkey = Crypto::FileToKey("./id_rsa.pub", false);

  assert(opened_privkey != NULL);
  assert(opened_pubkey != NULL);

  unsigned char* opened_privkey_string =
      Crypto::KeyToString(opened_privkey, true);
  unsigned char* opened_pubkey_string =
      Crypto::KeyToString(opened_pubkey, false);

  assert(opened_privkey_string != NULL);
  assert(opened_pubkey_string != NULL);
  assert(strcmp((char*)privkey_string, (char*)opened_privkey_string) == 0);
  assert(strcmp((char*)pubkey_string, (char*)opened_pubkey_string) == 0);

  printf("%s\n", opened_privkey_string);
  printf("%s\n", opened_pubkey_string);

  char* message = "chris is cool";
  printf("Message: %s\n", message);

  unsigned char* sig = Crypto::Sign(message, opened_privkey);
  if (sig == NULL) {
    printf("Could not generate signature!\n");
    return false;
  } else {
    printf("Signed message with private key-- signature: %x\n", sig);
  }

  printf("Verifying message with public key...\n");

  bool verified = Crypto::Verify(message, sig, opened_pubkey);
  if (verified) {
    printf("Verified! Signature is valid for message: %s\n", message);
  } else {
    printf("Not Verified...\n");
    return false;
  }

  unsigned char *sig2 = (unsigned char *) "asdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdf";

  bool verified2 = Crypto::Verify(message, sig2, opened_pubkey);
  if (verified2) {
    printf("Verified?! This shouldn't succeed...\n");
    return false;
  } else {
    printf("Not verified using bogus signature-- good!\n");
  }

  char* message2 = "chris is not cool?";
  bool verified3 = Crypto::Verify(message2, sig, opened_pubkey);
  if (verified3) {
    printf("Verified?! This shouldn't succeed...\n");
    return false;
  } else {
    printf("Not verified using bogus message-- good!\n");
  }

  int nonce_length = 32;
  unsigned char* nonce = Crypto::GenerateNonce(nonce_length);
  assert(nonce != NULL);
  for (int i = 0; i <= nonce_length; i++) {
    printf("%x", (char*)nonce[i]);
  }
  printf("\n");

  unsigned char* nonce2 = Crypto::GenerateNonce(nonce_length);
  assert(nonce != NULL);
  assert(strcmp((char*)nonce2, (char*)nonce) != 0);

  for (int i = 0; i <= nonce_length; i++) {
    printf("%x", (char*)nonce2[i]);
  }
  printf("\n");

  free(nonce);
  free(sig);
  free(privkey_string);
  free(pubkey_string);
  free(pubkey_string2);
  EVP_PKEY_free(privkey);

  return true;
}

}  // namespace TCPChat