#include "crypto.h"

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <string.h>

#include <iostream>
#include <regex>

namespace TCPChat {

namespace Crypto {

EVP_PKEY *GenerateKey() {
  EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
  EVP_PKEY_keygen_init(ctx);
  EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048);

  EVP_PKEY *key = NULL;
  EVP_PKEY_keygen(ctx, &key);

  EVP_PKEY_CTX_free(ctx);
  return key;
}

void WriteKeyToFile(int key, char *out) {}

int LoadKeyFromFile(std::string path) { return 0; }

// caller must free()
unsigned char *KeyToString(EVP_PKEY *key, bool is_private) {
  BIO *key_BIO = BIO_new(BIO_s_mem());

  if (is_private) {
    PEM_write_bio_PrivateKey(key_BIO, key, NULL, NULL, 0, 0, NULL);
  } else {
    PEM_write_bio_PUBKEY(key_BIO, key);
  }

  int key_length = BIO_pending(key_BIO);
  unsigned char *key_string = (unsigned char *)malloc(key_length);
  BIO_read(key_BIO, key_string, key_length);
  BIO_free(key_BIO);

  return key_string;
}

// caller must free()
EVP_PKEY *StringToKey(unsigned char *key_string, bool is_private) {
  BIO *key_BIO = BIO_new_mem_buf(key_string, -1);
  RSA *rsa = NULL;
  if (is_private) {
    PEM_read_bio_RSAPrivateKey(key_BIO, &rsa, NULL, NULL);
  } else {
    PEM_read_bio_RSA_PUBKEY(key_BIO, &rsa, NULL, NULL);
  }

  EVP_PKEY *key = EVP_PKEY_new();
  EVP_PKEY_assign_RSA(key, rsa);

  return key;
}

std::string GenerateNonce() { return ""; }

std::string Sign(std::string message, int key) { return "foobar"; }

bool Verify(std::string signature, std::string message, int pubkey) {
  return true;
}

std::string StripNewLines(std::string key) {
  return std::regex_replace(key, std::regex("\n"), "?");
}

std::string ExpandNewLines(std::string key) {
  return std::regex_replace(key, std::regex("\\?"), "\n");
}

}  // namespace Crypto

}  // namespace TCPChat