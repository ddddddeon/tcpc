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
The server acts as a message broker between all connected clients, but does not log any messages server-side. It accepts RSA public keys from clients on connection, then generates a random string and asks the client to sign the string with its private key. If the client signature is verified, the user is authenticated. The server maintains a local key/value database containing username/public key records.

To run the server:
```
// Will listen on localhost:9000 by default
tcpc-server

// Flags can be set to listen on a different interface or port
tcpc-server -i 0.0.0.0 -p 3030
```

## Client
The first time the client application is run, an RSA keypair will be generated and written to disk. The user can also specify a filepath pointing to an existing keypair. The client user will be able to claim usernames and associate them with their public key, and will have to use the same keypair to authenticate with these usernames going forward.

To run the client:
```bash
// If no host or port are specified, localhost and 9000 are the respective defaults
tcpc -h <host> -p <port>

// A custom keypair folder can be specified
tcpc -h <host> -p <port> -k /path/to/keypair-folder/

// A username can be specified on launch
tcpc -h <host> -p <port> -n alice

/* When a user is in the TCPC terminal, they can authenticate as a user (or claim it if it doesn't exist) by typing `/` followed by the username, and pressing enter. 
For example: /alice
```

## Important
This software is in development and as such is **not** production-ready! Transport-layer encryption has not yet been implemented.