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

    void Log(std::string str);
    void Info(std::string str);
    void Warn(std::string str);
    void Error(std::string str);
};