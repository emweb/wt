// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef UTILS_H_
#define UTILS_H_

#include <algorithm>
#include <cstring>
#include <map>
#include <set>
#include <string>
#include <sstream>
#include <vector>

namespace Wt {

  class WString;

  namespace Utils {

// appends the character to the string if it does not end with it
extern std::string terminate(const std::string& s, char c);

// in-place replace functions
extern std::string& replace(std::string& s, char c, const std::string& r);
extern std::string& replace(std::string& s, const std::string& c,
			    const std::string& r);

extern const std::string& escapeText(std::string& s, bool newLinesToo);

// word manipulation (for style class editing)
extern std::string eraseWord(const std::string& s, const std::string& w);
extern std::string addWord(const std::string& s, const std::string& w);

// Fast integer to string in given buffer
extern char *itoa(int value, char *result, int base = 10);

// Fast integer to string in given buffer, zero padded to length
extern char *pad_itoa(int value, int length, char *result);

// Fast (unsafe) comparison of first n characters
inline bool startsWith(const char *a, const char *b, int n) {
  return std::memcmp(a, b, n) == 0;
}

inline int length(const std::stringstream& s) {
#ifndef WT_CNOR
  return s.rdbuf()->in_avail();
#else
  return 0;
#endif
}

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
  result.insert(result.end(), elements.begin(), elements.end());
}

template<typename T>
inline void sort(std::vector<T>& result)
{
  std::sort(result.begin(), result.end());
}

template<typename T, typename Compare>
inline void sort(std::vector<T>& result, const Compare& compare)
{
  std::sort(result.begin(), result.end(), compare);
}

template<typename T>
inline void stable_sort(std::vector<T>& result)
{
  std::stable_sort(result.begin(), result.end());
}

template<typename T, typename Compare>
inline void stable_sort(std::vector<T>& result, const Compare& compare)
{
  std::stable_sort(result.begin(), result.end(), compare);
}

template <typename T, typename Compare>
inline int insertion_point(const std::vector<T>& v, const T& item,
			   Compare compare)
{
  return std::lower_bound(v.begin(), v.end(), item, compare) - v.begin();
}

template <typename K, typename V, typename T>
inline V& access(std::map<K, V>& m, const T& key)
{
  return m[key];
}

template <typename T>
inline const T& first(const std::set<T>& s)
{
  return *s.begin();
}

template <typename T>
inline const T& last(const std::set<T>& s)
{
  return *s.rbegin();
}

// Fast round and format to string routine
extern char *round_str(double d, int digits, char *buf);

// Only for Java target
extern std::string toHexString(int i);

// Split a string in a vector of strings, on every given token
extern std::vector<std::string> tokenizer(const std::string &in,
					  const std::string &sep);

// Replace all occurences of the 'from' char to the 'to' char in 'v'
extern void replaceAll(std::string& v, char from, char to);

extern void unescapeHexTokens(std::string& v);

extern void urlDecode(std::string &s);

extern std::string urlEncode(const std::string& url);

inline bool isNaN(double d) {
  return !(d == d);  
}
  }
}

#endif // UTILS_H_
