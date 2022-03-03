# TCPC

A TCP socket-based, public-key authenticated, command-line client/server chat application.

## Installation
This application targets Linux only at the present, and depends on [dcrypt](https://github.com/ddddddeon/dcrypt), which itself depends on the OpenSSL `libcrypto` library. Both `TCPC` and `dcrypt` use the clang toolchain for compilation and linking, but the `Makefile` can be tweaked to use GNU build tools instead if you like. As it is, you will need 

```bash
// Install openssl, clang and lld-- for example in Ubuntu:
sudo apt-get install libssl-dev clang lld-12

// Download, build and install dcrypt
git clone https://github.com/ddddddeon/dcrypt
cd dcrypt
make
sudo make install

// Build TCPC
git clone https://github.com/ddddddeon/tcpc
make
sudo make install
```

## Server

## Client

## Important
This software is in development and as such is **not** production-ready! Transport-layer encryption has not yet been implemented.