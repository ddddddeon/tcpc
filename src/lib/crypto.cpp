#include "crypto.h"

#include <iostream>
#include <regex>

using namespace CryptoPP;

namespace TCPChat {

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

CryptoPP::ByteQueue Crypto::LoadKeyFromFile(std::string path) {
  CryptoPP::ByteQueue bytes;
  CryptoPP::FileSource file(path.c_str(), true, new CryptoPP::Base64Decoder);
  file.TransferTo(bytes);
  bytes.MessageEnd();
  return bytes;
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

std::string Crypto::StripNewLines(std::string key) {
  return std::regex_replace(key, std::regex("\n"), "?");
}

std::string Crypto::ExpandNewLines(std::string key) {
  return std::regex_replace(key, std::regex("\\?"), "\n");
}

}  // namespace TCPChat