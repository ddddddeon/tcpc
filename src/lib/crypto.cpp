#include "crypto.h"

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <string.h>

#include <iostream>
#include <regex>

#define CHECK(func, msg, handle)                                  \
  do {                                                            \
    int ret = (func);                                             \
    if (ret != 1) {                                               \
      printf("%s (%s:%d: %s)\n", msg, __FILE__, __LINE__, #func); \
      handle;                                                     \
    }                                                             \
  } while (0)

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

void SetKey(BIO *bio, EVP_PKEY *key, bool is_private) {
  if (is_private) {
    PEM_write_bio_PrivateKey(bio, key, NULL, NULL, 0, 0, NULL);
  } else {
    PEM_write_bio_PUBKEY(bio, key);
  }
}

EVP_PKEY *GetKey(BIO *bio, bool is_private) {
  RSA *rsa = NULL;
  if (is_private) {
    PEM_read_bio_RSAPrivateKey(bio, &rsa, NULL, NULL);
  } else {
    PEM_read_bio_RSA_PUBKEY(bio, &rsa, NULL, NULL);
  }

  EVP_PKEY *key = EVP_PKEY_new();
  EVP_PKEY_assign_RSA(key, rsa);

  return key;
}

void KeyToFile(EVP_PKEY *key, char *out_file, bool is_private) {
  BIO *file_BIO = BIO_new_file(out_file, "w");
  SetKey(file_BIO, key, is_private);
  BIO_free(file_BIO);
}

unsigned char *KeyToString(EVP_PKEY *key, bool is_private) {
  BIO *key_BIO = BIO_new(BIO_s_mem());
  SetKey(key_BIO, key, is_private);
  int key_length = BIO_pending(key_BIO);
  unsigned char *key_string = (unsigned char *)malloc(key_length + 1);
  BIO_read(key_BIO, key_string, key_length);
  BIO_free(key_BIO);
  return key_string;
}

EVP_PKEY *FileToKey(char *in_file, bool is_private) {
  BIO *file_BIO = BIO_new_file(in_file, "rw");
  EVP_PKEY *key = GetKey(file_BIO, is_private);
  BIO_free(file_BIO);
  return key;
}

EVP_PKEY *StringToKey(unsigned char *key_string, bool is_private) {
  BIO *key_BIO = BIO_new_mem_buf(key_string, -1);
  EVP_PKEY *key = GetKey(key_BIO, is_private);
  BIO_free(key_BIO);
  return key;
}

unsigned char *Sign(char *message, EVP_PKEY *key) {
  // TODO handle errors
  size_t sig_length;

  // TODO check here
  EVP_MD_CTX *ctx = EVP_MD_CTX_create();

  CHECK(EVP_DigestSignInit(ctx, NULL, EVP_sha256(), NULL, key),
        "DigestSignInit failed", return NULL);
  CHECK(EVP_DigestSignUpdate(ctx, message, strlen(message)),
        "DigestSignUpdate failed", return NULL);
  CHECK(EVP_DigestSignFinal(ctx, NULL, &sig_length), "DigestSignFinal failed",
        return NULL);

  // TODO check here
  unsigned char *sig =
      (unsigned char *)OPENSSL_malloc(sizeof(unsigned char) * sig_length);

  CHECK(EVP_DigestSignFinal(ctx, sig, &sig_length), "DigestSignFinal failed",
        return NULL);

  EVP_MD_CTX_free(ctx);
  return sig;
}

bool Verify(char *message, unsigned char *signature, EVP_PKEY *pubkey) {
  size_t sig_length = 256;
  // TODO check here
  EVP_MD_CTX *ctx = EVP_MD_CTX_create();

  CHECK(EVP_DigestVerifyInit(ctx, NULL, EVP_sha256(), NULL, pubkey),
        "DigestVerifyInit failed", return false);

  CHECK(EVP_DigestVerifyUpdate(ctx, message, strlen(message)),
        "DigestVerifyUpdate failed", return false);

  CHECK(EVP_DigestVerifyFinal(ctx, signature, sig_length),
        "DigestVerifyFinal failed", return false);

  EVP_MD_CTX_free(ctx);
  return true;
}

std::string GenerateNonce() { return ""; }

std::string StripNewLines(std::string key) {
  return std::regex_replace(key, std::regex("\n"), "?");
}

std::string ExpandNewLines(std::string key) {
  return std::regex_replace(key, std::regex("\\?"), "\n");
}

}  // namespace Crypto

}  // namespace TCPChat