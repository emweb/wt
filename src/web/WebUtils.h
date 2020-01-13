// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_WEB_UTILS_H_
#define WT_WEB_UTILS_H_

#include <algorithm>
#include <cstring>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <sstream>
#include <unordered_set>
#include <vector>
#include <cstdlib>

#include <Wt/WDllDefs.h>

#ifdef _MSC_VER
#include <float.h>
#endif

namespace Wt {

  class WString;
  class EscapeOStream;

#ifndef WT_TARGET_JAVA
  typedef std::vector<float> FloatBuffer;
  typedef std::vector<int> IntBuffer;
#else
#include <java/buffer>
#endif

  namespace Utils {

#ifndef WT_TARGET_JAVA
template<typename Derived, typename Base>
std::unique_ptr<Derived> dynamic_unique_ptr_cast(std::unique_ptr<Base>&& p)
{
  if (Derived *result = dynamic_cast<Derived *>(p.get())) {
    p.release();
    return std::unique_ptr<Derived>(result);
  } else
    return std::unique_ptr<Derived>();
}
#else
template<typename Derived, typename Base>
std::unique_ptr<Derived> dynamic_unique_ptr_cast(std::unique_ptr<Base> p);
#endif

// appends the character to the string if it does not end with it
extern std::string append(const std::string& s, char c);
extern std::string prepend(const std::string& s, char c);

// in-place replace functions
extern std::string& replace(std::string& s, char c, const std::string& r);
extern std::string& replace(std::string& s, const std::string& c,
			    const std::string& r);

// lower case an UTF-8 string
extern std::string lowerCase(const std::string& s);

// sanitize unicode 
extern void sanitizeUnicode(EscapeOStream& sout, const std::string& text);

// word manipulation (for style class editing)
extern std::string WT_API eraseWord(const std::string& s, const std::string& w);
extern std::string addWord(const std::string& s, const std::string& w);

// Fast integer to string in given buffer
extern char *itoa(int value, char *result, int base = 10);
extern char *lltoa(long long value, char *result, int base = 10);

// Fast integer to string in given buffer, zero padded to length
extern char *pad_itoa(int value, int length, char *result);

// Fast (unsafe) comparison of first n characters
inline bool startsWith(const char *a, const char *b, int n) {
  return std::memcmp(a, b, n) == 0;
}

inline int hexToInt(const char* str) {
  return std::strtol(str, nullptr, 16);
}

inline int length(const std::stringstream& s) {
#ifndef WT_CNOR
  return (int)s.rdbuf()->in_avail();
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
    return static_cast<int>(i - v.begin());
}

template<typename T>
inline bool add(std::vector<T>& v, const T& value)
{
  typename std::vector<T>::iterator i = std::find(v.begin(), v.end(), value);

  if (i != v.end())
    return false;
  else {
    v.push_back(value);
    return true;
  }
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
inline std::unique_ptr<T> take(std::vector<std::unique_ptr<T> >& v,
			       const T *value)
{
  for (std::size_t i = 0; i < v.size();) {
    if (v[i].get() == value) {
      auto result = std::move(v[i]);
      v.erase(v.begin() + i);
      return result;
    } else {
      ++i;
    }
  }

  return std::unique_ptr<T>();
}

template<typename K, typename V>
inline const K * keyForUniquePtrValue(const std::map<K, std::unique_ptr<V> >& m, const V *v)
#ifndef WT_TARGET_JAVA
{
  for (auto& i : m)
    if (i.second.get() == v)
      return &i.first;

  return nullptr;
}
#else // WT_TARGET_JAVA
;
#endif

template<typename K, typename V>
void eraseAndNext(std::map<K, V>& m, typename std::map<K, V>::iterator& i)
{
#ifndef WT_TARGET_JAVA
  m.erase(i++);
#endif // WT_TARGET_JAVA
}

template<typename T>
inline void insert(std::vector<T>& result, const std::vector<T>& elements)
{
#ifndef WT_CNOR
  result.insert(result.end(), elements.begin(), elements.end());
#endif // WT_CNOR
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
inline unsigned insertion_point(const std::vector<T>& v, const T& item,
				Compare compare)
{
  return static_cast<unsigned>(
    std::lower_bound(v.begin(), v.end(), item, compare) - v.begin());
}

template <typename T>
inline unsigned lower_bound(const std::vector<T>& v, const T& item)
{
  return static_cast<unsigned>(
    std::lower_bound(v.begin(), v.end(), item) - v.begin());
}

template <typename T>
inline unsigned upper_bound(const std::vector<T>& v, const T& item)
{
  return static_cast<unsigned>(
    std::upper_bound(v.begin(), v.end(), item) - v.begin());
}

template <typename K, typename V, typename T>
inline V& access(std::map<K, V>& m, const T& key)
{
  return m[key];
}

template <typename K, typename V>
inline void insert(std::map<K, V>& m, const K& key, const V& value)
{
#ifndef WT_TARGET_JAVA
  m.insert(std::make_pair(key, value));
#endif // WT_TARGET_JAVA
}

template <typename T>
inline const T& first(const std::set<T>& s)
{
  return *s.begin();
}

template <typename T>
inline const T& first(const std::unordered_set<T>& s)
{
  return *s.begin();
}

template <typename T>
inline const T& last(const std::set<T>& s)
{
  return *s.rbegin();
}

// Fast round and format to string routine, CSS compliant (does not use
// scientific notation)
extern char *round_css_str(double d, int digits, char *buf);

// Fast round and format to string routine, JS compliant
extern char *round_js_str(double d, int digits, char *buf);

// Only for Java target
extern std::string toHexString(int i);

// Replace all occurences of the 'from' char to the 'to' char in 'v'
extern void replaceAll(std::string& v, char from, char to);

extern WT_API std::string urlEncode(const std::string& url,
				    const std::string& allowed);

extern std::string dataUrlDecode(const std::string& url,
				 std::vector<unsigned char> &data);

#ifndef WT_TARGET_JAVA
extern void inplaceUrlDecode(std::string& s);
#endif

extern std::string EncodeHttpHeaderField(const std::string &fieldname,
                                         const Wt::WString &fieldValue);

inline bool isNaN(double d) {
#ifdef _MSC_VER
  // received bug reports that on 64 bit windows, MSVC2005
  // generates wrong code for d != d.
  return _isnan(d) != 0;
#else
  return !(d == d);  
#endif
}

/*
 * These are workarounds for typ mismatches between Java and C++ port:
 * in C++ vector<string>, in Java string[], with respect to FormData
 * parameters
 */
template<typename T> inline bool isEmpty(const T& vector) {
#ifndef WT_TARGET_JAVA
  return vector.empty();
#else
  return false;
#endif // WT_TARGET_JAVA
}

template<typename T> inline int size(const T& vector) {
#ifndef WT_TARGET_JAVA
  return vector.size();
#else
  return 0;
#endif // WT_TARGET_JAVA
}

template<typename Map, typename K, typename V> 
inline void find(const Map& map, const K& key, V& result)
{
#ifndef WT_TARGET_JAVA
  std::pair<typename Map::const_iterator, typename Map::const_iterator> range 
    = map.equal_range(key);
  
  for (typename Map::const_iterator i = range.first; i != range.second; ++i)
    result.push_back(i->second);
#endif
}

#ifdef WT_TARGET_JAVA
extern void stringToDouble(const char *str, char **end, double &value);
#endif

extern std::string readFile(const std::string& fname);

extern WString formatFloat(const WString &format, double value);

#ifndef WT_TARGET_JAVA
inline FloatBuffer createFloatBuffer(int size) {
  FloatBuffer vec;
  vec.reserve(size);
  return vec;
}

inline const unsigned char* toCharPointer(const FloatBuffer& fb) {
  return reinterpret_cast<const unsigned char*>(&(fb[0]));
}

inline IntBuffer createIntBuffer(int size) {
  IntBuffer vec;
  vec.reserve(size);
  return vec;
}
#else
FloatBuffer createFloatBuffer(int size);
unsigned char* toCharPointer(const FloatBuffer& fb);
IntBuffer createIntBuffer(int size);
#endif


#ifndef WT_TARGET_JAVA
template<typename T>
int sizeofFunction(const T &t) { return sizeof(t)/sizeof(*t); }
// template<typename T, int size>
// int sizeofFunction(const T t) { return size/sizeof(*t); }
#else
int sizeofFunction(const double[]);
int sizeofFunction(const float[]);
int sizeofFunction(const int[]);
int sizeofFunction(const std::string[]);
#endif

extern long WT_API stol(const std::string& v);
extern unsigned long WT_API stoul(const std::string& v);
extern long long WT_API stoll(const std::string& v);
extern unsigned long long WT_API stoull(const std::string& v);
extern int WT_API stoi(const std::string& v);
extern double WT_API stod(const std::string& v);
extern float WT_API stof(const std::string& v);

  }
}

#endif // WT_WEB_UTILS_H_
