#include<iostream>
#include<string>
#include"friend.pb.h"
#include"mprpcapplication.h"
#include"rpcprovider.h"
#include<vector>
#include"logger.h"
#include"user.pb.h"
using namespace std;

class FriendService:public fixbug::FriendsServiceRpc{
public:
    std::vector<std::string>GetFriendsList(uint32_t userid){
        std::cout<<"do GetFriendsList service!\n";
        std::vector<std::string>vec;
        vec.push_back("wang wu");
        vec.push_back("liu hai");
        vec.push_back("jack");
        return vec;
    }
    void GetFriendsList(::google::protobuf::RpcController* controller,
                       const ::fixbug::GetFriendsListRequest* request,
                       ::fixbug::GetFriendsListResponse* response,
                       ::google::protobuf::Closure* done){
        uint32_t userid=request->userid();
        std::vector<std::string>friendsList=GetFriendsList(userid);
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        for(auto&name:friendsList){
            response->add_friends(name);
        }
        done->Run();
    }


private:
};

int main(int argc,char**argv){
    Logger& logger=Logger::GetInstance();
    std::string msg="callee start successfully!"; 
    logger.Log(msg);
    //调用框架初始化
    MprpcApplication::Init(argc,argv);

    //把FriendService发布到rpc结点
    RpcProvider provider;
    provider.NotifyService(new FriendService());
    //启动,进入阻塞状态 
    provider.Run();
    msg="callee stop successfully!"; 
    logger.Log(msg);
    return 0;
}