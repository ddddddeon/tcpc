#include <string>

#include "client.h"
#include "../lib/logger.h"

void Client::Connect()
{
    Logger logger;

    logger.Info("Connected to " + Host + ":" + std::to_string(Port) + " as " + Name);
}