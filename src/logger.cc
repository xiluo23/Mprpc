#include"logger.h"
#include<time.h>
#include<iostream>
Logger::Logger(){
    //启动写日志线程
    std::thread writeLogTask([&](){
        while(1){
            //获取当前日期，获取日志信息，写入日志文件
            time_t now=time(nullptr);
            tm*nowtm=localtime(&now);
            char file_name[128];
            sprintf(file_name,"%d-%d-%d-log.txt",nowtm->tm_year+1900,nowtm->tm_mon+1,nowtm->tm_mday);
            FILE*fp=fopen(file_name,"a+");
            if(fp==nullptr){
                std::cout<<"logger file:"<<file_name<<"open error"<<std::endl;
                exit(EXIT_FAILURE);
            }
            std::string msg=m_lckQue.Pop();
            char time_buf[128]={0};
            sprintf(time_buf,"%d-%d-%d =>[%s]",nowtm->tm_hour,nowtm->tm_min,nowtm->tm_sec,m_loglevel==INFO?"INFO":"ERR");
            msg.insert(0,time_buf);
            msg.append("\n");
            fputs(msg.c_str(),fp);
            fclose(fp);
        }
    });
    //线程分离
    writeLogTask.detach();
}

void Logger::SetLogLevel(LogLevel level){
    m_loglevel=level;
}
//把日志信息写入lockqueue
void Logger::Log(std::string msg){
    m_lckQue.Push(msg);
}


