#ifndef LOGGER_H
#define LOGGER_H

#include <string>

#define INFO 0
#define WARN 1
#define ERROR 2

class Logger
{
public:
    int level;

    Logger()
    {
        level = 0;
    }

    void Info(std::string str);
    void Warn(std::string str);
    void Error(std::string str);
    void Name(std::string name, std::string str);
    void Log(std::string str);
};

#endif /* !LOGGER_H */