#ifndef SAPPHIRE_STRING_TOOLS_H
#define SAPPHIRE_STRING_TOOLS_H

#include <string>
#include <vector>

class StringTools {
public:
    StringTools() = delete;

    static std::vector<std::string> split(const std::string& string, char delimiter = ';');

    static bool is_whitespace(char c);
    static bool is_number(char c);
    static bool is_letter(char c);

    static char to_lower(char c);
    static char to_upper(char c);

    static std::string strip(const std::string& string);

    static std::string to_lower(const std::string& string);
    static std::string to_upper(const std::string& string);

    static std::string replace(const std::string& string, char c_old, char c_new);

    static std::string join_paths(const std::string& lhs, const std::string& rhs);
    static std::string get_folder(const std::string& path);

    static bool compare(const std::string& lhs, const std::string& rhs, bool caseless = true, bool same_size = true);
};

#endif
