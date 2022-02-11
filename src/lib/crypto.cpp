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
  ByteQueue bytes;
  FileSource file(path.c_str(), true, new Base64Decoder);
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

std::string GenerateNonce() {
  AutoSeededRandomPool rng;
  byte bytes[32];
  rng.GenerateBlock(bytes, 32);
  std::string nonce(bytes, bytes + 32);
  return nonce;
}

std::string Sign(std::string message, RSA::PrivateKey privkey) {
  AutoSeededRandomPool rng;
  RSASS<PSSR, SHA>::Signer signer(privkey);
  size_t max_length = signer.MaxSignatureLength();
  SecByteBlock signature(max_length);

  std::cout << "Message: " << message << std::endl;
  std::cout << "Message length: " << message.length() << std::endl;

  size_t length = signer.SignMessage(rng, (const byte *)message.c_str(),
                                     message.length(), (byte *)signature);
  signature.resize(length);

  std::cout << "Original size: " << signature.size() << std::endl;

  // std::string encoded;
  // CryptoPP::StringSource ss(signature.data(), signature.size(), true,
  //                           new StringSink(encoded));

  // std::cout << "Encoded size: " << encoded.size() << std::endl;

  std::string sig((const char *)signature.data(), signature.size());

  return sig;
}

bool Verify(std::string signature, std::string message, RSA::PublicKey pubkey) {
  // TODO remove
  std::cout << "Message: " << message << std::endl;
  std::cout << "Signature: " << signature.data() << std::endl;
  std::cout << "Signature size: " << signature.size() << std::endl;

  RSASS<PSSR, SHA>::Verifier verifier(pubkey);

  // Base64Decoder decoder;
  // decoder.Put((const byte *)signature.data(), signature.size());
  // decoder.MessageEnd();
  // size_t size = decoder.MaxRetrievable();
  // byte decoded[size];
  // decoder.Get((byte *)&decoded[0], size);

  SecByteBlock bytes((const byte *)signature.data(), signature.size());

  std::cout << "Message: " << message << std::endl;
  std::cout << "Message length: " << message.length() << std::endl;

  std::cout << "Signature: " << bytes.data() << std::endl;
  std::cout << "Signature size: " << bytes.size() << std::endl;

  // TODO figure out why this isn't verifying--
  // message.length and bytes.length match from client to server...
  bool verified = verifier.VerifyMessage((const byte *)message.c_str(),
                                         message.length(), bytes, bytes.size());
  return verified;
}

}  // namespace Crypto

}  // namespace TCPChat