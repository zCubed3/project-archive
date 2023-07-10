#ifndef SAPPHIRE_CONFIG_FILE_H
#define SAPPHIRE_CONFIG_FILE_H

#include <string>
#include <vector>

class ConfigFile {
public:
    struct ConfigEntry {
        std::string name;
        std::string string_value;

        std::vector<std::string> get_string_list();
        std::vector<int> get_int_list();
        std::vector<float> get_float_list();
        int try_get_int(int fallback = 0);
        float try_get_float(float fallback = 0.0F);
    };

    struct ConfigSection {
        std::string name;
        std::vector<ConfigEntry> entries;

        void clear();
        void push_entry(const std::string &name, const std::string &string_value);

        std::string try_get_string(const std::string &name, const std::string &fallback = "");
        std::vector<std::string> try_get_string_list(const std::string &name, const std::vector<std::string> &fallback = {});
        std::vector<int> try_get_int_list(const std::string &name, const std::vector<int> &fallback = {});
        std::vector<float> try_get_float_list(const std::string &name, const std::vector<float> &fallback = {});
        int try_get_int(const std::string &name, int fallback = 0);
        float try_get_float(const std::string &name, float fallback = 0.0F);

        void set_string(const std::string &name, const std::string &value);
    };

    ConfigSection global_section;
    std::vector<ConfigSection> sections;

    bool read_from_path(const std::string &path);
    bool read(const std::string &contents);

    bool write_to_path(const std::string &path);

    ConfigSection &get_section(const std::string &name);

    std::string try_get_string(const std::string &name, const std::string &section, const std::string &fallback = "");
    std::vector<std::string> try_get_string_list(const std::string &name, const std::string &section, const std::vector<std::string> &fallback = {});
    std::vector<int> try_get_int_list(const std::string &name, const std::string &section, const std::vector<int> &fallback = {});
    std::vector<float> try_get_float_list(const std::string &name, const std::string &section, const std::vector<float> &fallback = {});
    int try_get_int(const std::string &name, const std::string &section = "", int fallback = 0);
    float try_get_float(const std::string &name, const std::string &section = "", float fallback = 0);

    void set_string(const std::string &name, const std::string &section, const std::string &value);
};


#endif
