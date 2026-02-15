#include"mprpcchannel.h"
#include"rpcheader.pb.h"
#include<sys/socket.h>
#include<error.h>
#include<zookeeperutil.h>
#include"mprpcapplication.h"
#include<arpa/inet.h>
#include<netinet/in.h>
#include<unistd.h>
#include"mprpccontroller.h"
#include"logger.h"
//heaer_size+service_name method_name args_size+args

//所有通过stub代理对象调用的rpc方法，都走到这里了，统一做rpc方法调用的数据序列化和网络发送
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller, const google::protobuf::Message* request,
                          google::protobuf::Message* response, google::protobuf::Closure* done)
{
    Logger& logger=Logger::GetInstance();
    const google::protobuf::ServiceDescriptor*sd=method->service();
    std::string service_name=sd->name();
    std::string method_name=method->name();
    //获取参数的序列化字符串长度args_size
    std::string args_str;
    int args_size;
    if(request->SerializeToString(&args_str)){
        args_size=args_str.size();
    }
    else{
        std::string msg="Serialize request error!"; 
        logger.Log(msg);
        std::cout<<"serialize request error!\n";
        controller->SetFailed("serialize request error!");
        return ;
    }
    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);

    std::string rpc_header_str;
    uint32_t header_size;
    if(rpcHeader.SerializeToString(&rpc_header_str)){
        header_size=rpc_header_str.size();
    }
    else{
        std::string msg="serialize rpcHeader error!"; 
        logger.Log(msg);
        std::cout<<"serialize rpcHeader error!\n";
        controller->SetFailed("serialize rpcHeader error!");
        return ;
    }
    //组织待发送的rpc请求的字符串
    std::string send_rpc_str;
    send_rpc_str.insert(0,std::string((char*)&header_size,4));
    send_rpc_str+=rpc_header_str;
    send_rpc_str+=args_str;

    int clientfd=socket(AF_INET,SOCK_STREAM,0);
    if(-1==clientfd){
        std::cout<<"create socket error! errno:"<<errno<<std::endl;
        char errtxt[512]={0};
        sprintf(errtxt,"create socket error! errno:%d",errno);
        controller->SetFailed(errtxt);
        logger.Log(errtxt);
        exit(EXIT_FAILURE);
    }
    // std::string ip=MprpcApplication::GetConfig().Load("rpcserverip");
    // uint16_t port=atoi(MprpcApplication::GetConfig().Load("rpcserverport").c_str());
    ZkClient zkCli;
    zkCli.Start();
    std::string method_path="/"+service_name+"/"+method_name;
    std::string host_data=zkCli.GetData(method_path.c_str());
    //无此方法
    if(host_data==""){
        controller->SetFailed("method not found!");
        logger.Log("method not found!");
        return ;
    }
    //查找ip和port
    int idx=host_data.find(":");
    if(idx==-1){
        controller->SetFailed("method not found!");
        logger.Log("method not found!");
        return ;
    }
    std::string ip=host_data.substr(0,idx);
    uint16_t port=atoi(host_data.substr(idx+1).c_str());
    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(port);
    server_addr.sin_addr.s_addr=inet_addr(ip.c_str());
    if(-1==connect(clientfd,(struct sockaddr*)&server_addr,sizeof(server_addr))){
        std::cout<<"connect error!errno:"<<errno<<std::endl;
        char errtxt[512]={0};
        sprintf(errtxt,"connect error! errno:%d",errno);
        controller->SetFailed(errtxt);
        logger.Log(errtxt);
        close(clientfd);
        exit(EXIT_FAILURE);
    }
    if(-1==send(clientfd,send_rpc_str.c_str(),send_rpc_str.size(),0)){
        close(clientfd);
        std::cout<<"send error!errno:"<<errno<<std::endl;
        char errtxt[512]={0};
        sprintf(errtxt,"send error! errno:%d",errno);
        controller->SetFailed(errtxt);
        logger.Log(errtxt);
        return ;
    }
    //接受rpc请求的响应
    char recv_buf[1024]={0};
    int recv_size=0;
    if(-1==(recv_size=recv(clientfd,recv_buf,1024,0))){
        close(clientfd);
        std::cout<<"recv error!errno:"<<errno<<std::endl;
        char errtxt[512]={0};
        sprintf(errtxt,"recv error! errno:%d",errno);
        controller->SetFailed(errtxt);
        logger.Log(errtxt);
        return ;
    }
    if(!response->ParseFromArray(recv_buf,recv_size)){
        close(clientfd);
        std::cout<<"parse error!response_str:"<<recv_buf<<std::endl;
        char errtxt[512]={0};
        sprintf(errtxt,"parse error! errno:%d",errno);
        controller->SetFailed(errtxt);
        logger.Log(errtxt);
        return ;
    }
    close(clientfd);
}
