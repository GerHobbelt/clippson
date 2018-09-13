#pragma once
#ifndef WTL_CLIPPSON_HPP_
#define WTL_CLIPPSON_HPP_

#include "clipp.h"
#include "json.hpp"

#include <type_traits>
#include <algorithm>
#include <vector>
#include <string>

namespace wtl {

namespace detail {

struct nonempty {
    bool operator()(const std::string& s) const noexcept {return !s.empty();}
};

template <bool Condition>
using enable_if_t = typename std::enable_if<Condition, std::nullptr_t>::type;

template <class T, enable_if_t<std::is_integral<T>{}> = nullptr>
inline clipp::match::integers filter_type() {
    return clipp::match::integers{};
}

template <class T, enable_if_t<std::is_floating_point<T>{}> = nullptr>
inline clipp::match::numbers filter_type() {
    return clipp::match::numbers{};
}

template <class T, enable_if_t<!std::is_integral<T>{} && !std::is_floating_point<T>{}> = nullptr>
inline nonempty filter_type() {
    return nonempty{};
}

template <class T> inline
std::ostream& operator<<(std::ostream& ost, const std::vector<T>& v) {
    ost << "[";
    auto it = v.begin();
    if (it != v.end()) ost << *it;
    for (++it; it != v.end(); ++it) {
        ost << "," << *it;
    }
    ost << "]";
    return ost;
}

template <class T> inline
std::string doc_default(const T& x, const std::string& doc) {
    std::ostringstream oss;
    oss << doc << " (=" << x << ")";
    return oss.str();
}

template <class T>
struct is_vector : std::false_type {};

template <class T>
struct is_vector<std::vector<T>> : std::true_type {};

template <class T, enable_if_t<!is_vector<T>{}> = nullptr> inline
std::function<void(const char*)> set(nlohmann::json& target) {
    return [&target](const char* s){
        target = clipp::detail::make<T>::from(s);
    };
}

template <class T, enable_if_t<is_vector<T>{}> = nullptr> inline
std::function<void(const char*)> set(nlohmann::json& target) {
    return [&target](const char* s){
        target.push_back(clipp::detail::make<typename T::value_type>::from(s));
    };
}

template <class T> inline clipp::parameter
value(const std::string label="arg") {
    return clipp::value(detail::filter_type<T>(), label)
      .repeatable(detail::is_vector<T>{});
}

template <class T> inline clipp::parameter
value(nlohmann::json& target, const std::string label="arg") {
    return value<T>(label).call(detail::set<T>(target));
}

template <> inline clipp::parameter
value<const char*>(nlohmann::json& target, const std::string label) {
    return value<const char*>(label).call(detail::set<std::string>(target));
}

inline std::string lstrip(const std::string& s) {
    return s.substr(s.find_first_not_of('-'));
}

inline size_t length(const std::string& s) {
    return s.size() - s.find_first_not_of('-');
}

inline std::string longest(const std::vector<std::string>& args) {
    auto it = std::max_element(args.begin(), args.end(),
                  [](const std::string& lhs, const std::string& rhs) {
                      return length(lhs) < length(rhs);
                  });
    return lstrip(*it);
}

} // namespace detail

template <class T, detail::enable_if_t<!std::is_same<T, bool>{}> = nullptr>
inline clipp::group
option(std::vector<std::string>&& flags, T* target, const std::string& doc="", const std::string& label="arg") {
    const auto key = detail::longest(flags);
    return (
      (clipp::option("--" + key + "=") & detail::value<T>(label).set(*target)),
      (clipp::option(std::move(flags)) & detail::value<T>(label).set(*target))
        % detail::doc_default(*target, doc)
   );
}

inline clipp::parameter
option(std::vector<std::string>&& flags, bool* target, const std::string& doc=" ") {
    return clipp::option(std::move(flags)).set(*target).doc(doc);
}

template <class T, detail::enable_if_t<!std::is_same<T, bool>{}> = nullptr>
inline clipp::group
option(nlohmann::json& obj, std::vector<std::string>&& flags, const T init, const std::string& doc="", const std::string& label="arg") {
    const auto key = detail::longest(flags);
    auto& target_js = obj[key] = init;
    return (
      (clipp::option("--" + key + "=") & detail::value<T>(target_js, label)),
      (clipp::option(std::move(flags)) & detail::value<T>(target_js, label))
        % detail::doc_default(init, doc)
    );
}

template <class T, detail::enable_if_t<!std::is_same<T, bool>{} && !std::is_same<T, const char>{}> = nullptr>
inline clipp::group
option(nlohmann::json& obj, std::vector<std::string>&& flags, T* target, const std::string& doc="", const std::string& label="arg") {
    const auto key = detail::longest(flags);
    auto& target_js = obj[key] = *target;
    return (
      (clipp::option("--" + key + "=") & detail::value<T>(target_js, label).set(*target)),
      (clipp::option(std::move(flags)) & detail::value<T>(target_js, label).set(*target))
        % detail::doc_default(*target, doc)
    );
}

inline clipp::parameter
option(nlohmann::json& obj, std::vector<std::string>&& flags, const bool init=false, const std::string& doc=" ") {
    const auto key = detail::longest(flags);
    auto& target_js = obj[key] = init;
    return clipp::option(std::move(flags)).call(detail::set<bool>(target_js)).doc(doc);
}

inline clipp::parameter
option(nlohmann::json& obj, std::vector<std::string>&& flags, bool* target, const std::string& doc=" ") {
    const auto key = detail::longest(flags);
    auto& target_js = obj[key] = *target;
    return clipp::option(std::move(flags)).call(detail::set<bool>(target_js)).set(*target).doc(doc);
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
