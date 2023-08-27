#include "Server.h"
#include "Configuration.h"
#include "WebController.h"
#include "Wt/WServer.h"

#include <boost/thread.hpp>

#include "Android.h"

#include <stdlib.h>

#ifdef ANDROID
#include <jni.h>
#endif

void preventRemoveOfSymbolsDuringLinking() {
}

#ifdef ANDROID
  extern "C" {
    int main(int argc, const char** argv);
    
    void runMainAndCleanup(int argc, const char** argv) {
        main(argc, argv);
        delete[] argv;
    }

    JNIEXPORT
    jint
    JNICALL
    Java_eu_webtoolkit_android_WtAndroid_startwt
    (JNIEnv* env, jobject thiz, jobjectArray strArray)
    {
      unsigned i;

      jsize argsCount = env->GetArrayLength(strArray);
      std::vector<std::string> args;
      args.reserve(argsCount);
      for (i = 0; i < argsCount; i++) {
        jstring jstr = (jstring)env->GetObjectArrayElement(strArray, i);
        const char* rawChars = env->GetStringUTFChars(jstr, 0);
        std::string s = std::string(rawChars);
        env->ReleaseStringUTFChars(jstr, rawChars);
        env->DeleteLocalRef(jstr);

        if (boost::starts_with(s, "-D")) {
          std::string env = s.substr(2);
          size_t index = env.find("=");
          if (index != std::string::npos) {
              std::string key = env.substr(0, index);
              std::string value = env.substr(index + 1);
              if (setenv(key.c_str(), value.c_str(), 1) != 0) {
                std::cerr 
                  << "WtAndroid::startwt setenv() failed on: " 
                  << env 
                  << std::endl;
              }
          } else {
            std::cerr
              << "WtAndroid::startwt invalid environment variable definition: "
              << s
              << std::endl;
          }
        } else {
          args.push_back(s);
        }
      }

      int argc = args.size();
      const char ** argv = new const char*[argc];
      for (i = 0; i < argc; i++) {
        argv[i] = args[i].c_str();
      }

      try {
        if (Wt::WServer::instance())
          return Wt::WServer::instance()->httpPort();
        boost::thread mainThread(&runMainAndCleanup, argc, argv);
        while (true) {
          if (Wt::WServer::instance()) {
            int httpPort = Wt::WServer::instance()->httpPort();
            if (httpPort != 0) {
              Wt::WServer::instance()->controller()->configuration()
                .setSessionTimeout(-1);
              return httpPort;
            }
          }
          boost::this_thread::sleep(boost::posix_time::milliseconds(100));
        }
      } catch (Wt::WServer::Exception& e) {
        std::cerr << e.what() << "\n";
      } catch (std::exception& e) {
        std::cerr << "exception: " << e.what() << "\n";
      } catch (...) {
        std::cerr << "exception: " << "\n";
      }
    }
  }

#endif //ANDROID
