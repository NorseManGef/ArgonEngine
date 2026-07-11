#ifndef ARGON_LOG
#define ARGON_LOG

#include <plog/Log.h>
#include <plog/Initializers/ConsoleInitializer.h>
#include <plog/Initializers/RollingFileInitializer.h>

namespace Argon {
    const char* ARGON_LOG_NAME = "logs/argon.log";
    const unsigned ARGON_LOG_MAX_SIZE = 1048576;
    const unsigned ARGON_LOG_MAX_FILES = 10;
}

namespace plog {

class ArgonFormatter {
public:
    static util::nstring header() {
        return util::nstring();
    }    

    static util::nstring format(const Record& record) {
        util::nostringstream ss;

        tm t;
        util::localtime_s(&t, &record.getTime().time);

        std::string tss = std::to_string(t.tm_hour) + ":" +
                          std::to_string(t.tm_min) + ":" +
                          std::to_string(t.tm_sec);

        auto sev = record.getSeverity();

        if(sev == Severity::none || sev == Severity::info) {
                ss << record.getMessage() << "\n";
                return ss.str();
        }

        ss << tss;
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
