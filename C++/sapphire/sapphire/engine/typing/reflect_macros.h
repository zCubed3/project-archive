#ifndef SAPPHIRE_REFLECT_MACROS_H
#define SAPPHIRE_REFLECT_MACROS_H

#include <cstdlib>

#include <engine/typing/hash_functions.h>

// TODO: Multiple parents?
#define REFLECT_BASE(CLASS_NAME, PREFIX, POSTFIX)                                           \
private:                                                                                    \
    friend class ClassRegistry;                                                             \
                                                                                            \
private:                                                                                    \
    static const char* get_class_name_static() { return #CLASS_NAME; }                      \
    static size_t get_class_hash_static() { return hash_cstr_djb2(#CLASS_NAME); }           \
                                                                                            \
public:                                                                                     \
    PREFIX const char* get_class_name() POSTFIX { return #CLASS_NAME; }                     \
    PREFIX size_t get_class_hash() POSTFIX { return hash_cstr_djb2(#CLASS_NAME); }

#define REFLECT_INHERITANCE_CHECKS(CLASS_NAME)                                              \
public:                                                                                     \
    template<class TParent>                                                                 \
    bool is_child_of() {                                                                    \
        return ClassRegistry::is_child_of<TParent>(get_class_hash());                       \
    }                                                                                       \


#define REFLECT_BASE_CLASS(CLASS_NAME)                                                      \
    REFLECT_BASE(CLASS_NAME, virtual, )                                                     \
                                                                                            \
private:                                                                                    \
    static const char* get_parent_class_name_static() { return nullptr; }                   \
    static size_t get_parent_class_hash_static() { return -1; }                             \
                                                                                            \
public:                                                                                     \
    virtual const char* get_parent_class_name() { return nullptr; }                         \
    virtual size_t get_parent_class_hash() { return -1; }                                   \
                                                                                            \
    REFLECT_INHERITANCE_CHECKS(CLASS_NAME)                                                  \


#define REFLECT_CLASS(CLASS_NAME, PARENT_NAME)                                              \
    REFLECT_BASE(CLASS_NAME, , override)                                                    \
                                                                                            \
private:                                                                                    \
    static const char* get_parent_class_name_static() { return #PARENT_NAME; }              \
    static size_t get_parent_class_hash_static() { return hash_cstr_djb2(#PARENT_NAME); }   \
                                                                                            \
public:                                                                                     \
    const char* get_parent_class_name() override { return #PARENT_NAME; }                   \
    size_t get_parent_class_hash() override { return hash_cstr_djb2(#PARENT_NAME); }        \

#endif
