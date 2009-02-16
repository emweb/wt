// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef UTILS_H_
#define UTILS_H_

#include <algorithm>
#include <string>
#include <vector>
#include <map>

namespace Wt {

  class WString;

  namespace Utils {

// appends the character to the string if it does not end with it
extern std::string terminate(const std::string& s, char c);

// in-place replace functions
extern void replace(std::string& s, char c, const std::string& r);
extern void replace(std::string& s, const std::string& c, const std::string& r);

// word manipulation (for style class editing)
extern WString eraseWord(const WString& s, const std::string& w);
extern WString addWord(const WString& s, const std::string& w);

// Finds an element in a vector. Returns the first reference to the
// element, or -1 if the element is not found.
template<typename T>
inline int indexOf(const std::vector<T>& v, const T& value)
{
  typename std::vector<T>::const_iterator i
    = std::find(v.begin(), v.end(), value);

  if (i == v.end())
    return -1;
  else
    return i - v.begin();
}

template<typename T>
inline bool erase(std::vector<T>& v, const T& value)
{
  typename std::vector<T>::iterator i = std::find(v.begin(), v.end(), value);

  if (i != v.end()) {
    v.erase(i);
    return true;
  } else
    return false;
}

template<typename T>
inline void insert(std::vector<T>& result, const std::vector<T>& elements)
{
#ifndef WT_JAVA
  result.insert(result.end(), elements.begin(), elements.end());
#endif // WT_JAVA
}

template<typename T>
inline void sort(std::vector<T>& result)
{
#ifndef WT_JAVA
  std::sort(result.begin(), result.end());
#endif // WT_JAVA
}

template <typename K, typename V, typename T>
inline V& access(std::map<K, V>& m, const T& key)
{
  return m[key];
}

  }
}

#endif // UTILS_H_
