#include"mprpcapplication.h"
#include<iostream>
#include<unistd.h>
#include"logger.h"
MprpcConfig MprpcApplication::m_config;

MprpcConfig&MprpcApplication::GetConfig(){
    return m_config;
}

void showArgsHelp(){
    std::cout<<"format:command -i <configfile>"<<std::endl;
}
void MprpcApplication::Init(int argc,char**argv){
    if(argc<2){
        showArgsHelp();
        exit(EXIT_FAILURE);
    }
    int c=0;
    std::string config_file;
    while((c=getopt(argc,argv,"i:"))!=-1){
        switch(c){
            case 'i':
                config_file=optarg;
                break;
            case '?':
                std::cout<<"invalid args!"<<std::endl;
                showArgsHelp();
                exit(EXIT_FAILURE);
            case ':':
                std::cout<<"need <configfile>!"<<std::endl;
                showArgsHelp();
                exit(EXIT_FAILURE);
            default:
                break;
        }
    }
    //加载配置文件 rpcserver_ip rpcserver_port zookeeper_ip zookeeper_port
    m_config.LoadConfigFile(config_file.c_str());
    Logger& logger=Logger::GetInstance();
    std::string msg="m_config finish successfully!"; 
    logger.Log(msg);
}

MprpcApplication&MprpcApplication::GetInstance(){
    static MprpcApplication instance;
    return instance;
}