#ifndef __SYLAR_CONFIG_H__
#define __SYLAR_CONFIG_H__

#include <boost/lexical_cast.hpp>
#include "log.h"
#include <string>
#include<yaml-cpp/yaml.h>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include<functional>

namespace server_name {

class ConfigVarBase {
public:
    typedef std::shared_ptr<ConfigVarBase> ptr;
    ConfigVarBase(const std::string& name, const std::string &description = "")
        :m_name(name)
        ,m_description(description){
            std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
    }
    virtual ~ConfigVarBase() {}

    const std::string &getName() const { return m_name;}
    const std::string &getDescription() const { return m_description;}

    virtual std::string toString() = 0;
    virtual bool fromString(const std::string &val) = 0;
    virtual std::string getTypeName() = 0 ;
protected:
    std::string m_name;
    std::string m_description;
};

template<class F, class T>
class LexicalCast {
public:
    T operator()(const F &val) {
        return boost::lexical_cast<T>(val);
    }
};

template<class T>
class LexicalCast<std::string, std::vector<T> > {
public:
    std::vector<T> operator()(const std::string &v) {
        YAML::Node node = YAML::Load(v);
        typename std::vector<T> vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

template<class T>
class LexicalCast<std::vector<T>, std::string> {
public:
    std::string operator()(const std::vector<T> &v) {
        YAML::Node node;
        std::stringstream ss;
        for(auto &i : v){
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        ss << node;
        return ss.str();
    }
};

template<class T>
class LexicalCast<std::string, std::list<T> > {
public:
    std::list<T> operator()(const std::string &v) {
        YAML::Node node = YAML::Load(v);
        typename std::list<T> vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

template<class T>
class LexicalCast<std::list<T>, std::string> {
public:
    std::string operator()(const std::list<T> &v) {
        YAML::Node node;
        std::stringstream ss;
        for(auto &i : v){
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        ss << node;
        return ss.str();
    }
};

template<class T>
class LexicalCast<std::string, std::set<T> > {
public:
    std::set<T> operator()(const std::string &v) {
        YAML::Node node = YAML::Load(v);
        typename std::set<T> vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

template<class T>
class LexicalCast<std::set<T>, std::string> {
public:
    std::string operator()(const std::set<T> &v) {
        YAML::Node node;
        std::stringstream ss;
        for(auto &i : v){
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        ss << node;
        return ss.str();
    }
};

template<class T>
class LexicalCast<std::string, std::unordered_set<T> > {
public:
    std::unordered_set<T> operator()(const std::string &v) {
        YAML::Node node = YAML::Load(v);
        typename std::unordered_set<T> vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

template<class T>
class LexicalCast<std::unordered_set<T>, std::string> {
public:
    std::string operator()(const std::unordered_set<T> &v) {
        YAML::Node node;
        std::stringstream ss;
        for(auto &i : v){
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        ss << node;
        return ss.str();
    }
};


template<class T>
class LexicalCast<std::string, std::map<std::string, T> > {
public:
    std::map<std::string, T> operator()(const std::string &v) {
        YAML::Node node = YAML::Load(v);
        typename std::map<std::string, T> mp;
        std::stringstream ss;
        for(auto it = node.begin(); it != node.end(); ++it) {
            ss.str("");
            ss << it->second;
            mp.insert(std::make_pair(it->first.Scalar(),
                        LexicalCast<std::string, T>()(ss.str())));
        }
        return mp;
    }
};

template<class T>
class LexicalCast<std::map<std::string, T>, std::string> {
public:
    std::string operator()(const std::map<std::string, T> &v) {
        YAML::Node node;
        std::stringstream ss;
        for(auto &i : v){
            node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
        }
        ss << node;
        return ss.str();
    }
};

template<class T>
class LexicalCast<std::string, std::unordered_map<std::string, T> > {
public:
    std::unordered_map<std::string, T> operator()(const std::string &v) {
        YAML::Node node = YAML::Load(v);
        typename std::unordered_map<std::string, T> mp;
        std::stringstream ss;
        for(auto it = node.begin(); it != node.end(); ++it) {
            ss.str("");
            ss << it->second;
            mp.insert(std::make_pair(it->first.Scalar(),
                        LexicalCast<std::string, T>()(ss.str())));
        }
        return mp;
    }
};

template<class T>
class LexicalCast<std::unordered_map<std::string, T>, std::string> {
public:
    std::string operator()(const std::unordered_map<std::string, T> &v) {
        YAML::Node node;
        std::stringstream ss;
        for(auto &i : v){
            node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
        }
        ss << node;
        return ss.str();
    }
};


//FromStr T operator() (const std::string&)
//Tostr std::string operator() (const T&)
template<class T, class ToStr = LexicalCast<T, std::string>,
    class FromStr = LexicalCast<std::string, T> >
class ConfigVar : public ConfigVarBase {
public:
    typedef std::shared_ptr<ConfigVar> ptr;
    typedef std::function<void(const T &old_value, const T &new_value)> on_change_cb;
    typedef RWMutex RWMutexType;

    ConfigVar(const std::string &name, const T &default_name, const std::string &description = "")
        :ConfigVarBase(name, description)
        , m_val(default_name){
    }

    std::string toString() override {
        try {
            //return boost::lexical_cast<std::string>(m_val);
            RWMutexType::ReadLock lock(m_mutex);
            return ToStr()(m_val);
        } catch (std::exception &e) {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ConfigVar::toString exception"
                << e.what() << " content: " << typeid(m_val).name() << " to string";
        }
        return "";
    }

    bool fromString(const std::string &val) override {
        try {
            //m_val = boost::lexical_cast<T>(val);
            setValue(FromStr()(val));
        } catch (std::exception &e) {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ConfigVar::fromString exception"
                << e.what() << " content: string to " << typeid(m_val).name();
        }
        return false;
    }

    const T &getValue() {
        RWMutexType::ReadLock lock(m_mutex);
        return m_val;
    }
    void setValue(const T & v) {
        {
            RWMutexType::ReadLock lock(m_mutex);
            if(m_val == v){
                return;
            }

            for(auto &i : m_cbs) {
                i.second(m_val, v);
            }
        }
        RWMutexType::WriteLock lock(m_mutex);
        m_val = v;
    }
    std::string getTypeName() override {return typeid(T).name();}

    uint64_t addListener(on_change_cb cd) {
        static uint64_t s_fun_id = 0;
        RWMutexType::WriteLock lock(m_mutex);
        ++s_fun_id;
        m_cbs[s_fun_id] = cd;

        return s_fun_id;
    }

    void delListener(uint64_t key) {
        RWMutexType::WriteLock lock(m_mutex);
        m_cbs.erase(key);
    }

    on_change_cb getListener(uint64_t key) {
        RWMutexType::ReadLock lock(m_mutex);
        auto it = m_cbs.find(key);
        return it == m_cbs.end() ? nullptr : it->second;
    }

    void clearListener() {
        RWMutexType::WriteLock lock(m_mutex);
        m_cbs.clear();
    }
private:
        T m_val;
        //变更回调函数，uint64_ key,要求唯一，　一般可以用hash
        std::map<uint64_t, on_change_cb> m_cbs;
        RWMutexType m_mutex;
};

class Config {
public:
    typedef std::map<std::string, ConfigVarBase::ptr> ConfigVarMap;
    typedef RWMutex RWMutexType;

    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string &name,
            const T &default_value, const std::string &description = ""){
      //  auto tmp = Lookup<T>(name);
        RWMutexType::WriteLock lock(GetMutex());
        auto it = GetDates().find(name);
        if(it != GetDates().end()){
            auto tmp = std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
            if(tmp) {
                SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Lookup name = " << name << " exists";
                return tmp;
            } else {
                SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Lookup name = " << name << " exists but type not "
                    << typeid(T).name() << " real_type = " << it->second->getTypeName()
                    << " " << it->second->toString();
                return nullptr;
            }
        }

        //find_first_not_of()正向查找在原字符串中第一个与指定字符串（或字符）中的任一字符都
        //不匹配的字符，返回它的位置。若查找失败，则返回npos。
        if(name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789")
                != std::string::npos) {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Lookup name invalid "<< name;
            throw std::invalid_argument(name);
        }

        typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
        GetDates()[name] = v;

        return v;
    }

    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string name) {
        RWMutexType::ReadLock lock(GetMutex());
        auto it = GetDates().find(name);
        if(it == GetDates().end()){
            return nullptr;
        }
        return std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
    }

    static void LoadFromYaml(const YAML::Node &root);
    static ConfigVarBase::ptr LookupBase(const std::string & name);
    static void Visit(std::function<void(ConfigVarBase::ptr)> cb);

private:
    //这个方法是为了在调用s_datas的方法时，s_datas一定先被初始化
    static ConfigVarMap &GetDates() {
        static ConfigVarMap s_datas;
        return s_datas;
    }

    static RWMutexType &GetMutex() {
        static RWMutexType s_mutex;
        return s_mutex;
    }
};



}
#endif













