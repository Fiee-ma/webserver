#ifndef __WEBSERVER_SOCKADDR_H__
#define __WEBSERVER_SOCKADDR_H__

#include <memory>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include "log.h"

namespace server_name {

class IPAddress;
class Address {
public:
    typedef std::shared_ptr<Address> ptr;
    static Address::ptr Create(const sockaddr *addr, socklen_t addrlen);
    static bool Lookup(std::vector<Address::ptr> &result, const std::string &host,
            int family = AF_INET, int type = 0, int protocol = 0);
    static Address::ptr LookupAny(const std::string &host, int family, int type = 0, int protocol = 0);
    static std::shared_ptr<IPAddress> LookupAnyIPAdress(const std::string &host,
            int family = AF_INET, int type = 0, int protocol = 0);
    static bool GetInterfaceAddresses(std::multimap<std::string, std::pair<Address::ptr, uint32_t> > &result
            , int family = AF_INET);
    static bool GetInterfaceAddresses(std::vector<std::pair<Address::ptr, uint32_t> > &result
            , const std::string &iface, int family = AF_INET);
    virtual ~Address() {}

    int getFamily() const;

    virtual const sockaddr *getAddr() const = 0;
    virtual sockaddr *getAddr() = 0;
    virtual socklen_t getAddrLen() const = 0;

    virtual std::ostream &insert(std::ostream &os) const = 0;
    std::string toString();

    bool operator<(const Address &rhs) const;
    bool operator==(const Address &rhs) const;
    bool operator!=(const Address &rhs) const;
};

class IPAddress : public Address {
public:
    typedef std::shared_ptr<IPAddress> ptr;

    virtual IPAddress::ptr broadcastAddress(uint32_t prefix_len) = 0;
    virtual IPAddress::ptr networdAddress(uint32_t prefix_len) = 0;
    virtual IPAddress::ptr subnetMast(uint32_t prefix_len) = 0;

    virtual uint16_t getPort() const = 0;
    virtual void setPort(uint16_t v) = 0;
    static IPAddress::ptr Create(const char *address, uint16_t port = 0);
};

class IPv4Address : public IPAddress {
public:
    typedef std::shared_ptr<IPv4Address> ptr;
    IPv4Address(const sockaddr_in &address);
    IPv4Address(uint32_t address = INADDR_ANY, uint16_t port = 0);

    const sockaddr *getAddr() const override;
    sockaddr *getAddr() override;
    socklen_t getAddrLen() const override;

    std::ostream &insert(std::ostream &os) const override;

    IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
    IPAddress::ptr networdAddress(uint32_t prefix_len) override;
    IPAddress::ptr subnetMast(uint32_t prefix_len) override;

    uint16_t getPort() const override;
    void setPort(uint16_t v) override;
    static IPv4Address::ptr Create(const char *address, uint16_t port = 0);

private:
   sockaddr_in m_addr;
};

class IPv6Address : public IPAddress {
public:
    typedef std::shared_ptr<IPv6Address> ptr;
    IPv6Address();
    IPv6Address(const sockaddr_in6 &addr6);
    IPv6Address(const uint8_t address[16], uint16_t port = 0);

    const sockaddr *getAddr() const override;
    sockaddr *getAddr() override;
    socklen_t getAddrLen() const override;
    std::ostream &insert(std::ostream &os) const override;

    IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
    IPAddress::ptr networdAddress(uint32_t prefix_len) override;
    IPAddress::ptr subnetMast(uint32_t prefix_len) override;

    uint16_t getPort() const override;
    void setPort(uint16_t v) override;
    static IPv6Address::ptr Create(const char *address, uint16_t port = 0);

private:
   sockaddr_in6 m_addr;
};

class UnixAddress : public Address {
public:
    typedef std::shared_ptr<UnixAddress> ptr;
    UnixAddress();
    UnixAddress(const std::string &path);

    const sockaddr *getAddr() const override;
    sockaddr *getAddr() override;
    socklen_t getAddrLen() const override;
    std::ostream &insert(std::ostream &os) const override;
    void setAddrLen(uint32_t v);
    std::string getPath() const;
private:
    struct sockaddr_un m_addr;
    socklen_t m_length;
};

class UnknownAddress : public Address {
public:
    typedef std::shared_ptr<UnknownAddress> ptr;
    UnknownAddress(int family);
    UnknownAddress(const sockaddr &addr);

    const sockaddr *getAddr() const override;
    sockaddr *getAddr() override;
    socklen_t getAddrLen() const override;
    std::ostream &insert(std::ostream &os) const override;
private:
    sockaddr m_addr;
};

std::ostream& operator<<(std::ostream& os, const Address& addr);
}

#endif
