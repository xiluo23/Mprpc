#pragma once
#include"google/protobuf/service.h"
#include<memory>
#include<muduo/net/TcpServer.h>
#include<muduo/net/EventLoop.h>
#include<muduo/net/InetAddress.h>
#include<muduo/net/TcpConnection.h>
#include<string>
#include"mprpcapplication.h"
#include<functional>
#include<google/protobuf/descriptor.h>
#include<unordered_map>
#include"zookeeperutil.h"

class RpcProvider{
public:
    //可以发布rpc方法的函数接口
    void NotifyService(google::protobuf::Service*service);

    void Run();

private:
    std::unique_ptr<muduo::net::TcpServer>m_tcpserverPtr;
    muduo::net::EventLoop m_eventLoop;

    struct ServiceInfo{
        google::protobuf::Service* m_service;//服务对象
        std::unordered_map<std::string,const google::protobuf::MethodDescriptor*>m_methodMap;//保存服务方法
    };
    //注册成功的服务对象和其服务方法的所有信息
    std::unordered_map<std::string,ServiceInfo>m_serviceMap;

    void OnConnection(const muduo::net::TcpConnectionPtr&);
    void OnMessage(const muduo::net::TcpConnectionPtr&,muduo::net::Buffer*,muduo::Timestamp);
    //Closure的回调操作，用于序列化rpc的响应和网络发送
    void SendRpcResponse(const muduo::net::TcpConnectionPtr&,google::protobuf::Message*);

};