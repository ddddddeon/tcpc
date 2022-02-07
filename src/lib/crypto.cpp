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

  std::cout << "Generating public key..." << std::endl;

  RSA::PublicKey pubkey(privkey);

  std::cout << "Encoding..." << std::endl;

  // std::string encodedPriv, encodedPub;
  // Base64Encoder pubKeySink(new StringSink(encodedPub));
  // pubkey.DEREncode(pubKeySink);
  // std::cout << encodedPub << std::endl;

  WriteKeyToFile(privkey, "./id_rsa");
  WriteKeyToFile(pubkey, "./id_rsa.pub");

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