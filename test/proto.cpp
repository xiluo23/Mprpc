#include"test.pb.h"
#include<iostream>
using namespace fixbug;
using namespace std;

int main(){
    LoginResponse resp;
    resp.set_code(0);
    resp.set_msg("success");


    return 0;
}
