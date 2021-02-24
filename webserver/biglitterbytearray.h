#ifndef __WEBSERVER_BIGLITTERENDIAN_H__
#define __WEBSERVER_BIGLITTERENDIAN_H__

#define WEBSERVER_LITTLE_ENDIAN 1
#define WEBSERVER_BIG_ENDIAN 2

#include <byteswap.h>
#include <stdint.h>
#include <iostream>

namespace server_name {

template <class T>
typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type
byteswap(T value) {
    return (T)bswap_64((uint64_t)value);
}

template <class T>
typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type
byteswap(T value) {
    return (T)bswap_32((uint32_t)value);
}

template <class T>
typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type
byteswap(T value) {
    return (T)bswap_16((uint16_t)value);
}

#if BYTE_ORDER == BIG_ENDIAN
#define WEBSERVER_BYTE_ORDER WEBSERVER_BIG_ENDIAN
#else
#define WEBSERVER_BYTE_ORDER WEBSERVER_LITTLE_ENDIAN
#endif

#if WEBSERVER_BYTE_ORDER == WEBSERVER_LITTLE_ENDIAN

template <class T>
T byteswapOnLittleEndian(T t) {
//    std::cout <<"1.byteswapOnLittleEndian" << std::endl;
    return t;
}

template<class T>
T byteswapOnBigEndian(T t) {
    return byteswap(t);
}

#else

T byteswapOnLittleEndian(T t) {
    std::cout <<"2.byteswapOnLittleEndian" << std::endl;
    return byteswap(t);
}

template<class T>
T byteswapOnBigEndian(T t) {
    return t;
}

#endif
}

#endif
