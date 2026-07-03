#ifndef ARGON_LOG
#define ARGON_LOG

#include <plog/Log.h>
#include <plog/Initializers/ConsoleInitializer.h>
#include <plog/Initializers/RollingFileInitializer.h>

#define ILS inline Log
#define SILS static ILS

namespace Argon {

#define IF_LOG(severity) IF_PLOG(severity);

class Log {
public:
    using Severity = plog::Severity;
    using OutputStream = plog::OutputStream;

    template<typename T>
    Log& operator<<(const T& value) {
        PLOG_IF(_s, _enabled) << value;

        return *this;
    }

private:
    Log() = default;
    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;

    Log(Severity s, bool enabled = true):
        _s(s),
        _enabled(enabled)
    {}

    Severity _s;
    bool _enabled;

public:
    SILS logn(bool cond = true) {return Log(Severity::none, cond);}
    SILS logf(bool cond = true) {return Log(Severity::fatal, cond);}
    SILS loge(bool cond = true) {return Log(Severity::error, cond);}
    SILS logw(bool cond = true) {return Log(Severity::warning, cond);}
    SILS logi(bool cond = true) {return Log(Severity::info, cond);}
    SILS logd(bool cond = true) {return Log(Severity::debug, cond);}
    SILS logv(bool cond = true) {return Log(Severity::verbose, cond);}

    SILS log_none(bool cond = true) {return Log(Severity::none, cond);}
    SILS log_fatal(bool cond = true) {return Log(Severity::fatal, cond);}
    SILS log_error(bool cond = true) {return Log(Severity::error, cond);}
    SILS log_warning(bool cond = true) {return Log(Severity::warning, cond);}
    SILS log_info(bool cond = true) {return Log(Severity::info, cond);}
    SILS log_debug(bool cond = true) {return Log(Severity::debug, cond);}
    SILS log_verbose(bool cond = true) {return Log(Severity::verbose, cond);}

    static inline void file_init(Severity maxSeverity, 
                                 const char* fileName, 
                                 size_t maxFileSize = 0, 
                                 int maxFiles = 0) {
        plog::init(maxSeverity, fileName, maxFileSize, maxFiles);
    }

    static inline void console_init(Severity maxSeverity,
                                    OutputStream outputStream) {
        plog::init<plog::TxtFormatter>(maxSeverity, outputStream);
    }
};

}
#endif
