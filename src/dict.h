#include <map>
#include <any>
#include <string>
#include <iostream>
#include <sstream>

class Dictionary {
public:
    Dictionary() {}
    Dictionary(const std::string& key, const std::any& value) { set(key, value); }
    Dictionary(const std::map<std::string, std::any>& map) : map(map) {}

    void clear() { map.clear(); }

    void set(const std::string& key, const std::any& value) { map[key] = value; }
    std::any get(const std::string& key) { return map[key]; }
    template <typename T> T get(const std::string& key) { return std::any_cast<T>(map[key]); }

    std::any operator[](const std::string& key) { return map[key]; }
    template <typename T> T operator[](const std::string& key) { return std::any_cast<T>(map[key]); }

    std::string to_string(const std::string& indent="") const;

    // data
    std::map<std::string, std::any> map;
};

inline std::ostream& operator<<(std::ostream& out, const Dictionary& dict) { return out << dict.to_string(); }
