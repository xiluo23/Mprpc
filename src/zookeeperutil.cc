#include "zookeeperutil.h"
#include "mprpcapplication.h"
#include <semaphore.h>

// 全局观察器
void global_watcher(zhandle_t* zh, int type, int state, const char* path, void* watchCtx)
{
    if (type == ZOO_SESSION_EVENT) {
        if (state == ZOO_CONNECTED_STATE) {
            sem_t* sem = (sem_t*)zoo_get_context(zh);
            if (sem != nullptr) {
                sem_post(sem);
            }
        }
    }
}

// exists 回调
void exists_completion(int rc, const struct Stat* stat, const void* data)
{
    int* result = (int*)data;
    *result = rc;
}

// create 回调
void create_completion(int rc, const char* value, const void* data)
{
    int* result = (int*)data;
    *result = rc;
}

// get 回调
void get_completion(int rc, const char* value, int value_len,
                    const struct Stat* stat, const void* data)
{
    if (rc == ZOK) {
        std::string* out = (std::string*)data;
        out->assign(value, value_len);
    }
}

ZkClient::ZkClient() : m_zhandle(nullptr) {}

ZkClient::~ZkClient()
{
    if (m_zhandle) {
        zookeeper_close(m_zhandle);
    }
}

// 连接 zk server
void ZkClient::Start()
{
    std::string host = MprpcApplication::GetConfig().Load("zookeeperip");
    std::string port = MprpcApplication::GetConfig().Load("zookeeperport");
    std::string connstr = host + ":" + port;

    m_zhandle = zookeeper_init(connstr.c_str(), global_watcher, 30000, nullptr, nullptr, 0);
    if (!m_zhandle) {
        std::cout << "zookeeper_init error!\n";
        exit(EXIT_FAILURE);
    }

    sem_t sem;
    sem_init(&sem, 0, 0);
    zoo_set_context(m_zhandle, &sem);
    sem_wait(&sem);
    zoo_set_context(m_zhandle, nullptr);

    std::cout << "zookeeper_init success\n";
}

void ZkClient::Create(const char* path, const char* data, int datalen, int state)
{
    char path_buffer[128];
    int bufferlen = sizeof(path_buffer);
    int flag;
    
    // Check if node exists
    flag = zoo_exists(m_zhandle, path, 0, nullptr);
    if (ZNONODE == flag) {
        // Create node
        flag = zoo_create(m_zhandle, path, data, datalen, &ZOO_OPEN_ACL_UNSAFE, state, path_buffer, bufferlen);
        if (flag == ZOK) {
            std::cout << "znode create success...path:" << path << std::endl;
        } else {
            std::cout << "flag:" << flag << std::endl;
            std::cout << "znode create error...path:" << path << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

std::string ZkClient::GetData(const char* path)
{
    char buffer[64];
    int bufferlen = sizeof(buffer);
    int flag = zoo_get(m_zhandle, path, 0, buffer, &bufferlen, nullptr);
    if (flag != ZOK) {
        std::cout << "get znode error...path:" << path << std::endl;
        return "";
    }
    else {
        return std::string(buffer, bufferlen);
    }
}
