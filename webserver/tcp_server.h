#ifndef __WEBSERVER_TCP_SERVER_H__
#define __WEBSERVER_TCP_SERVER_H__

#include <memory>
#include <functional>
#include "sockaddr.h"
#include "iomanager.h"
#include "socket.h"
#include "noncopyable.h"

namespace server_name {

/**
 * @brief TCP服务器封装
 */
class TcpServer : public std::enable_shared_from_this<TcpServer>
                    , Noncopyable {
public:
    typedef std::shared_ptr<TcpServer> ptr;
    /**
     * @brief 构造函数
     * @param[in] worker socket客户端工作的协程调度器
     * @param[in] accept_worker 服务器socket执行接收socket连接的协程调度器
     */
    TcpServer(IOManager* worker = server_name::IOManager::GetThis()
              ,IOManager* io_woker = server_name::IOManager::GetThis()
              ,IOManager* accept_worker = server_name::IOManager::GetThis());

    /**
     * @brief 析构函数
     */
    virtual ~TcpServer();

    /**
     * @brief 绑定地址
     * @return 返回是否绑定成功
     */
    virtual bool bind(Address::ptr addr, bool ssl = false);

    /**
     * @brief 绑定地址数组
     * @param[in] addrs 需要绑定的地址数组
     * @param[out] fails 绑定失败的地址
     * @return 是否绑定成功
     */
    virtual bool bind(const std::vector<Address::ptr>& addrs
                        ,std::vector<Address::ptr>& fails
                        ,bool ssl = false);

    bool loadCertificates(const std::string& cert_file, const std::string& key_file);

    /**
     * @brief 启动服务
     * @pre 需要bind成功后执行
     */
    virtual bool start();

    /**
     * @brief 停止服务
     */
    virtual void stop();

    /**
     * @brief 返回读取超时时间(毫秒)
     */
    uint64_t getRecvTimeout() const { return m_recvTimeout;}

    /**
     * @brief 返回服务器名称
     */
    std::string getName() const { return m_name;}

    /**
     * @brief 设置读取超时时间(毫秒)
     */
    void setRecvTimeout(uint64_t v) { m_recvTimeout = v;}

    /**
     * @brief 设置服务器名称
     */
    virtual void setName(const std::string& v) { m_name = v;}

    /**
     * @brief 是否停止
     */
    bool isStop() const { return m_isStop;}

//    TcpServerConf::ptr getConf() const { return m_conf;}
//    void setConf(TcpServerConf::ptr v) { m_conf = v;}
//    void setConf(const TcpServerConf& v);

    virtual std::string toString(const std::string& prefix = "");

    std::vector<Socket::ptr> getSocks() const { return m_socks;}
protected:

    /**
     * @brief 处理新连接的Socket类
     */
    virtual void handleClient(Socket::ptr client);

    /**
     * @brief 开始接受连接
     */
    virtual void startAccept(Socket::ptr sock);
protected:
    /// 监听Socket数组
    std::vector<Socket::ptr> m_socks;
    /// 新连接的Socket工作的调度器
    IOManager* m_worker;
    IOManager* m_ioWorker;
    /// 服务器Socket接收连接的调度器
    IOManager* m_acceptWorker;
    /// 接收超时时间(毫秒)
    uint64_t m_recvTimeout;
    /// 服务器名称
    std::string m_name;
    /// 服务器类型
    std::string m_type = "tcp";
    /// 服务是否停止
    bool m_isStop;

    bool m_ssl = false;

//    TcpServerConf::ptr m_conf;
};

}




#endif
