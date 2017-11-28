#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

#if defined(__linux__)
#include <time.h>
#elif defined(__MACH__)
#include <mach/mach_time.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

uint64_t nanotime() {
#if defined(__linux__)
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * UINT64_C(1000000000) + ts.tv_nsec;
#elif defined(__MACH__)
    static mach_timebase_info_data_t info;
    if (info.denom == 0)
        mach_timebase_info(&info);
    return mach_absolute_time() * info.numer / info.denom;
#elif defined(_WIN32)
    static LARGE_INTEGER frequency;
    if (frequency.QuadPart == 0)
        QueryPerformanceFrequency(&frequency);
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return counter.QuadPart * UINT64_C(1000000000) / frequency.QuadPart;
#endif
}

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "gason.h"

struct Stat {
    size_t numberCount;
    size_t stringCount;
    size_t objectCount;
    size_t arrayCount;
    size_t falseCount;
    size_t trueCount;
    size_t nullCount;
    size_t sourceSize;
    uint64_t parseTime;
    uint64_t updateTime;
    const char *parserName;
};

struct Rapid {
    rapidjson::Document doc;

    bool parse(const std::vector<char> &buffer) {
        doc.Parse(buffer.data());
        return doc.HasParseError();
    }
    const char *strError() {
        return rapidjson::GetParseError_En(doc.GetParseError());
    }
    void update(Stat &stat) {
        genStat(stat, doc);
    }
    static void genStat(Stat &stat, const rapidjson::Value &v) {
        using namespace rapidjson;
        switch (v.GetType()) {
        case kNullType:
            stat.nullCount++;
            break;
        case kFalseType:
            stat.falseCount++;
            break;
        case kTrueType:
            stat.trueCount++;
            break;
        case kObjectType:
            for (Value::ConstMemberIterator m = v.MemberBegin(); m != v.MemberEnd(); ++m) {
                genStat(stat, m->value);
            }
            stat.objectCount++;
            stat.stringCount += (v.MemberEnd() - v.MemberBegin());
            break;
        case kArrayType:
            for (Value::ConstValueIterator i = v.Begin(); i != v.End(); ++i)
                genStat(stat, *i);
            stat.arrayCount++;
            break;
        case kStringType:
            stat.stringCount++;
            break;
        case kNumberType:
            stat.numberCount++;
            break;
        }
    }
    static const char *name() {
        return "rapid normal";
    }
};

struct RapidInsitu : Rapid {
    std::vector<char> source;

    bool parse(const std::vector<char> &buffer) {
        source = buffer;
        doc.ParseInsitu(source.data());
        return doc.HasParseError();
    }
    static const char *name() {
        return "rapid insitu";
    }
};

struct Gason {
    std::vector<char> source;
    JsonAllocator allocator;
    JsonValue value;
    char *endptr;
    int result;

    bool parse(const std::vector<char> &buffer) {
        source = buffer;
        return (result = jsonParse(source.data(), &endptr, &value, allocator)) == JSON_OK;
    }
    const char *strError() {
        return jsonStrError(result);
    }
    void update(Stat &stat) {
        genStat(stat, value);
    }
    static void genStat(Stat &stat, JsonValue v) {
        switch (v.getTag()) {
        case JSON_ARRAY:
            for (auto i : v) {
                genStat(stat, i->value);
            }
            stat.arrayCount++;
            break;
        case JSON_OBJECT:
            for (auto i : v) {
                genStat(stat, i->value);
                stat.stringCount++;
            }
            stat.objectCount++;
            break;
        case JSON_STRING:
            stat.stringCount++;
            break;
        case JSON_NUMBER:
            stat.numberCount++;
            break;
        case JSON_TRUE:
            stat.trueCount++;
            break;
        case JSON_FALSE:
            stat.falseCount++;
            break;
        case JSON_NULL:
            stat.nullCount++;
            break;
        }
    }
    static const char *name() {
        return "gason";
    }
};

template <typename T>
static Stat run(size_t iterations, const std::vector<char> &buffer) {
    Stat stat;
    memset(&stat, 0, sizeof(stat));

    std::vector<T> docs(iterations);

    auto t = nanotime();
    for (auto &i : docs) {
        i.parse(buffer);
    }
    stat.parseTime += nanotime() - t;

    t = nanotime();
    for (auto &i : docs)
        i.update(stat);
    stat.updateTime += nanotime() - t;

    stat.sourceSize = buffer.size() * iterations;
    stat.parserName = T::name();

    return stat;
}

static void print(const Stat &stat) {
    printf("%7zd %7zd %7zd %7zd %7zd %7zd %7zd %7.2f %7.2f %7.2f %7.2f %s\n",
           stat.numberCount,
           stat.stringCount,
           stat.objectCount,
           stat.arrayCount,
           stat.falseCount,
           stat.trueCount,
           stat.nullCount,
           stat.sourceSize / 1048576.0,
           stat.updateTime / 1e6,
           stat.parseTime / 1e6,
           stat.sourceSize / (stat.parseTime / 1e9) / 1048576.0,
           stat.parserName);
}

#if defined(__clang__)
#define COMPILER "Clang " __clang_version__
#elif defined(__GNUC__)
#define COMPILER "GCC " __VERSION__
#endif

#ifndef __x86_64__
#define __x86_64__ 0
#endif

#ifndef NDEBUG
#define NDEBUG 0
#endif

int main(int argc, const char **argv) {
    printf("gason benchmark, %s, x86_64 %d, SIZEOF_POINTER %d, NDEBUG %d\n", COMPILER, __x86_64__, __SIZEOF_POINTER__, NDEBUG);

    printf("%7s %7s %7s %7s %7s %7s %7s %7s %7s %7s %7s\n",
           "Number", "String", "Object", "Array", "False", "True", "Null", "Size", "Update", "Parse", "Speed");

    size_t iterations = 10;
    for (int i = 1; i < argc; ++i) {
        if (!strcmp("-n", argv[i])) {
            iterations = strtol(argv[++i], NULL, 0);
            continue;
        }

        FILE *fp = fopen(argv[i], "r");
        if (!fp) {
            perror(argv[i]);
            exit(EXIT_FAILURE);
        }
        fseek(fp, 0, SEEK_END);
        size_t size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        std::vector<char> buffer;
        buffer.resize(size + 1);
        fread(buffer.data(), 1, size, fp);
        fclose(fp);

        printf("%7c %7c %7c %7c %7c %7c %7c %7c %7c %7c %7c %s, %zd x %zd\n",
               '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', argv[i], size, iterations);
        print(run<Rapid>(iterations, buffer));
        print(run<RapidInsitu>(iterations, buffer));
        print(run<Gason>(iterations, buffer));
    }
    return 0;
}
