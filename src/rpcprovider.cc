#include"rpcprovider.h"
#include"rpcheader.pb.h"
#include"logger.h"

//提供给外部使用的，可以发布rpc方法的函数接口
void RpcProvider::NotifyService(google::protobuf::Service*service){
    ServiceInfo service_info;
    Logger&logger=Logger::GetInstance();
    //获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor*pServiceDesc=service->GetDescriptor();
    //获取服务名字
    std::string service_name=pServiceDesc->name();
    //获取服务对象service的方法数量
    int methodCnt=pServiceDesc->method_count();
    for(int i=0;i<methodCnt;i++){
        //获取服务对象指定下标的服务方法描述
        const google::protobuf::MethodDescriptor*pmethodDesc=pServiceDesc->method(i);
        std::string method_name=pmethodDesc->name();
        service_info.m_methodMap.insert({method_name,pmethodDesc});
    }
    service_info.m_service=service;
    m_serviceMap.insert({service_name,service_info});

    char msg[128];
    sprintf(msg,"%s init finish successfully!",service_name.c_str());
    logger.Log(msg);
}

void RpcProvider::Run(){
    std::string ip=MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port=atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip,port);

    //创建TcpServer
    muduo::net::TcpServer server(&m_eventLoop,address,"Rpcprovider");
    //绑定连接回调
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection,this,std::placeholders::_1));
    //绑定消息读写回调
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
    //设置线程数量
    server.setThreadNum(4);

    //把当前rpc节点上要发布的服务全部注册到zk上，让rpc cli可以从zk上获取服务
    ZkClient zkCli;
    zkCli.Start();
    //service_name永久节点，method_name临时方法
    for(auto&sp:m_serviceMap){
        std::string service_path="/"+sp.first;
        zkCli.Create(service_path.c_str(),nullptr,0);
        for(auto&mp:sp.second.m_methodMap){
            //service_name/method_name
            std::string method_path=service_path+"/"+mp.first;
            char method_path_data[128];
            sprintf(method_path_data,"%s:%d",ip.c_str(),port);
            zkCli.Create(method_path.c_str(),method_path_data,strlen(method_path_data),ZOO_EPHEMERAL);
            //ZOO_EPHEMERAL临时性方法
        }
    }
    std::cout<<"RpcProvider start service at ip:"<<ip<<" port:"<<port<<std::endl;

    //启动网络服务
    server.start();
    m_eventLoop.loop();
}
//socket连接回调
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr&conn){
    if(!conn->connected()){
        conn->shutdown();
    }

}
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr&conn,muduo::net::Buffer*buffer,muduo::Timestamp timestamp){
    std::string recv_buf=buffer->retrieveAllAsString();
    Logger&logger=Logger::GetInstance();
    //读取前4个字节内容
    uint32_t header_size=0;
    recv_buf.copy((char*)&header_size,4,0);
    //根据header_size读取数据头的原始字节序
    std::string rpc_header_str=recv_buf.substr(4,header_size);
    mprpc::RpcHeader rpcHeader;
    uint32_t args_size;
    std::string service_name,method_name; 
    if(rpcHeader.ParseFromString(rpc_header_str)){
        //成功
        service_name=rpcHeader.service_name();
        method_name=rpcHeader.method_name();
        args_size=rpcHeader.args_size();

    }
    else{
        std::cout<<"rpc_header_str:"<<rpc_header_str<<" parse error!\n";
        return ;
    }
    //获取rpc方法参数字符流数据
    std::string args_str=recv_buf.substr(4+header_size,args_size);

    //获取service对象和method对象
    auto it=m_serviceMap.find(service_name);
    if(it==m_serviceMap.end()){
        std::cout<<service_name<<" is not exist!\n";
        return ;
    }
    
    auto mit=it->second.m_methodMap.find(method_name);
    if(mit==it->second.m_methodMap.end()){
        std::cout<<service_name<<": "<<method_name<<" is not exist!\n";
        return ;
    }
    google::protobuf::Service* service=it->second.m_service;
    const google::protobuf::MethodDescriptor*method=mit->second;

    //生成rpc方法调用的request和response
    google::protobuf::Message* request=service->GetRequestPrototype(method).New();
    if(!request->ParseFromString(args_str)){
        std::cout<<"request parse error!content:"<<args_str<<std::endl;
        return ;
    }
    google::protobuf::Message* response=service->GetResponsePrototype(method).New();
    //给下面的method方法调用，绑定一个Closure回调
    google::protobuf::Closure*done=google::protobuf::NewCallback<RpcProvider,const muduo::net::TcpConnectionPtr&,google::protobuf::Message*>
        (this,&RpcProvider::SendRpcResponse,conn,response);
    
    //在框架上根据远端rpc请求，调用当前rpc节点上发布的方法
    service->CallMethod(method,nullptr,request,response,done);
    char msg[128];
    sprintf(msg,"connection %s call service: %s method: %s",conn->name().c_str(),service_name.c_str(),method_name.c_str());
    logger.Log(msg);
}


void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr&conn,google::protobuf::Message*response){
    std::string response_str;
    if(response->SerializeToString(&response_str)){
        //序列化成功后，通过网络把rpc方法的执行结果发送给调用方
        conn->send(response_str);
    }
    else{
        std::cout<<"serialize response_str error!\n";
    }
    conn->shutdown();
}
