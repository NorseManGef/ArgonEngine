#ifndef ARGON_LOG
#define ARGON_LOG

#include <plog/Log.h>
#include <plog/Initializers/ConsoleInitializer.h>
#include <plog/Initializers/RollingFileInitializer.h>

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

        auto sev = record.getSeverity();

        if(sev == Severity::none || sev == Severity::info) {
                ss << record.getMessage() << "\n";
                return ss.str();
        }

        ss << t.tm_hour << ":" << t.tm_min << ":" << t.tm_sec;
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
