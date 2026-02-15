#include<iostream>
#include"mprpcapplication.h"
#include"friend.pb.h"
#include"mprpcchannel.h"
#include"mprpccontroller.h"
#include"logger.h"
int main(int argc,char**argv){
    MprpcApplication::Init(argc,argv);
    
    fixbug::FriendsServiceRpc_Stub stub(new MprpcChannel());
    
    fixbug::GetFriendsListRequest request;
    request.set_userid(1);
    fixbug::GetFriendsListResponse response;
    MprpcController controller;
    stub.GetFriendsList(&controller,&request,&response,nullptr);
    if(controller.Failed()){
        std::cout<<controller.ErrorText()<<std::endl;
        exit(EXIT_FAILURE);
    }
    if(response.result().errcode()==0){
        std::cout<<"rpc GetFriendsList success:"<<std::endl;
        for(int i=0;i<response.friends().size();i++){
            std::cout<<response.friends(i)<<std::endl;
        }
    }
    else{
        std::cout<<"rpc GetFriendsList error::"<<response.result().errcode()<<std::endl;
    }
    return 0;
}