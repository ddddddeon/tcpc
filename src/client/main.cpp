#include <iostream>

#include "client.h"

int main(int argc, char *argv[])
{
    Client client("localhost", 9000, "chris");

    client.Connect();
}