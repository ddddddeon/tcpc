#include "crypto.h"

#include <crypto++/base64.h>
#include <crypto++/files.h>
#include <crypto++/hex.h>
#include <crypto++/osrng.h>
#include <crypto++/rsa.h>

#include <iostream>

using namespace CryptoPP;

RSA::PrivateKey Crypto::GenerateKey() {
  AutoSeededRandomPool rng;
  RSA::PrivateKey privkey;
  privkey.GenerateRandomWithKeySize(rng, 2048);
  privkey.SetPublicExponent(65535);

  RSA::PublicKey pubkey(privkey);

  try {
    WriteKeyToFile(privkey, "./id_rsa");
    WriteKeyToFile(pubkey, "./id_rsa.pub");
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
  }

  return privkey;
}

void Crypto::WriteKeyToFile(RSAFunction &key, char *out) {
  Base64Encoder encoder;
  FileSink output(out);
  ByteQueue queue;
  key.Save(queue);
  queue.CopyTo(encoder);
  encoder.MessageEnd();

  encoder.CopyTo(output);
  output.MessageEnd();
}

std::string Crypto::PubKeyToString(RSA::PublicKey pubkey) {
  Base64Encoder encoder;
  std::string pubkey_string;
  StringSink output(pubkey_string);
  ByteQueue queue;
  pubkey.Save(queue);
  queue.CopyTo(encoder);
  encoder.MessageEnd();

  encoder.CopyTo(output);
  output.MessageEnd();

  return pubkey_string;
}

RSA::PublicKey Crypto::StringToPubKey(std::string pubkey_string) {
  StringSource pubkey_source(pubkey_string, true, new Base64Decoder);
  RSA::PublicKey pubkey;
  pubkey.Load(pubkey_source);

  return pubkey;
}