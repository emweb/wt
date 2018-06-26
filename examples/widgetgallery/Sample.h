#ifndef WT_TARGET_JAVA

#define SAMPLE_BEGIN(name) \
  namespace { \
    std::unique_ptr<Wt::WWidget> name() {

#define SAMPLE_BEGIN2(type, name) \
  namespace { \
    type name() {

#define SAMPLE_END(...) __VA_ARGS__; } }

#else

#ifdef SAMPLE_BEGIN
#undef SAMPLE_BEGIN
#endif

#define SAMPLE_BEGIN(name) \
  namespace { \
    extern std::unique_ptr<Wt::WWidget> name() {

#ifdef SAMPLE_BEGIN2
#undef SAMPLE_BEGIN2
#endif

#define SAMPLE_BEGIN2(type, name) \
  namespace { \
    extern type name() {

#ifdef SAMPLE_END
#undef SAMPLE_END
#endif

#define SAMPLE_END(...) __VA_ARGS__; } }

#endif
