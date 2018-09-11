#pragma once
#ifndef WTL_CLIPPSON_HPP_
#define WTL_CLIPPSON_HPP_

#include "clipp.h"
#include "json.hpp"

#include <type_traits>
#include <vector>
#include <string>

namespace wtl {

namespace detail {

struct nonempty {
    bool operator()(const std::string& s) {return !s.empty();}
};

template <bool Condition>
using enable_if_t = typename std::enable_if<Condition, std::nullptr_t>::type;

template <class T, enable_if_t<std::is_integral<T>{}> = nullptr>
inline clipp::match::integers filter_type(T) {
    return clipp::match::integers{};
}

template <class T, enable_if_t<std::is_floating_point<T>{}> = nullptr>
inline clipp::match::numbers filter_type(T) {
    return clipp::match::numbers{};
}

template <class T, enable_if_t<!std::is_trivial<T>{}> = nullptr>
inline nonempty filter_type(T) {
    return nonempty{};
}

template <class T, enable_if_t<std::is_pointer<T>{}> = nullptr>
inline nonempty filter_type(T) {
    return nonempty{};
}

template <class T> inline
std::string doc_default(const T& x, const std::string& doc) {
    std::ostringstream oss;
    oss << doc << " (=" << x << ")";
    return oss.str();
}

template <typename T> inline
std::function<void(const char*)> set(nlohmann::json& obj, const std::string& key) {
    auto& item = obj.at(key);
    return [&item](const char* s){item = s;};
}

template <> inline
std::function<void(const char*)> set<bool>(nlohmann::json& obj, const std::string& key) {
    auto& item = obj.at(key);
    return [&](const char*){item = true;};
}

template <> inline
std::function<void(const char*)> set<int>(nlohmann::json& obj, const std::string& key) {
    auto& item = obj.at(key);
    return [&item](const char* s){item = std::stoi(s);};
}

template <> inline
std::function<void(const char*)> set<long>(nlohmann::json& obj, const std::string& key) {
    auto& item = obj.at(key);
    return [&item](const char* s){item = std::stol(s);};
}

template <> inline
std::function<void(const char*)> set<unsigned>(nlohmann::json& obj, const std::string& key) {
    auto& item = obj.at(key);
    return [&item](const char* s){item = static_cast<unsigned>(std::stoul(s));};
}

template <> inline
std::function<void(const char*)> set<unsigned long>(nlohmann::json& obj, const std::string& key) {
    auto& item = obj.at(key);
    return [&item](const char* s){item = std::stoul(s);};
}

template <> inline
std::function<void(const char*)> set<unsigned long long>(nlohmann::json& obj, const std::string& key) {
    auto& item = obj.at(key);
    return [&item](const char* s){item = std::stoull(s);};
}

template <> inline
std::function<void(const char*)> set<double>(nlohmann::json& obj, const std::string& key) {
    auto& item = obj.at(key);
    return [&item](const char* s){item = std::stod(s);};
}

} // namespace detail

template <class T, detail::enable_if_t<!std::is_same<T, bool>{}> = nullptr>
inline clipp::group
option(std::vector<std::string>&& flags, T* target, const std::string& doc="", const std::string& label="arg") {
    return (
      clipp::option(std::move(flags)) &
      clipp::value(detail::filter_type(*target), label, *target)
    ) % detail::doc_default(*target, doc);
}

inline clipp::parameter
option(std::vector<std::string>&& flags, bool* target, const std::string& doc=" ") {
    return clipp::option(std::move(flags)).set(*target).doc(doc);
}

template <class T, detail::enable_if_t<!std::is_same<T, bool>{}> = nullptr>
inline clipp::group
option(nlohmann::json& obj, std::vector<std::string>&& flags, const T init, const std::string& doc="", const std::string& label="arg") {
    const auto key = flags.back();
    obj[key] = init;
    return (
      clipp::option(std::move(flags)) &
      clipp::value(detail::filter_type(init), label)
        .call(detail::set<T>(obj, key))
    ) % detail::doc_default(init, doc);
}

template <class T, detail::enable_if_t<!std::is_same<T, bool>{}> = nullptr>
inline clipp::group
option(nlohmann::json& obj, std::vector<std::string>&& flags, T* target, const std::string& doc="", const std::string& label="arg") {
    const auto key = flags.back();
    obj[key] = *target;
    return (
      clipp::option(std::move(flags)) &
      clipp::value(detail::filter_type(*target), label)
        .call(detail::set<T>(obj, key))
        .set(*target)
    ) % detail::doc_default(*target, doc);
}

inline clipp::parameter
option(nlohmann::json& obj, std::vector<std::string>&& flags, const bool init=false, const std::string& doc=" ") {
    const auto key = flags.back();
    obj[key] = init;
    return clipp::option(std::move(flags)).call(detail::set<bool>(obj, key)).doc(doc);
}

inline clipp::parameter
option(nlohmann::json& obj, std::vector<std::string>&& flags, bool* target, const std::string& doc=" ") {
    const auto key = flags.back();
    obj[key] = *target;
    return clipp::option(std::move(flags)).call(detail::set<bool>(obj, key)).set(*target).doc(doc);
}

inline clipp::doc_formatting doc_format() {
    return clipp::doc_formatting{}
      .first_column(0)
      .doc_column(24)
      .last_column(80)
      .indent_size(2)
      .flag_separator(",")
    ;
}

inline std::ostream& usage(
  std::ostream& ost,
  const clipp::group& cli,
  const std::string& program="PROGRAM",
  clipp::doc_formatting fmt=doc_format()) {
    return ost << clipp::usage_lines(cli, program, fmt) << "\n\n"
               << clipp::documentation(cli, fmt) << "\n";
}

}  // namespace wtl

#endif /* WTL_CLIPPSON_HPP_ */
