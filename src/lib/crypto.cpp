#include "crypto.h"

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <string.h>

#include <iostream>
#include <regex>

#define CHECK_MD(func, retval) \
  CHECK_EQUALS(1, func, EVP_MD_CTX_free(ctx); return retval)

#define CHECK_EQUALS(val, func, handle)                                  \
  do {                                                                   \
    int ret = (func);                                                    \
    if (ret != val) {                                                    \
      printf("%s (%s:%d: %s)\n", "Error at", __FILE__, __LINE__, #func); \
      handle;                                                            \
    }                                                                    \
  } while (0)

#define CHECK_NULL(func, message, retval)                             \
  do {                                                                \
    if ((func) == NULL) {                                             \
      printf("%s (%s:%d: %s)\n", message, __FILE__, __LINE__, #func); \
      return retval;                                                  \
    }                                                                 \
  } while (0)

#ifdef __cplusplus
namespace TCPChat {

namespace Crypto {
#endif

EVP_PKEY *GenerateKey() {
  EVP_PKEY_CTX *ctx = NULL;
  CHECK_NULL(ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL),
             "Could not create EVP_PKEY context", NULL);
  CHECK_EQUALS(1, EVP_PKEY_keygen_init(ctx), return NULL);
  EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048);

  EVP_PKEY *key = NULL;
  EVP_PKEY_keygen(ctx, &key);

  EVP_PKEY_CTX_free(ctx);
  return key;
}

bool SetKey(BIO *bio, EVP_PKEY *key, bool is_private) {
  if (is_private) {
    CHECK_EQUALS(1, PEM_write_bio_PrivateKey(bio, key, NULL, NULL, 0, 0, NULL),
                 return false);
  } else {
    CHECK_EQUALS(1, PEM_write_bio_PUBKEY(bio, key), return false);
  }

  return true;
}

EVP_PKEY *GetKey(BIO *bio, bool is_private) {
  RSA *rsa = NULL;
  if (is_private) {
    CHECK_NULL(PEM_read_bio_RSAPrivateKey(bio, &rsa, NULL, NULL),
               "Could not read private key from BIO", NULL);
  } else {
    CHECK_NULL(PEM_read_bio_RSA_PUBKEY(bio, &rsa, NULL, NULL),
               "Could not read public key from BIO", NULL);
  }

  EVP_PKEY *key = NULL;
  CHECK_NULL(key = EVP_PKEY_new(), "Could not allocat EVP_PKEY", NULL);
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
  size_t sig_length;
  EVP_MD_CTX *ctx = NULL;

  CHECK_NULL(ctx = EVP_MD_CTX_create(), "Could not initialize EVP context",
             NULL);

  CHECK_MD(EVP_DigestSignInit(ctx, NULL, EVP_sha256(), NULL, key), NULL);
  CHECK_MD(EVP_DigestSignUpdate(ctx, message, strlen(message)), NULL);
  CHECK_MD(EVP_DigestSignFinal(ctx, NULL, &sig_length), NULL);

  unsigned char *sig = NULL;
  CHECK_NULL(sig = (unsigned char *)malloc(sig_length),
             "Could not allocate memory for signature", NULL);

  CHECK_MD(EVP_DigestSignFinal(ctx, sig, &sig_length), NULL);

  EVP_MD_CTX_free(ctx);
  return sig;
}

bool Verify(char *message, unsigned char *signature, EVP_PKEY *pubkey) {
  size_t sig_length = 256;
  EVP_MD_CTX *ctx = NULL;

  CHECK_NULL(ctx = EVP_MD_CTX_create(), "Could not initialize EVP context",
             false);

  CHECK_MD(EVP_DigestVerifyInit(ctx, NULL, EVP_sha256(), NULL, pubkey), false);
  CHECK_MD(EVP_DigestVerifyUpdate(ctx, message, strlen(message)), false);
  CHECK_MD(EVP_DigestVerifyFinal(ctx, signature, sig_length), false);

  EVP_MD_CTX_free(ctx);
  return true;
}

// TODO don't use STL here
std::string GenerateNonce() { return ""; }

std::string StripNewLines(std::string key) {
  return std::regex_replace(key, std::regex("\n"), "?");
}

std::string ExpandNewLines(std::string key) {
  return std::regex_replace(key, std::regex("\\?"), "\n");
}

#ifdef __cplusplus
}  // namespace Crypto

}  // namespace TCPChat
#endif