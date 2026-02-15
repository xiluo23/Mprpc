#include"mprpcconfig.h"
#include<iostream>
#include<stdio.h>

void MprpcConfig::Trim(std::string&src_buf){
    int idx=src_buf.find_first_not_of(' ');
    if(idx!=-1){
        //有空格
        src_buf=src_buf.substr(idx,src_buf.size()-idx);
    }
    //去除字符串后面多余空格
    idx=src_buf.find_last_not_of(' ');
    if(idx!=-1){
        //有空格
        src_buf=src_buf.substr(0,idx+1);
    }
}

void MprpcConfig::LoadConfigFile(const char*config_file){
    FILE*fp=fopen(config_file,"r");
    if(fp==nullptr){
        std::cout<<config_file<<"is note exist!"<<std::endl;
        exit(EXIT_FAILURE);
    }
    //#注释  空格 回车
    int idx;
    while(!feof(fp)){
        char buf[512]={0};
        fgets(buf,sizeof(buf),fp);
        std::string src_buf(buf);
        //去回车
        if(src_buf.empty()||src_buf[0]=='\n'||src_buf[0]=='#')continue;
        //解析配置项
        idx=src_buf.find_first_of('=');
        if(idx==-1){
            //不合法
            continue;
        }
        std::string key=src_buf.substr(0,idx);
        std::string value=src_buf.substr(idx+1);
        //去除最后一个'\n'
        if(!value.empty()&&value.back()=='\n')
            value.pop_back();
        Trim(key);
        Trim(value);
        m_configMap.insert({key,value});
    }
}

std::string MprpcConfig::Load(const std::string&key){
    if(m_configMap.find(key)!=m_configMap.end())
        return m_configMap[key];
    return "";
}