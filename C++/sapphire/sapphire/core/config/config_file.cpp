#include "config_file.h"

#include <core/data/string_tools.h>

#include <fstream>
#include <sstream>

std::vector<std::string> ConfigFile::ConfigEntry::get_string_list() {
    return StringTools::split(string_value);
}

std::vector<int> ConfigFile::ConfigEntry::get_int_list() {
    std::string buffer;
    std::vector<int> values{};

    for (char c: string_value) {
        if (c != ';' && c != '.' && c != '-') {
            buffer += c;
        } else {
            values.push_back(atoi(buffer.c_str()));
            buffer.clear();
        }
    }

    if (!buffer.empty()) {
        values.push_back(atoi(buffer.c_str()));
    }

    return values;
}

std::vector<float> ConfigFile::ConfigEntry::get_float_list() {
    std::string buffer;
    std::vector<float> values{};

    for (char c: string_value) {
        if (c != ';' && c != '.' && c != '-') {
            buffer += c;
        } else {
            values.push_back(static_cast<float>(atof(buffer.c_str())));
            buffer.clear();
        }
    }

    if (!buffer.empty()) {
        values.push_back(static_cast<float>(atof(buffer.c_str())));
    }

    return values;
}

int ConfigFile::ConfigEntry::try_get_int(int fallback) {
    // TODO: Safety could be better
    for (char c: string_value) {
        if (!StringTools::is_number(c) && c != '.' && c != '-') {
            return fallback;
        }
    }

    if (string_value.empty()) {
        return fallback;
    }

    return atoi(string_value.data());
}

float ConfigFile::ConfigEntry::try_get_float(float fallback) {
    // TODO: Safety could be better
    for (char c: string_value) {
        if (!StringTools::is_number(c) && c != '.' && c != '-') {
            return fallback;
        }
    }

    if (string_value.empty()) {
        return fallback;
    }

    return static_cast<float>(atof(string_value.data()));
}

void ConfigFile::ConfigSection::clear() {
    entries.clear();
}

void ConfigFile::ConfigSection::push_entry(const std::string &name, const std::string &string_value) {
    entries.push_back({name, string_value});
}

void ConfigFile::ConfigSection::set_string(const std::string &name, const std::string &value) {
    for (ConfigEntry &entry: entries) {
        if (entry.name == name) {
            entry.string_value = value;
            return;
        }
    }

    push_entry(name, value);
}

std::string ConfigFile::ConfigSection::try_get_string(const std::string &name, const std::string &fallback) {
    for (ConfigEntry &entry: entries) {
        if (entry.name == name) {
            return entry.string_value;
        }
    }

    return fallback;
}

std::vector<std::string> ConfigFile::ConfigSection::try_get_string_list(const std::string &name, const std::vector<std::string> &fallback) {
    for (ConfigEntry &entry: entries) {
        if (entry.name == name) {
            return entry.get_string_list();
        }
    }

    return fallback;
}

std::vector<int> ConfigFile::ConfigSection::try_get_int_list(const std::string &name, const std::vector<int> &fallback) {
    for (ConfigEntry &entry: entries) {
        if (entry.name == name) {
            return entry.get_int_list();
        }
    }

    return fallback;
}

std::vector<float> ConfigFile::ConfigSection::try_get_float_list(const std::string &name, const std::vector<float> &fallback) {
    for (ConfigEntry &entry: entries) {
        if (entry.name == name) {
            return entry.get_float_list();
        }
    }

    return fallback;
}

int ConfigFile::ConfigSection::try_get_int(const std::string &name, int fallback) {
    for (ConfigEntry &entry: entries) {
        if (entry.name == name) {
            return entry.try_get_int(fallback);
        }
    }

    return fallback;
}

float ConfigFile::ConfigSection::try_get_float(const std::string &name, float fallback) {
    for (ConfigEntry &entry: entries) {
        if (entry.name == name) {
            return entry.try_get_float(fallback);
        }
    }

    return fallback;
}

bool ConfigFile::read_from_path(const std::string &path) {
    std::ifstream file(path);

    if (file.is_open()) {
        std::stringstream contents;
        contents << file.rdbuf();
        file.close();

        read(contents.str());
        return true;
    }

    return false;
}

bool ConfigFile::read(const std::string &contents) {
    std::stringstream stream(contents);

    std::string line;

    ConfigSection *top_section = &global_section;

    std::string array_name;
    std::string array;
    bool began_array = false;

    while (std::getline(stream, line)) {
        // If this line begins with a comment we skip it
        size_t begin = 0;
        while (StringTools::is_whitespace(line[begin]) && begin < line.size()) {
            begin++;
        }

        if (begin > line.size()) {
            continue;
        }

        // Is this a comment?
        if (line[begin] == '#') {
            continue;
        }

        // If the line then begins with a [ we assume we're starting a new section
        if (line[begin] == '[') {
            // We then need the line without the beginning and end
            size_t end = begin;
            while (end < line.size()) {
                if (line[end] == ']') {
                    break;
                }

                end++;
            }

            size_t section_begin = begin + 1;
            std::string section_name = line.substr(section_begin, end - section_begin);

            // If the section doesn't exist we push it
            bool found = false;
            for (ConfigSection &section: sections) {
                if (section.name == section_name) {
                    top_section = &section;
                    found = true;
                    break;
                }
            }

            if (!found) {
                ConfigSection section{};
                section.name = section_name;

                sections.push_back(section);
                top_section = &sections.back();
            }
        }

        if (line[begin] == '}' && began_array) {
            began_array = false;
            top_section->set_string(array_name, array);
        }

        // Anything else counts as a variable (as long as there is an equals sign)
        size_t equals = line.find_first_of('=');

        if (equals != std::string::npos) {
            std::string var = line.substr(begin, equals - begin);
            std::string val = line.substr(equals + 1);

            // Remove padding whitespace and quotes
            var = StringTools::strip(var);
            val = StringTools::strip(val);

            // If our value is just a bracket we begin an array
            if (val == "{") {
                began_array = true;

                array_name = var;
                array.clear();
            } else {
                top_section->set_string(var, val);
            }
        }

        if (began_array && equals == std::string::npos) {
            array += StringTools::strip(line) + ";";
        }
    }

    return true;
}

bool ConfigFile::write_to_path(const std::string &path) {
    std::ofstream file(path);

    file << "# Warning, your changes may be overwritten!\n" << std::endl;
    file << "# Global Entries" << std::endl;
    for (ConfigEntry& entry: global_section.entries) {
        file << entry.name << " = " << entry.string_value << std::endl;
    }

    file << "\n# Scoped Entries" << std::endl;
    for (ConfigSection& section: sections) {
        file << "\n[" << section.name << "]" << std::endl;
        for (ConfigEntry &entry: section.entries) {
            file << entry.name << " = " << entry.string_value << std::endl;
        }
    }

    file.close();

    return true;
}

ConfigFile::ConfigSection& ConfigFile::get_section(const std::string &name) {
    for (ConfigSection& section: sections) {
        if (section.name == name) {
            return section;
        }
    }

    return global_section;
}

std::string ConfigFile::try_get_string(const std::string &name, const std::string &section, const std::string &fallback) {
    if (section.empty()) {
        global_section.try_get_string(name, fallback);
    } else {
        for (ConfigSection &test_section: sections) {
            if (test_section.name == section) {
                return test_section.try_get_string(name, fallback);
            }
        }
    }

    return fallback;
}

std::vector<std::string> ConfigFile::try_get_string_list(const std::string &name, const std::string &section, const std::vector<std::string> &fallback) {
    if (section.empty()) {
        global_section.try_get_string_list(name, fallback);
    } else {
        for (ConfigSection &test_section: sections) {
            if (test_section.name == section) {
                return test_section.try_get_string_list(name, fallback);
            }
        }
    }

    return fallback;
}

std::vector<int> ConfigFile::try_get_int_list(const std::string &name, const std::string &section, const std::vector<int> &fallback) {
    if (section.empty()) {
        global_section.try_get_int_list(name, fallback);
    } else {
        for (ConfigSection &test_section: sections) {
            if (test_section.name == section) {
                return test_section.try_get_int_list(name, fallback);
            }
        }
    }

    return fallback;
}

std::vector<float> ConfigFile::try_get_float_list(const std::string &name, const std::string &section, const std::vector<float> &fallback) {
    if (section.empty()) {
        global_section.try_get_float_list(name, fallback);
    } else {
        for (ConfigSection &test_section: sections) {
            if (test_section.name == section) {
                return test_section.try_get_float_list(name, fallback);
            }
        }
    }

    return fallback;
}

int ConfigFile::try_get_int(const std::string &name, const std::string &section, int fallback) {
    if (section.empty()) {
        global_section.try_get_int(name, fallback);
    } else {
        for (ConfigSection &test_section: sections) {
            if (test_section.name == section) {
                return test_section.try_get_int(name, fallback);
            }
        }
    }

    return fallback;
}

float ConfigFile::try_get_float(const std::string &name, const std::string &section, float fallback) {
    if (section.empty()) {
        global_section.try_get_float(name, fallback);
    } else {
        for (ConfigSection &test_section: sections) {
            if (test_section.name == section) {
                return test_section.try_get_float(name, fallback);
            }
        }
    }

    return fallback;
}

void ConfigFile::set_string(const std::string &name, const std::string &section, const std::string &value) {
    ConfigSection& found_section = get_section(section);

    if (found_section.name == global_section.name && !section.empty()) {
        ConfigSection new_section{};
        new_section.name = section;

        new_section.set_string(name, value);
        sections.push_back(new_section);
    } else {
        found_section.set_string(name, value);
    }
}
