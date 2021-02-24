#ifndef __WEBSERVER_IOBYTEARRAY_H__
#define __WEBSERVER_IOBYTEARRAY_H__

#include <memory>
#include <string>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <vector>

namespace server_name {

class ByteArray {
public:
    typedef std::shared_ptr<ByteArray> ptr;

    struct Node {
        Node(size_t s);
        Node();
        ~Node();

        char *ptr;
        Node *next;
        size_t size;
    };

    ByteArray(size_t base_size = 4096);
    ~ByteArray();

    //write
    void writeFint8(int8_t value);
    void writeFuint8(uint8_t value);
    void writeFint16(int16_t value);
    void writeFuint16(uint16_t value);
    void writeFint32(int32_t value);
    void writeFuint32(uint32_t value);
    void writeFint64(int64_t value);
    void writeFuint64(uint64_t value);

    void writeInt32(int32_t value);
    void writeUint32(uint32_t value);
    void writeInt64(int64_t value);
    void writeUint64(uint64_t value);

    void writeFloat(float value);
    void writeDouble(double value);

    void writeStringF16(const std::string &value);
    void writeStringF32(const std::string &value);
    void writeStringF64(const std::string &value);
    void writeStringVint(const std::string &value);
    void writeStringWithoutLength(const std::string &value);

    //read
    int8_t readFint8();
    uint8_t readFuint8();
    int16_t readFint16();
    uint16_t readFuint16();
    int32_t readFint32();
    uint32_t readFuint32();
    int64_t readFint64();
    uint64_t readFuint64();

    int32_t readInt32();
    uint32_t readUint32();
    int64_t readInt64();
    uint64_t readUint64();

    float readFloat();
    double readDouble();
    std::string readStringF16();
    std::string readStringF32();
    std::string readStringF64();
    std::string readStringVint();

    //内部操作
    void clear();

    /**
     * 写入size长度的数据
     * buf为内存缓冲指针
     * size是数据大小
     */
    void write(const void *buf, size_t size);
    /**
     * 读取size长度的数据
     * buf为内存缓冲指针
     * size是数据大小
     */
    void read(void *buf, size_t size);
    void read(void *buf, size_t size, size_t position) const;

    size_t getPosition() const { return m_position;}
    void setPosition(size_t v);
    bool writeToFile(const std::string &name) const;
    bool readFromFile(const std::string &name);
    size_t getBaseSize() const {return m_baseSize;}
    /// 返回可读数据大小
    size_t getReadSize() const { return m_size - m_position;}
    /// 判断是否是小端
    bool isLittleEndian() const;
    /// 是否设置为小端
    void setIsLittleEndian(bool val);
    ///将ByteArray里面的数据[m_position, m_size)转成std::string
    std::string toString() const;
    std::string toHexString() const;
    uint64_t getReadBuffers(std::vector<iovec>& buffers, uint64_t len = ~0ull) const;
    uint64_t getReadBuffers(std::vector<iovec>& buffers, uint64_t len, uint64_t position) const;
    uint64_t getWriteBuffers(std::vector<iovec>& buffers, uint64_t len);
    /// 返回数据长度
    size_t getSize() const { return m_size;}

private:
    void addCapacity(size_t size);
    size_t getCapacity() const {return m_capacity - m_position;}
private:
    /// 内存块的大小
    size_t m_baseSize;
    /// 当前操作位置
    size_t m_position;
    /// 当前的总容量
    size_t m_capacity;
    /// 当前的数据大小
    size_t m_size;
    /// 字节序 默认为大端
    int8_t m_endian;
    /// 第一个内存块指针
    Node *m_root;
    /// 当前操作的内存指针
    Node *m_cur;
};





}

#endif
