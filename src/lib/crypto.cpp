#include "crypto.h"

#include <string.h>

#include <iostream>
#include <regex>

namespace TCPChat {

namespace Crypto {

int GenerateKey(std::string path) { return 0; }

void WriteKeyToFile(int key, char *out) {}

int LoadKeyFromFile(std::string path) { return 0; }

std::string PubKeyToString(int pubkey) {}

int StringToPubKey(std::string pubkey_string) {}

std::string GenerateNonce() { return ""; }

std::string Sign(std::string message, int privkey) { return "foobar"; }

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