#pragma once
#include"lockqueue.h"
#include<string>
enum LogLevel{
    INFO,//普通
    ERROR,//错误
};

class Logger{
public:
    void SetLogLevel(LogLevel level);
    void Log(std::string msg);
    static Logger&GetInstance(){
        static Logger instance;
        return instance;
    }
private:
    Logger();
    Logger(const Logger&)=delete;
    Logger(const Logger&&)=delete;
    int m_loglevel;//日志级别
    LockQueue<std::string>m_lckQue;
};

#define LOG_INFO(msg) Logger::GetInstance().Log(msg)
