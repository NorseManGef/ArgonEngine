#ifndef ARGON_LOG
#define ARGON_LOG
#include <filesystem>
#include <plog/Log.h>
#include <plog/Initializers/ConsoleInitializer.h>
#include <plog/Initializers/RollingFileInitializer.h>

#ifndef ARGON_LOG_DIR
#ifndef __APPLE__
#define ARGON_LOG_DIR "logs"
#endif
#ifdef __APPLE__
#define ARGON_LOG_DIR "Library/Application Support/ArgonSoftware/ArgonApp/logs"
#endif
#endif
#ifndef ARGON_LOG_NAME
#define ARGON_LOG_NAME "argon.log"
#endif
#ifndef ARGON_LOG_MAX_SIZE
#define ARGON_LOG_MAX_SIZE 1048576
#endif
#ifndef ARGON_LOG_MAX_FILES
#define ARGON_LOG_MAX_FILES 10
#endif

namespace Argon {
    inline void init_log_directory(const std::string& log_directory = ARGON_LOG_DIR) {
        #ifdef __APPLE__
          std::filesystem::path logdir;
          if(getenv("HOME")!=nullptr)
            logdir = std::filesystem::path(std::getenv("HOME")) / log_directory;
          else
            throw std::runtime_error("HOME not set");
        #endif
        #ifndef __APPLE__
          std::filesystem::path logdir(log_directory);
        #endif
        if(!exists(logdir)){
            std::filesystem::create_directories(logdir);
        }
    }
}

namespace plog {

class ArgonFormatter {
public:
    static util::nstring header() {
        return util::nstring();
    }    

    static util::nstring format(const Record& record) {
        util::nostringstream ss;

        tm time;
        util::localtime_s(&time, &record.getTime().time);

        auto sev = record.getSeverity();

        if(sev == Severity::none || sev == Severity::info) {
                ss << record.getMessage() << "\n";
                return ss.str();
        }

        ss << time.tm_hour << ":" << time.tm_min << ":" << time.tm_sec;
        ss << PLOG_NSTR(" [") << record.getFunc() << PLOG_NSTR("@") << record.getLine() << PLOG_NSTR("] ");

        switch(record.getSeverity()) {
            case Severity::fatal:
                ss << "FATAL: ";
                break;
            case Severity::error:
                ss << "ERROR: ";
                break;
            case Severity::warning:
                ss << "WARNING: ";
                break;
            case Severity::debug:
                ss << "DEBUG: ";
                break;
            case Severity::verbose:
                ss << "VERBOSE: ";
            default:
                break;
        }

        ss << record.getMessage() << "\n";

        return ss.str();
    }
};

}

#endif
