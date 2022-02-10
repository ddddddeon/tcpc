#include "crypto.h"

#include <string.h>

#include <iostream>
#include <regex>

using namespace CryptoPP;

namespace TCPChat {

namespace Crypto {

RSA::PrivateKey GenerateKey(std::string path) {
  AutoSeededRandomPool rng;
  RSA::PrivateKey privkey;
  privkey.GenerateRandomWithKeySize(rng, 2048);
  privkey.SetPublicExponent(65535);

  RSA::PublicKey pubkey(privkey);

  std::string privkey_out = path + "id_rsa";
  std::string pubkey_out = path + "id_rsa.pub";

  try {
    WriteKeyToFile(privkey, (char *)privkey_out.c_str());
    WriteKeyToFile(pubkey, (char *)pubkey_out.c_str());
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
  }

  return privkey;
}

void WriteKeyToFile(RSAFunction &key, char *out) {
  Base64Encoder encoder;
  FileSink output(out);
  ByteQueue queue;
  key.Save(queue);
  queue.CopyTo(encoder);
  encoder.MessageEnd();

  encoder.CopyTo(output);
  output.MessageEnd();
}

CryptoPP::ByteQueue LoadKeyFromFile(std::string path) {
  CryptoPP::ByteQueue bytes;
  CryptoPP::FileSource file(path.c_str(), true, new CryptoPP::Base64Decoder);
  file.TransferTo(bytes);
  bytes.MessageEnd();
  return bytes;
}

std::string PubKeyToString(RSA::PublicKey pubkey) {
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

RSA::PublicKey StringToPubKey(std::string pubkey_string) {
  StringSource pubkey_source(pubkey_string, true, new Base64Decoder);
  RSA::PublicKey pubkey;
  pubkey.Load(pubkey_source);

  return pubkey;
}

std::string StripNewLines(std::string key) {
  return std::regex_replace(key, std::regex("\n"), "?");
}

std::string ExpandNewLines(std::string key) {
  return std::regex_replace(key, std::regex("\\?"), "\n");
}

bool Verify(std::string signature, RSA::PublicKey pubkey) {
  std::cout << signature << std::endl;
  return true;
}

}  // namespace Crypto

}  // namespace TCPChat