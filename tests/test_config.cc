#include "../webserver/fileconfig.h"
#include "../webserver/log.h"
#include <yaml-cpp/yaml.h>


//server_name::ConfigVar<int>::ptr g_int_value_config =
//    server_name::Config::Lookup("system.port", (int)8080, "system port");

//server_name::ConfigVar<float>::ptr g_float_test_value_config =
//    server_name::Config::Lookup("system.port", (float)8080, "system port");

//server_name::ConfigVar<float>::ptr g_float_value_config =
//    server_name::Config::Lookup("system.value", (float)18.88f, "system port");
//
//server_name::ConfigVar<std::vector<int> >::ptr g_int_vector_config =
//    server_name::Config::Lookup("system.int_vec", std::vector<int>{1, 2}, "system int_vec");
//
//server_name::ConfigVar<std::list<int> >::ptr g_int_list_config =
//    server_name::Config::Lookup("system.int_list", std::list<int>{1, 2}, "system int_list");
//
//server_name::ConfigVar<std::set<int> >::ptr g_int_set_config =
//    server_name::Config::Lookup("system.int_set", std::set<int>{1, 2}, "system int_set");
//
//server_name::ConfigVar<std::unordered_set<int> >::ptr g_int_unordered_set_config =
//    server_name::Config::Lookup("system.int_unordered_set", std::unordered_set<int>{1, 2}, "system int_unordered_set");
//
//server_name::ConfigVar<std::map<std::string, int> >::ptr g_str_int_map_config =
//    server_name::Config::Lookup("system.str_int_mp", std::map<std::string, int>{{"k", 2}}, "system str int map");
//
//server_name::ConfigVar<std::unordered_map<std::string, int> >::ptr g_str_int_unordered_map_config =
//    server_name::Config::Lookup("system.str_int_unordered_mp", std::unordered_map<std::string, int>{{"k", 2}}, "system str int unordered_map");
#if 0
void print_yaml(const YAML::Node node, int level) {
    if(node.IsScalar()) {
        WEBSERVER_LOG_INFO(WEBSERVER_LOG_ROOT()) << std::string(level * 4, ' ')
            << node.Scalar() << " - " << node.Type() << " - " << level;
    } else if(node.IsNull()) {
        WEBSERVER_LOG_INFO(WEBSERVER_LOG_ROOT()) << std::string(level * 4, ' ')
            << "NULL - " << node.Type() << " - " << level;
    } else if(node.IsMap()) {
        for(auto it = node.begin();
                it != node.end(); ++it){
            WEBSERVER_LOG_INFO(WEBSERVER_LOG_ROOT()) << std::string(level * 4, ' ')
                << it->first << " - " << it->second.Type() << " - " << level;
            print_yaml(it->second, level + 1);
        }
    } else if(node.IsSequence()) {
        for(size_t i = 0; i < node.size(); ++i) {
            WEBSERVER_LOG_INFO(WEBSERVER_LOG_ROOT()) << std::string(level * 4, ' ')
                << i << " - " << node[i].Type() << " - " << level;
            print_yaml(node[i], level + 1);
        }
    }
}

void test_yaml() {
    YAML::Node root = YAML::LoadFile("/home/marulong/WEBSERVER/bin/conf/log.yml");

    print_yaml(root, 0);
    //WEBSERVER_LOG_INFO(WEBSERVER_LOG_ROOT()) << root;
}

void test_config() {
    WEBSERVER_LOG_INFO(WEBSERVER_LOG_ROOT()) << "before: " << g_int_value_config->getValue();
    WEBSERVER_LOG_INFO(WEBSERVER_LOG_ROOT()) << "before: " << g_float_value_config->toString();
//    WEBSERVER_LOG_INFO(WEBSERVER_LOG_ROOT()) << "before: " << g_float_test_value_config->toString();


#define XX(g_var, name, prefix) \
    { \
        auto v = g_var->getValue(); \
        for(auto &i : v){ \
            WEBSERVER_LOG_INFO(WEBSERVER_LOG_ROOT()) << #prefix " " #name": " << i; \
        } \
    }

#define XX_Map(g_var, name, prefix) \
    { \
        auto v = g_var->getValue(); \
        for(auto &i : v){ \
            WEBSERVER_LOG_INFO(WEBSERVER_LOG_ROOT()) << #prefix " " #name": " << "{" \
            << i.first << " - " << i.second << "}"; \
        } \
    }

    XX(g_int_vector_config, before, int_vec);
    XX(g_int_list_config, before, int_list);
    XX(g_int_set_config, before, int_set);
    XX(g_int_unordered_set_config, before, int_unordered_set);
    XX_Map(g_str_int_map_config, before, g_str_int_map)
    XX_Map(g_str_int_unordered_map_config, before, g_str_int_unordered_map)

    YAML::Node root = YAML::LoadFile("/home/marulong/WEBSERVER/bin/conf/log.yml");
    server_name::Config::LoadFromYaml(root);

    XX(g_int_vector_config, after, int_vec);
    XX(g_int_list_config, after, int_list);
    XX(g_int_set_config, after, int_set);
    XX(g_int_unordered_set_config, after, int_unordered_set);
    XX_Map(g_str_int_map_config, after, g_str_int_map);
    XX_Map(g_str_int_unordered_map_config, after, g_str_int_unordered_map);


    WEBSERVER_LOG_INFO(WEBSERVER_LOG_ROOT()) << "after: " << g_int_value_config->getValue();
    WEBSERVER_LOG_INFO(WEBSERVER_LOG_ROOT()) << "after: " << g_float_value_config->toString();

}

class Person {
public:
    std::string m_name;
    int m_age = 0;
    bool m_sex = 0;

    std::string toString() const {
        std::stringstream ss;
        ss << "[Person name = " << m_name
            <<" age = " << m_age
            <<" sex = " << (m_sex == 0 ? std::string("男") : std::string("女"))
            << "]";
        return ss.str();
    }

    bool operator==(const Person &oth) const {
        return m_name == oth.m_name
            && m_age == oth.m_age
            && m_sex == oth.m_sex;
    }
};

namespace server_name {

template<>
class LexicalCast<std::string, Person> {
public:
    Person operator()(const std::string &v) {
        YAML::Node node = YAML::Load(v);
        Person p;
        p.m_name = node["name"].as<std::string>();
        p.m_age = node["age"].as<int>();
        p.m_sex = node["sex"].as<bool>();

        return p;
    }
};

template<>
class LexicalCast<Person, std::string> {
public:
    std::string operator()(const Person &p) {
        YAML::Node node;
        std::stringstream ss;
        node["name"] = p.m_name;
        node["age"] = p.m_age;
        node["sex"] = p.m_sex;
        ss << node;

        return ss.str();
    }
};

}

server_name::ConfigVar<Person>::ptr g_person =
    server_name::Config::Lookup("class.person", Person(), "system person");

server_name::ConfigVar<std::map<std::string, Person> >::ptr g_person_map =
    server_name::Config::Lookup("class.map", std::map<std::string, Person>(), "system map person");

server_name::ConfigVar<std::map<std::string, std::vector<Person> > >::ptr g_person_vec_map =
    server_name::Config::Lookup("class.vec_map", std::map<std::string, std::vector<Person> >(), "system vec_map person");

void test_class() {

#define XX_PM(g_var, prefix) \
    { \
        auto m = g_person_map->getValue(); \
        for(auto &i : m){ \
            WEBSERVER_LOG_INFO(WEBSERVER_LOG_ROOT()) << #prefix << \
                i.first << " - " << i.second.toString(); \
        } \
        WEBSERVER_LOG_INFO(WEBSERVER_LOG_ROOT()) << #prefix << ": size=" << m.size(); \
    }

#define XX_VEC_MP(g_var, prefix) \
    { \
        auto m = g_person_map->getValue(); \
        for(auto &i : m){ \
            WEBSERVER_LOG_INFO(WEBSERVER_LOG_ROOT()) << #prefix << \
                i.first << " - " << i.second.toString(); \
        } \
        WEBSERVER_LOG_INFO(WEBSERVER_LOG_ROOT()) << #prefix << ": size=" << m.size(); \
    }

    g_person->addListener([](const Person &old_value, const Person &new_value){
            WEBSERVER_LOG_INFO(WEBSERVER_LOG_ROOT()) << " old_value=" << old_value.toString()
             << " new_value=" << new_value.toString();
    });

    WEBSERVER_LOG_INFO(WEBSERVER_LOG_ROOT()) << " before " <<
        g_person->getValue().toString() << " - " << g_person->toString();
    XX_PM(g_person_map, "class.mp before");
    WEBSERVER_LOG_INFO(WEBSERVER_LOG_ROOT()) << " before " <<
        g_person_vec_map->toString();

    YAML::Node root = YAML::LoadFile("/home/marulong/webserver/bin/conf/test.yml");
    server_name::Config::LoadFromYaml(root);
    WEBSERVER_LOG_INFO(WEBSERVER_LOG_ROOT()) << "after" <<
        g_person->getValue().toString() << " - " << g_person->toString();
    XX_PM(g_person_map, "class.mp after");

    auto mp_vec = g_person_vec_map->getValue();
    for(auto &i : mp_vec){
        for(auto &s : i.second){
            WEBSERVER_LOG_INFO(WEBSERVER_LOG_ROOT()) << " after map_vec" <<
                i.first << " - " << s.toString();
        }
    }
}

#endif
void test_log() {
    static server_name::Logger::ptr system_log = WEBSERVER_LOG_NAME("root");
    WEBSERVER_LOG_INFO(system_log) << "hello system" << std::endl;
    std::cout << server_name::LoggerMgr::GetInstance()->toYamlString() << std::endl;
    YAML::Node root = YAML::LoadFile("/home/marulong/webserver/bin/conf/log.yml");
    server_name::Config::LoadFromYaml(root);
    std::cout << "=======================" << std::endl;
    std::cout << server_name::LoggerMgr::GetInstance()->toYamlString() << std::endl;
    std::cout << "=======================" << std::endl;
    std::cout << root << std::endl;
    WEBSERVER_LOG_INFO(system_log) << "hello system" << std::endl;

    system_log->setFormatter("%d - %m%n");
    WEBSERVER_LOG_INFO(system_log) << "hello system" << std::endl;

    server_name::Config::Visit([](const server_name::ConfigVarBase::ptr var) {
            WEBSERVER_LOG_INFO(system_log) << " name= " << var->getName()
                                     << " description= " << var->getDescription()
                                     << " typename= " << var->getTypeName()
                                     << " value= " << var->toString();
            });
}

int main(int argc, char **argv) {
    //WEBSERVER_LOG_INFO(WEBSERVER_LOG_ROOT()) << g_int_value_config->toString();
    //WEBSERVER_LOG_INFO(WEBSERVER_LOG_ROOT()) << g_float_value_config->toString();

    //test_yaml();
//    test_config();
    //test_class();
    test_log();

    return 0;
}
