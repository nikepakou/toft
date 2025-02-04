// Copyright 2010, The TOFT Authors.
// Author: CHEN Feng <chen3feng@gmail.com>

#ifndef TOFT_BASE_CLASS_REGISTRY_CLASS_REGISTRY_H
#define TOFT_BASE_CLASS_REGISTRY_CLASS_REGISTRY_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <map>
#include <string>
#include <vector>

#include "toft/base/preprocess.h"

namespace toft {

// To be used internally as base of ClassRegistry to reduce code bloating.
class ClassRegistryBase {
protected:
    // GetFunction: 函数指针，指向的函数不需要形参，并返回void*类型
    // void* : 任何非常量对象的地址都能存入void*
    typedef void* (*GetFunction)();

protected:
    ClassRegistryBase() {}
    ~ClassRegistryBase() {}

    void DoAddClass(const std::string& entry_name, GetFunction getter);
    void* DoGetObject(const std::string& entry_name) const;

public:
    size_t ClassCount() const;
    const std::string& ClassName(size_t i) const;

private:
    typedef std::map<std::string, GetFunction> ClassMap;
    ClassMap m_class_map;
    std::vector<std::string> m_class_names;
};

// ClassRegistry manage (name -> creator/singleton) mapping.
// One base class may have multiple registry instance, distinguished by the
// registry_name.
template <typename BaseClassName>
class ClassRegistry : public ClassRegistryBase {
public:
    typedef BaseClassName* (*ObjectGetter)();

public:
    ClassRegistry() {}
    ~ClassRegistry() {}

    // reinterpret_cast 适合于函数指针的转化 《more effective c++》.2
    // 换成static_cast 编译不过 
    void AddClass(const std::string& entry_name, ObjectGetter getter) {
        // 将原类型函数指针对象转换为void*对象
        DoAddClass(entry_name, reinterpret_cast<GetFunction>(getter));
    }

    BaseClassName* CreateObject(const std::string& entry_name) const {
        // 将void*对象转换为原类型指针对象
        return static_cast<BaseClassName*>(DoGetObject(entry_name));
    }
};

// ClassRegistrySingleton manage (name -> creator/singleton) mapping.
// One base class may have multiple registry instance, distinguished by the
// registry_name.
template <typename BaseClassName>
class ClassRegistrySingleton : public ClassRegistryBase {
public:
    typedef BaseClassName* (*ObjectGetter)();

public:
    ClassRegistrySingleton() {}
    ~ClassRegistrySingleton() {}

    void AddClass(const std::string& entry_name, ObjectGetter getter) {
        DoAddClass(entry_name, reinterpret_cast<GetFunction>(getter));
    }

    BaseClassName* GetSingleton(const std::string& entry_name) const {
        return static_cast<BaseClassName*>(DoGetObject(entry_name));
    }
};

// TOFT_CLASS_REGISTRY_DEFINE Make a unique type for a given registry_name.
// This class is the base of the generated unique type
// 为给定的注册表名创建唯一的类型。此类是生成唯一类型的基础
template <typename BaseClassName>
struct ClassRegistryTagBase {
    typedef BaseClassName BaseClass;
    typedef ClassRegistry<BaseClassName> RegistryType;
};

// TOFT_CLASS_REGISTRY_DEFINE Make a unique type for a given registry_name.
// This class is the base of the generated unique type
template <typename BaseClassName>
struct ClassRegistrySingletonTagBase {
    typedef BaseClassName BaseClass;
    typedef ClassRegistrySingleton<BaseClassName> RegistryType;
};

// Get the registry singleton instance for a given registry_name
template <typename RegistryTag>
typename RegistryTag::RegistryType& ClassRegistryInstance() {
    static typename RegistryTag::RegistryType registry;
    return registry;
}

// All class can share the same creator as a function template
// 所有类都可以作为函数模板共享同一个创建者
template <typename BaseClassName, typename SubClassName>
BaseClassName* ClassRegistry_NewObject() {
    return new SubClassName();
}

// Used to register a given class into given registry
// 用于将给定类注册到给定注册表中
template <typename RegistryTag>
class ClassRegisterer {
    typedef typename RegistryTag::BaseClass BaseClassName;
public:
    ClassRegisterer(
        const std::string& entry_name,
        typename ClassRegistry<BaseClassName>::ObjectGetter getter) {
        ClassRegistryInstance<RegistryTag>().AddClass(entry_name, getter);
    }
    ~ClassRegisterer() {}
};

// Different RegistryTag generate different instance
template <typename SubClassName, typename RegistryTag>
typename RegistryTag::BaseClass* ClassRegistry_GetSingleton() {
    static SubClassName singleton;
    return &singleton;
}

} // namespace toft

// Define a registry for a base class.
//
// The first parameter, registry_name, should be unique globally.
// Mulitiple registry can be defined for one base class with different
// registry_name.
//
// This macro should be used in the same namespace as base_class_name.
// 此宏应与base_class_name在同一命名空间中使用
#define TOFT_CLASS_REGISTRY_DEFINE(registry_name, base_class_name) \
    struct registry_name##RegistryTag: \
        public ::toft::ClassRegistryTagBase<base_class_name> {};

#define TOFT_CLASS_REGISTRY_DEFINE_SINGLETON(registry_name, base_class_name) \
    struct registry_name##RegistryTag: \
        public ::toft::ClassRegistrySingletonTagBase<base_class_name> {};

// User could select one of following two versions of
// TOFT_CLASS_REGISTRY_OBJECT_CREATOR, with or without singleton, but couldn't use
// both. So if the user decides to use the singleton version, all
// implementations must have public default constructor.
//
// These macros should be used in the same namespace as class_name, and
// class_name should not be namespace prefixed.
// 这些宏应该与class_name在同一名称空间中使用，并且类名称不应以空间名称为前缀。
//
// But namespace prefix is required for registry_name and base_class_name if
// they are defined in different namespace, for example, ::toft::File
// 但是如果注册表名和基类名是在不同的命名空间中定义的，则它们需要命名空间前缀
//
// 实际上执行的是ClassRegisterer构造函数
// 其中 TOFT_PP_JOIN(g_object_creator_registry_##class_name, __LINE__) 是对象名称
#define TOFT_CLASS_REGISTRY_REGISTER_CLASS(registry_name, \
                                           base_class_name, \
                                           entry_name_as_string, \
                                           class_name) \
    static ::toft::ClassRegisterer<registry_name##RegistryTag> \
        TOFT_PP_JOIN(g_object_creator_registry_##class_name, __LINE__)( \
            entry_name_as_string, \
            &toft::ClassRegistry_NewObject<base_class_name, class_name>)

#define TOFT_CLASS_REGISTRY_REGISTER_CLASS_SINGLETON(registry_name, \
                                                     base_class_name, \
                                                     entry_name_as_string, \
                                                     class_name) \
    static ::toft::ClassRegisterer<registry_name##RegistryTag> \
        TOFT_PP_JOIN(g_object_creator_registry_##class_name, __LINE__)( \
            entry_name_as_string, \
            &::toft::ClassRegistry_GetSingleton<class_name, registry_name##RegistryTag>)

// Create object from registry by name.
// Namespace prefix is required for registry_name if it is defined in different
// namespace
#define TOFT_CLASS_REGISTRY_CREATE_OBJECT(registry_name, entry_name_as_string) \
    ::toft::ClassRegistryInstance<registry_name##RegistryTag>().CreateObject(entry_name_as_string)

// Get object singleton from registry by name.
#define TOFT_CLASS_REGISTRY_GET_SINGLETON(registry_name, entry_name_as_string) \
    ::toft::ClassRegistryInstance<registry_name##RegistryTag>().GetSingleton(entry_name_as_string)

// Obtain the number of classes registered to the registry.
#define TOFT_CLASS_REGISTRY_CLASS_COUNT(registry_name) \
    ::toft::ClassRegistryInstance<registry_name##RegistryTag>().ClassCount()

// Obtain class name by index for the registry.
#define TOFT_CLASS_REGISTRY_CLASS_NAME(registry_name, i) \
    ::toft::ClassRegistryInstance<registry_name##RegistryTag>().ClassName(i)

#endif // TOFT_BASE_CLASS_REGISTRY_CLASS_REGISTRY_H
