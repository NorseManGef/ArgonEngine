#ifndef ARGON_LOG
#define ARGON_LOG
#include <filesystem>
#include <plog/Log.h>
#include <plog/Initializers/ConsoleInitializer.h>
#include <plog/Initializers/RollingFileInitializer.h>

#ifndef ARGON_LOG_DIR
#define ARGON_LOG_DIR "logs"
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
    inline void init_log_directory() {
        std::filesystem::path logdir(ARGON_LOG_DIR);
        if(!exists(logdir)){
            std::filesystem::create_directory(logdir);
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
