find_package(PkgConfig REQUIRED)

include(CheckIncludeFile)
include(CheckSymbolExists)
include(CheckCSourceCompiles)
include(CMakePushCheckState)

macro(check_c_source_compiles_static SOURCE VAR)
  set(saved_CMAKE_TRY_COMPILE_TARGET_TYPE "${CMAKE_TRY_COMPILE_TARGET_TYPE}")
  set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
  check_c_source_compiles("${SOURCE}" ${VAR} ${ARGN})
  set(CMAKE_TRY_COMPILE_TARGET_TYPE "${saved_CMAKE_TRY_COMPILE_TARGET_TYPE}")
endmacro()

macro(FindLibraryAndSONAME _LIB)
  cmake_parse_arguments(_FLAS "" "" "LIBDIRS" ${ARGN})

  string(TOUPPER ${_LIB} _UPPERLNAME)
  string(REGEX REPLACE "\\-" "_" _LNAME "${_UPPERLNAME}")

  find_library(${_LNAME}_LIB ${_LIB} PATHS ${_FLAS_LIBDIRS})

  # FIXME: fail FindLibraryAndSONAME when library is not shared.
  if(${_LNAME}_LIB MATCHES ".*\\${CMAKE_SHARED_LIBRARY_SUFFIX}.*" AND NOT ${_LNAME}_LIB MATCHES ".*\\${CMAKE_STATIC_LIBRARY_SUFFIX}.*")
    set(${_LNAME}_SHARED TRUE)
  else()
    set(${_LNAME}_SHARED FALSE)
  endif()

  if(${_LNAME}_LIB)
    # reduce the library name for shared linking

    get_filename_component(_LIB_REALPATH ${${_LNAME}_LIB} REALPATH)  # resolves symlinks
    get_filename_component(_LIB_LIBDIR ${_LIB_REALPATH} DIRECTORY)
    get_filename_component(_LIB_JUSTNAME ${_LIB_REALPATH} NAME)

    if(APPLE)
      string(REGEX REPLACE "(\\.[0-9]*)\\.[0-9\\.]*dylib$" "\\1.dylib" _LIB_REGEXD "${_LIB_JUSTNAME}")
    else()
      string(REGEX REPLACE "(\\.[0-9]*)\\.[0-9\\.]*$" "\\1" _LIB_REGEXD "${_LIB_JUSTNAME}")
    endif()

    if(NOT EXISTS "${_LIB_LIBDIR}/${_LIB_REGEXD}")
      set(_LIB_REGEXD "${_LIB_JUSTNAME}")
    endif()
    set(${_LNAME}_LIBDIR "${_LIB_LIBDIR}")

    message(STATUS "dynamic lib${_LIB} -> ${_LIB_REGEXD}")
    set(${_LNAME}_LIB_SONAME ${_LIB_REGEXD})
  endif()

  message(DEBUG "DYNLIB OUTPUTVAR: ${_LIB} ... ${_LNAME}_LIB")
  message(DEBUG "DYNLIB ORIGINAL LIB: ${_LIB} ... ${${_LNAME}_LIB}")
  message(DEBUG "DYNLIB REALPATH LIB: ${_LIB} ... ${_LIB_REALPATH}")
  message(DEBUG "DYNLIB JUSTNAME LIB: ${_LIB} ... ${_LIB_JUSTNAME}")
  message(DEBUG "DYNLIB SONAME LIB: ${_LIB} ... ${_LIB_REGEXD}")
endmacro()
 
set(CONSTANT_DEFINES
  HAVE_GCC_ATOMICS=1
  HAVE_FLOAT_H=1
  HAVE_STDARG_H=1
  HAVE_STDDEF_H=1
  HAVE_STDINT_H=1
  HAVE_LIBC=1
  HAVE_ALLOCA_H=1
  HAVE_ICONV_H=1
  HAVE_INTTYPES_H=1
  HAVE_LIMITS_H=1
  HAVE_MALLOC_H=1
  HAVE_MATH_H=1
  HAVE_MEMORY_H=1
  HAVE_SIGNAL_H=1
  HAVE_STDIO_H=1
  HAVE_STDLIB_H=1
  HAVE_STRINGS_H=1
  HAVE_STRING_H=1
  HAVE_SYS_TYPES_H=1
  HAVE_WCHAR_H=1
  HAVE_DLOPEN=1
  HAVE_MALLOC=1
  HAVE_FDATASYNC=1
  HAVE_GETENV=1
  HAVE_GETHOSTNAME=1
  HAVE_SETENV=1
  HAVE_PUTENV=1
  HAVE_UNSETENV=1
  HAVE_ABS=1
  HAVE_BCOPY=1
  HAVE_MEMSET=1
  HAVE_MEMCPY=1
  HAVE_MEMMOVE=1
  HAVE_MEMCMP=1
  HAVE_WCSLEN=1
  HAVE_WCSNLEN=1
  HAVE_WCSLCPY=1
  HAVE_WCSLCAT=1
  HAVE_WCSSTR=1
  HAVE_WCSCMP=1
  HAVE_WCSNCMP=1
  HAVE_WCSTOL=1
  HAVE_STRLEN=1
  HAVE_STRNLEN=1
  HAVE_STRLCPY=1
  HAVE_STRLCAT=1
  HAVE_STRPBRK=1
  HAVE_INDEX=1
  HAVE_RINDEX=1
  HAVE_STRCHR=1
  HAVE_STRRCHR=1
  HAVE_STRSTR=1
  HAVE_STRTOK_R=1
  HAVE_STRTOL=1
  HAVE_STRTOUL=1
  HAVE_STRTOLL=1
  HAVE_STRTOULL=1
  HAVE_STRTOD=1
  HAVE_ATOI=1
  HAVE_ATOF=1
  HAVE_STRCMP=1
  HAVE_STRNCMP=1
  HAVE_VSSCANF=1
  HAVE_VSNPRINTF=1
  HAVE_ACOS=1
  HAVE_ACOSF=1
  HAVE_ASIN=1
  HAVE_ASINF=1
  HAVE_ATAN=1
  HAVE_ATANF=1
  HAVE_ATAN2=1
  HAVE_ATAN2F=1
  HAVE_CEIL=1
  HAVE_CEILF=1
  HAVE_COPYSIGN=1
  HAVE_COPYSIGNF=1
  HAVE_COS=1
  HAVE_COSF=1
  HAVE_EXP=1
  HAVE_EXPF=1
  HAVE_FABS=1
  HAVE_FABSF=1
  HAVE_FLOOR=1
  HAVE_FLOORF=1
  HAVE_FMOD=1
  HAVE_FMODF=1
  HAVE_ISINF=1
  HAVE_ISINFF=1
  HAVE_ISINF_FLOAT_MACRO=1
  HAVE_ISNAN=1
  HAVE_ISNANF=1
  HAVE_ISNAN_FLOAT_MACRO=1
  HAVE_LOG=1
  HAVE_LOGF=1
  HAVE_LOG10=1
  HAVE_LOG10F=1
  HAVE_LROUND=1
  HAVE_LROUNDF=1
  HAVE_MODF=1
  HAVE_MODFF=1
  HAVE_POW=1
  HAVE_POWF=1
  HAVE_ROUND=1
  HAVE_ROUNDF=1
  HAVE_SCALBN=1
  HAVE_SCALBNF=1
  HAVE_SIN=1
  HAVE_SINF=1
  HAVE_SQRT=1
  HAVE_SQRTF=1
  HAVE_TAN=1
  HAVE_TANF=1
  HAVE_TRUNC=1
  HAVE_TRUNCF=1
  HAVE_FOPEN64=1
  HAVE_FSEEKO=1
  HAVE_FSEEKO64=1
  HAVE_MEMFD_CREATE=1
  HAVE_POSIX_FALLOCATE=1
  HAVE_SIGACTION=1
  HAVE_SIGTIMEDWAIT=1
  HAVE_SA_SIGACTION=1
  HAVE_ST_MTIM=1
  HAVE_SETJMP=1
  HAVE_NANOSLEEP=1
  HAVE_GMTIME_R=1
  HAVE_LOCALTIME_R=1
  HAVE_NL_LANGINFO=1
  HAVE_SYSCONF=1
  HAVE_CLOCK_GETTIME=1
  HAVE_GETPAGESIZE=1
  HAVE_ICONV=1
  HAVE_PTHREAD_SETNAME_NP=1
  HAVE_SEM_TIMEDWAIT=1
  HAVE_GETAUXVAL=1
  HAVE_PPOLL=1
  HAVE__EXIT=1
  HAVE_GETRESUID=1
  HAVE_GETRESGID=1
  HAVE_LINUX_INPUT_H=1
  SDL_INPUT_LINUXEV=1
  HAVE_HIDAPI=1
  #HAVE_LIBUSB=1
  #HAVE_LIBUSB_H=1
  #HAVE_HIDAPI_LIBUSB=1
  HAVE_SDL_HIDAPI
  SDL_JOYSTICK_HIDAPI=1
  HAVE_SDL_JOYSTICK=1
  HAVE_HIDAPI_JOYSTICK=1
  SDL_JOYSTICK_LINUX=1
  SDL_JOYSTICK_VIRTUAL=1
  HAVE_VIRTUAL_JOYSTICK=1
  SDL_HAPTIC_LINUX=1
  HAVE_SDL_HAPTIC=1
  SDL_PROCESS_POSIX=1
  HAVE_SDL_PROCESS=1
  SDL_TIME_UNIX=1
  SDL_TIMER_UNIX=1
  HAVE_SDL_TIME=1
  HAVE_SDL_TIMERS=1
  SDL_VIDEO_VULKAN=1
  HAVE_VULKAN=1
  SDL_VIDEO_RENDER_VULKAN=1
  HAVE_RENDER_VULKAN=1
  SDL_GPU_VULKAN=1
  HAVE_SDL_GPU=1
  SDL_VIDEO_RENDER_GPU=1
  HAVE_RENDER_GPU=1
  SDL_POWER_LINUX=1
  SDL_FILESYSTEM_UNIX=1
  SDL_STORAGE_STEAM=1
  SDL_FSOPS_POSIX=1
  HAVE_SDL_FSOPS=1
  _GNU_SOURCE
  SDL_build_config_h_

  DYNAPI_NEEDS_DLOPEN=1
  HAVE_SDL_LOADSO=1
)

set(CONDITIONAL_DEFINES)

set(LIB_LINUX)

check_symbol_exists(dlopen "dlfcn.h" HAVE_DLOPEN_IN_LIBC)
if(NOT HAVE_DLOPEN_IN_LIBC)
  cmake_push_check_state()
  list(APPEND CMAKE_REQUIRED_LIBRARIES dl)
  check_symbol_exists(dlopen "dlfcn.h" HAVE_DLOPEN_IN_LIBDL)
  cmake_pop_check_state()
  if(HAVE_DLOPEN_IN_LIBDL)
    list(APPEND LIB_LINUX
      dl
    )
  endif(HAVE_DLOPEN_IN_LIBDL)
endif(NOT HAVE_DLOPEN_IN_LIBC)
if(HAVE_DLOPEN_IN_LIBC OR HAVE_DLOPEN_IN_LIBDL)
  list(APPEND CONDITIONAL_DEFINES
    HAVE_DLOPEN=1
    SDL_LOADSO_DLOPEN=1
  )
endif()

pkg_check_modules(DBUS IMPORTED_TARGET dbus-1)
if(DBUS_FOUND)
  list(APPEND CONDITIONAL_DEFINES 
    HAVE_DBUS_DBUS_H=1
    HAVE_FCITX=1
    HAVE_DBUS=1
    SDL_DBUS=1
  )
  list(APPEND LIB_LINUX
    PkgConfig::DBUS
  )
endif(DBUS_FOUND)

check_include_file("sys/inotify.h" HAVE_SYS_INOTIFY_H)
check_symbol_exists(inotify_init "sys/inotify.h" HAVE_INOTIFY_INIT)
check_symbol_exists(inotify_init1 "sys/inotify.h" HAVE_INOTIFY_INIT1)

if(HAVE_SYS_INOTIFY_H AND HAVE_INOTIFY_INIT)
  list(APPEND CONDITIONAL_DEFINES
    HAVE_INOTIFY=1
    HAVE_INOTIFY_INIT=1
  )
endif(HAVE_SYS_INOTIFY_H AND HAVE_INOTIFY_INIT)

pkg_check_modules(IBUS IMPORTED_TARGET ibus-1.0)
if(IBUS_FOUND AND HAVE_SYS_INOTIFY_H)
  list(APPEND CONDITIONAL_DEFINES
    HAVE_IBUS_IBUS_H=1
    HAVE_IBUS=1
    SDL_IBUS=1
  )
  list(APPEND LIB_LINUX
    PkgConfig::IBUS
  )
endif(IBUS_FOUND AND HAVE_SYS_INOTIFY_H)

if(DBUS_FOUND OR IBUS_FOUND)
  list(APPEND CONDITIONAL_DEFINES
    SDL_USE_IME=1
  )
endif(DBUS_FOUND OR IBUS_FOUND)

pkg_check_modules(UDEV IMPORTED_TARGET libudev)
if(UDEV_FOUND)
  pkg_check_modules(EVDEV REQUIRED IMPORTED_TARGET libevdev)
  list(APPEND CONDITIONAL_DEFINES
    HAVE_LIBUDEV_H=1
    HAVE_LIBUDEV=1
  )
  list(APPEND LIB_LINUX
    PkgConfig::UDEV
    PkgConfig::EVDEV
  )
  #FindLibraryAndSONAME(udev)
  #if(UDEV_LIB_SONAME)
  #  list(APPEND CONDITIONAL_DEFINES
  #    SDL_UDEV_DYNAMIC="${UDEV_LIB_SONAME}"
  #  )
  #endif(UDEV_LIB_SONAME)
endif(UDEV_FOUND)

pkg_check_modules(PIPEWIRE IMPORTED_TARGET "libpipewire-0.3>=0.3.44")
if(PIPEWIRE_FOUND)
  list(APPEND CONDITIONAL_DEFINES
    HAVE_PIPEWIRE=1
    SDL_AUDIO_DRIVER_PIPEWIRE=1
    SDL_CAMERA_DRIVER_PIPEWIRE=1
    HAVE_SDL_AUDIO=1
  )
  list(APPEND LIB_LINUX
    PkgConfig::PIPEWIRE
  )
endif(PIPEWIRE_FOUND)

pkg_check_modules(PULSE IMPORTED_TARGET libpulse-simple)
if(PULSE_FOUND)
  list(APPEND CONDITIONAL_DEFINES
    HAVE_PULSEAUDIO=1
    SDL_AUDIO_DRIVER_PULSEAUDIO=1
    HAVE_SDL_AUDIO=1
  )
  list(APPEND LIB_LINUX
    PkgConfig::PULSE
  )
endif()

pkg_check_modules(JACK IMPORTED_TARGET jack)
if(JACK_FOUND)
  list(APPEND CONDITIONAL_DEFINES
    HAVE_JACK=1
    SDL_AUDIO_DRIVER_JACK=1
    HAVE_SDL_AUDIO=1
  )
  list(APPEND LIB_LINUX
    PkgConfig::JACK
  )
endif(JACK_FOUND)

pkg_check_modules(WAYLAND IMPORTED_TARGET wayland-client)
if(WAYLAND_FOUND)

  list(APPEND CONDITIONAL_DEFINES
    HAVE_WAYLAND=1
    HAVE_SDL_VIDEO=1
  )
  list(APPEND LIB_LINUX
    PkgConfig::WAYLAND
  )

  pkg_check_modules(WAYLAND_SCANNER IMPORTED_TARGET wayland-scanner)
  if(WAYLAND_SCANNER_FOUND)
    include(linux/cmake/waylandProtocolGenerator.cmake)
  endif(WAYLAND_SCANNER_FOUND)

  pkg_check_modules(WAYLAND_EGL REQUIRED IMPORTED_TARGET wayland-egl)
  if(WAYLAND_EGL_FOUND)
    list(APPEND LIB_LINUX
      PkgConfig::WAYLAND_EGL
    )
  endif(WAYLAND_EGL_FOUND)

  pkg_check_modules(WAYLAND_CURSOR REQUIRED IMPORTED_TARGET wayland-cursor)
  if(WAYLAND_CURSOR_FOUND)
    list(APPEND LIB_LINUX
      PkgConfig::WAYLAND_CURSOR
    )
  endif(WAYLAND_CURSOR_FOUND)

  pkg_check_modules(XKBCOMMON IMPORTED_TARGET xkbcommon)
  if(XKBCOMMON_FOUND)
    if(XKBCOMMON_VERSION MATCHES "^([0-9]+)\\.([0-9]+)\\.([0-9]+)")
      set(SDL_XKBCOMMON_VERSION_MAJOR ${CMAKE_MATCH_1})
      set(SDL_XKBCOMMON_VERSION_MINOR ${CMAKE_MATCH_2})
      set(SDL_XKBCOMMON_VERSION_PATCH ${CMAKE_MATCH_3})
    else()
      message(WARNING "Failed to parse xkbcommon version; defaulting to lowest supported (0.5.0)")
      set(SDL_XKBCOMMON_VERSION_MAJOR 0)
      set(SDL_XKBCOMMON_VERSION_MINOR 5)
      set(SDL_XKBCOMMON_VERSION_PATCH 0)
    endif() 
    list(APPEND LIB_LINUX
      PkgConfig::XKBCOMMON
    )
  endif(XKBCOMMON_FOUND)

  pkg_check_modules(LIBDECOR IMPORTED_TARGET libdecor-0)
  if(LIBDECOR_FOUND)
    if (PC_LIBDECOR_VERSION MATCHES "^([0-9]+)\\.([0-9]+)\\.([0-9]+)")
      set(SDL_LIBDECOR_VERSION_MAJOR ${CMAKE_MATCH_1})
      set(SDL_LIBDECOR_VERSION_MINOR ${CMAKE_MATCH_2})
      set(SDL_LIBDECOR_VERSION_PATCH ${CMAKE_MATCH_3})
    else()
      message(WARNING "Failed to parse libdecor version; defaulting to lowest supported (0.1.0)")
      set(SDL_LIBDECOR_VERSION_MAJOR 0)
      set(SDL_LIBDECOR_VERSION_MINOR 1)
      set(SDL_LIBDECOR_VERSION_PATCH 0)
    endif()
    list(APPEND CONDITIONAL_DEFINES
      HAVE_WAYLAND_LIBDECOR=1
      HAVE_LIBDECOR_H=1
    )
    list(APPEND LIB_LINUX
      PkgConfig::LIBDECOR
    )
  endif(LIBDECOR_FOUND)

  list(APPEND CONDITIONAL_DEFINES
    SDL_VIDEO_DRIVER_WAYLAND=1
  )
endif(WAYLAND_FOUND)

pkg_check_modules(X11 IMPORTED_TARGET x11)
if(X11_FOUND)
  find_path(X11_INCLUDEDIR
    NAMES X11/Xlib.h
    PATHS
      ${X11_INCLUDE_DIRS}
      /usr/pkg/xorg/include
      /usr/X11R6/include
      /usr/X11R7/include
      /usr/local/include/X11
      /usr/include/X11
      /usr/openwin/include
      /usr/openwin/share/include
      /opt/graphics/OpenGL/include
      /opt/X11/include
  )

  find_file(HAVE_XCURSOR_H NAMES "X11/Xcursor/Xcursor.h" HINTS "${X11_INCLUDEDIR}")
  find_file(HAVE_XINPUT2_H NAMES "X11/extensions/XInput2.h" HINTS "${X11_INCLUDEDIR}")
  find_file(HAVE_XRANDR_H NAMES "X11/extensions/Xrandr.h" HINTS "${X11_INCLUDEDIR}")
  find_file(HAVE_XFIXES_H_ NAMES "X11/extensions/Xfixes.h" HINTS "${X11_INCLUDEDIR}")
  find_file(HAVE_XRENDER_H NAMES "X11/extensions/Xrender.h" HINTS "${X11_INCLUDEDIR}")
  find_file(HAVE_XSYNC_H NAMES "X11/extensions/sync.h" HINTS "${X11_INCLUDEDIR}")
  find_file(HAVE_XSS_H NAMES "X11/extensions/scrnsaver.h" HINTS "${X11_INCLUDEDIR}")
  find_file(HAVE_XSHAPE_H NAMES "X11/extensions/shape.h" HINTS "${X11_INCLUDEDIR}")
  find_file(HAVE_XTEST_H NAMES "X11/extensions/XTest.h" HINTS "${X11_INCLUDEDIR}")
  find_file(HAVE_XDBE_H NAMES "X11/extensions/Xdbe.h" HINTS "${X11_INCLUDEDIR}")
  find_file(HAVE_XEXT_H NAMES "X11/extensions/Xext.h" HINTS "${X11_INCLUDEDIR}")

  if(HAVE_XINPUT2_H)
    pkg_check_modules(XI REQUIRED IMPORTED_TARGET xi)
    list(APPEND CONDITIONAL_DEFINES HAVE_XINPUT2_H)
    list(APPEND LIB_LINUX PkgConfig::XI)
  endif()

  if(HAVE_XRANDR_H)
    pkg_check_modules(XRANDR REQUIRED IMPORTED_TARGET xrandr)
    list(APPEND CONDITIONAL_DEFINES HAVE_XRANDR_H)
    list(APPEND LIB_LINUX PkgConfig::XRANDR)
  endif()

  if(HAVE_XFIXES_H_)
    pkg_check_modules(XFIXES REQUIRED IMPORTED_TARGET xfixes)
    list(APPEND CONDITIONAL_DEFINES HAVE_XFIXES_H)
    list(APPEND LIB_LINUX PkgConfig::XFIXES)
  endif()

  if(HAVE_XRENDER_H)
    list(APPEND CONDITIONAL_DEFINES HAVE_XRENDER_H)
  endif()

  if(HAVE_XSYNC_H)
    list(APPEND CONDITIONAL_DEFINES HAVE_XSYNC_H)
  endif()

  if(HAVE_XSS_H)
    list(APPEND CONDITIONAL_DEFINES HAVE_XSS_H)
  endif()

  if(HAVE_XSHAPE_H)
    list(APPEND CONDITIONAL_DEFINES HAVE_XSHAPE_H)
  endif()

  if(HAVE_XTEST_H)
    pkg_check_modules(XTEST REQUIRED IMPORTED_TARGET xtst)
    list(APPEND CONDITIONAL_DEFINES HAVE_XTEST_H)
    list(APPEND LIB_LINUX PkgConfig::XTEST)
  endif()

  if(HAVE_XDBE_H)
    list(APPEND CONDITIONAL_DEFINES HAVE_XDBE_H)
  endif()

  if(HAVE_XEXT_H)
    list(APPEND CONDITIONAL_DEFINES HAVE_XEXT_H)

    pkg_check_modules(XEXT REQUIRED IMPORTED_TARGET xext)
    list(APPEND CONDITIONAL_DEFINES
      HAVE_X11=1
      HAVE_SDL_VIDEO=1
      SDL_VIDEO_DRIVER_X11=1
    )

    list(APPEND LIB_LINUX
      PkgConfig::X11
      PkgConfig::XEXT
    )
  endif(HAVE_XEXT_H)

  check_c_source_compiles_static("
  #include <X11/Xlib.h>
  int main(int argc, char **argv) {
    Display *display;
    XEvent event;
    XGenericEventCookie *cookie = &event.xcookie;
    XNextEvent(display, &event);
    XGetEventData(display, cookie);
    XFreeEventData(display, cookie);
    return 0; }" HAVE_XGENERICEVENT)
  if(HAVE_XGENERICEVENT)
    list(APPEND CONDITIONAL_DEFINES
      SDL_VIDEO_DRIVER_X11_SUPPORTS_GENERIC_EVENTS=1
    )
  endif()

  if(HAVE_XCURSOR_H)
    list(APPEND CONDITIONAL_DEFINES
      HAVE_X11_XCURSOR=1
      SDL_VIDEO_DRIVER_X11_XCURSOR=1
    )
  endif(HAVE_XCURSOR_H)

  if(HAVE_XDBE_H)
    list(APPEND CONDITIONAL_DEFINES
      HAVE_X11_XDBE=1
      SDL_VIDEO_DRIVER_X11_XDBE=1
    )
  endif(HAVE_XDBE_H)

  check_include_file("X11/XKBlib.h" HAS_XKBLIB)

  if(HAS_XKBLIB)
    list(APPEND CONDITIONAL_DEFINES
      SDL_VIDEO_DRIVER_X11_HAS_XKBLIB=1
    )
  endif(HAS_XKBLIB)

  if(HAVE_XINPUT2_H)
    list(APPEND CONDITIONAL_DEFINES
      HAVE_X11_XINPUT=1
      SDL_VIDEO_DRIVER_X11_XINPUT2=1
    )

    check_c_source_compiles("
        #include <X11/Xlib.h>
        #include <X11/Xproto.h>
        #include <X11/extensions/XInput2.h>
        XIScrollClassInfo *s;
        int main(int argc, char **argv) {}" HAVE_XINPUT2_SCROLLINFO)
    if(HAVE_XINPUT2_SCROLLINFO)
      list(APPEND CONDITIONAL_DEFINES
        SDL_VIDEO_DRIVER_X11_XINPUT2_SUPPORTS_SCROLLINFO=1
      )
    endif()

    check_c_source_compiles_static("
        #include <X11/Xlib.h>
        #include <X11/Xproto.h>
        #include <X11/extensions/XInput2.h>
        int event_type = XI_TouchBegin;
        XITouchClassInfo *t;
        Status XIAllowTouchEvents(Display *a,int b,unsigned int c,Window d,int f) {
          return (Status)0;
        }
        int main(int argc, char **argv) { return 0; }" HAVE_XINPUT2_MULTITOUCH)
    if(HAVE_XINPUT2_MULTITOUCH)
      list(APPEND CONDITIONAL_DEFINES
        SDL_VIDEO_DRIVER_X11_XINPUT2_SUPPORTS_MULTITOUCH=1
      )
    endif()

    check_c_source_compiles("
        #include <X11/Xlib.h>
        #include <X11/Xproto.h>
        #include <X11/extensions/XInput2.h>
        int event_type = XI_GesturePinchBegin;
        XIGesturePinchEvent *t;
        Status XIAllowTouchEvents(Display *a,int b,unsigned int c,Window d,int f) {
          return (Status)0;
        }
        int main(int argc, char **argv) { return 0; }" HAVE_XINPUT2_GESTURE)
    if(HAVE_XINPUT2_GESTURE)
      list(APPEND CONDITIONAL_DEFINES
        SDL_VIDEO_DRIVER_X11_XINPUT2_SUPPORTS_GESTURE=1
      )
    endif()
  endif(HAVE_XINPUT2_H)

  if(HAVE_XFIXES_H_ AND HAVE_XINPUT2_H)
    check_c_source_compiles_static("
        #include <X11/Xlib.h>
        #include <X11/Xproto.h>
        #include <X11/extensions/XInput2.h>
        #include <X11/extensions/Xfixes.h>
        BarrierEventID b;
        int main(int argc, char **argv) { return 0; }" HAVE_XFIXES_H)
    if(HAVE_XFIXES_H)
      list(APPEND CONDITIONAL_DEFINES
        HAVE_XFIXES_H
      )
    endif(HAVE_XFIXES_H)

    list(APPEND CONDITIONAL_DEFINES
      SDL_VIDEO_DRIVER_X11_XFIXES=1
      HAVE_X11_XFIXES=1
    )
  endif(HAVE_XFIXES_H_ AND HAVE_XINPUT2_H)

  if(HAVE_XSYNC_H)
    list(APPEND CONDITIONAL_DEFINES
      SDL_VIDEO_DRIVER_X11_XSYNC=1
      HAVE_X11_XSYNC=1
    )
  endif(HAVE_XSYNC_H)

  if(HAVE_XRANDR_H)
    list(APPEND CONDITIONAL_DEFINES
      SDL_VIDEO_DRIVER_X11_XRANDR=1
      HAVE_X11_XRANDR=1
    )
  endif(HAVE_XRANDR_H)

  if(HAVE_XSHAPE_H)
    list(APPEND CONDITIONAL_DEFINES
      SDL_VIDEO_DRIVER_X11_XSHAPE=1
      HAVE_X11_XSHAPE=1
    )
  endif(HAVE_XSHAPE_H)

  if(HAVE_XTEST_H)
    list(APPEND CONDITIONAL_DEFINES
      SDL_VIDEO_DRIVER_X11_XTEST=1
      HAVE_X11_XTEST=1
    )      
  endif(HAVE_XTEST_H)

  foreach(_LIB X11 Xext Xcursor Xi Xfixes Xrandr Xrender Xss Xtst)
    get_filename_component(_libdir "${X11_${_LIB}_LIB}" DIRECTORY)
    FindLibraryAndSONAME("${_LIB}" LIBDIRS ${_libdir})
  endforeach()

  list(APPEND CONDITIONAL_DEFINES
    HAVE_X11_SHARED=1
  )
  if(X11_LIB)
    list(APPEND CONDITIONAL_DEFINES
      SDL_VIDEO_DRIVER_X11_DYNAMIC="${X11_LIB_SONAME}"
    )
  endif(X11_LIB)
  if(XEXT_LIB)
    list(APPEND CONDITIONAL_DEFINES
      SDL_VIDEO_DRIVER_X11_DYNAMIC_XEXT="${XEXT_LIB_SONAME}"
    )
  endif(XEXT_LIB)
  if(XCURSOR_LIB)
    list(APPEND CONDITIONAL_DEFINES
      SDL_VIDEO_DRIVER_X11_DYNAMIC_XCURSOR="${XCURSOR_LIB_SONAME}"
    )
  endif(XCURSOR_LIB)
  if(XI_LIB)
    list(APPEND CONDITIONAL_DEFINES
      SDL_VIDEO_DRIVER_X11_DYNAMIC_XINPUT2="${XI_LIB_SONAME}"
    )
  endif(XI_LIB)
  if(XFIXES_LIB)
    list(APPEND CONDITIONAL_DEFINES
      SDL_VIDEO_DRIVER_X11_DYNAMIC_XFIXES="${XFIXES_LIB_SONAME}"
    )
  endif(XFIXES_LIB)
  if(XRANDR_LIB)
    list(APPEND CONDITIONAL_DEFINES
      SDL_VIDEO_DRIVER_X11_DYNAMIC_XRANDR="${XRANDR_LIB_SONAME}"
    )
  endif(XRANDR_LIB)
  if(XSS_LIB)
    list(APPEND CONDITIONAL_DEFINES
      SDL_VIDEO_DRIVER_X11_DYNAMIC_XSS="${XSS_LIB_SONAME}"
    )
  endif(XSS_LIB)
  if(XTST_LIB)
    list(APPEND CONDITIONAL_DEFINES
      SDL_VIDEO_DRIVER_X11_DYNAMIC_XTEST="${XTST_LIB_SONAME}"
    )
  endif(XTST_LIB)

  list(APPEND CMAKE_REQUIRED_LIBRARIES ${X11_LIB})
endif(X11_FOUND)

if(NOT X11_FOUND AND NOT WAYLAND_FOUND)
  message(FATAL_ERROR "No display backend found")
endif()

check_c_source_compiles("
  #include <fcntl.h>
  int flag = O_CLOEXEC;
  int main(int argc, char **argv) { return 0; }" HAVE_O_CLOEXEC)
if(HAVE_O_CLOEXEC)
  list(APPEND CONDITIONAL_DEFINES
    HAVE_O_CLOEXEC=1
  )
endif(HAVE_O_CLOEXEC)

pkg_check_modules(LIBURING IMPORTED_TARGET liburing-ffi)
if(LIBURING_FOUND)
  find_path(HAVE_LIBURING_H NAMES liburing.h)
  if(HAVE_LIBURING_H)
    list(APPEND CONDITIONAL_DEFINES
      HAVE_LIBURING_LIBURING=1
      HAVE_LIBURING_H=1
      HAVE_LIBURING=1
    ) 
  endif(HAVE_LIBURING_H)
endif(LIBURING_FOUND)

pkg_check_modules(FRIBIDI IMPORTED_TARGET fribidi)
if(FRIBIDI_FOUND)
  list(APPEND CONDITIONAL_FILES
    HAVE_FRIBIDI=1
    HAVE_FRIBIDI_H=1
  )
endif(FRIBIDI_FOUND)

pkg_check_modules(LIBTHAI IMPORTED_TARGET libthai)
if(LIBTHAI_FOUND)
  list(APPEND CONDITIONAL_FILES
    HAVE_LIBTHAI=1
    HAVE_LIBTHAI_H=1
  )
endif(LIBTHAI_FOUND)

check_c_source_compiles("
    #include <linux/kd.h>
    #include <linux/keyboard.h>
    #include <sys/ioctl.h>
    int main(int argc, char **argv) {
        struct kbentry kbe;
        kbe.kb_table = KG_CTRL;
        ioctl(0, KDGKBENT, &kbe);
        return 0;
    }" HAVE_INPUT_KD)
if(HAVE_INPUT_KD)
  list(APPEND CONDITIONAL_DEFINES
    SDL_INPUT_LINUXKD=1
  )
endif(HAVE_INPUT_KD)

cmake_push_check_state()
string(APPEND CMAKE_REQUIRED_FLAGS
  "-D_REENTRANT"
  "-pthread"
)

check_c_source_compiles("
    #include <pthread.h>
    int main(int argc, char** argv) {
      pthread_attr_t type;
      pthread_attr_init(&type);
      return 0;
    }" HAVE_PTHREADS)
if(HAVE_PTHREADS)
  list(APPEND CONDITIONAL_DEFINES
    SDL_THREAD_PTHREAD=1
  )
  add_compile_definitions(argonapp
    PRIVATE
      "-D_REENTRANT"
  )
  list(APPEND LIB_LINUX
    pthread
  )

  check_c_source_compiles("
    #include <pthread.h>
    int main(int argc, char **argv) {
      pthread_mutexattr_t attr;
      pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
      return 0;
    }" HAVE_RECURSIVE_MUTEXES)
  if(HAVE_RECURSIVE_MUTEXES)
    list(APPEND CONDITIONAL_DEFINES
      SDL_THREAD_PTHREAD_RECURSIVE_MUTEX=1
    )
  else()
    check_c_source_compiles("
        #include <pthread.h>
        int main(int argc, char **argv) {
          pthread_mutexattr_t attr;
          pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
          return 0;
        }" HAVE_RECURSIVE_MUTEXES_NP)
    if(HAVE_RECURSIVE_MUTEXES_NP)
      list(APPEND CONDITIONAL_DEFINES
        SDL_THREAD_PTHREAD_RECURSIVE_MUTEX_NP=1
      )
    endif()
  endif()

  if(SDL_PTHREADS_SEM)
    check_c_source_compiles("#include <pthread.h>
                             #include <semaphore.h>
                             int main(int argc, char **argv) { return 0; }" HAVE_PTHREADS_SEM)
    if(HAVE_PTHREADS_SEM)
      check_c_source_compiles("
          #include <pthread.h>
          #include <semaphore.h>
          int main(int argc, char **argv) {
              sem_timedwait(NULL, NULL);
              return 0;
          }" COMPILER_HAS_SEM_TIMEDWAIT)
          list(APPEND CONDITIONAL_DEFINES
            HAVE_SEM_TIMEDWAIT=${COMPILER_HAS_SEM_TIMEDWAIT}
          )
    endif()
  endif()

  check_include_file(pthread.h HAVE_PTHREAD_H)
  check_include_file(pthread_np.h HAVE_PTHREAD_NP_H)
  if (HAVE_PTHREAD_H)
    list(APPEND CONDITIONAL_DEFINES
      HAVE_PTHREAD_H=1
    )
    check_c_source_compiles("
        #include <pthread.h>
        int main(int argc, char **argv) {
          #ifdef __APPLE__
          pthread_setname_np(\"\");
          #else
          pthread_setname_np(pthread_self(),\"\");
          #endif
          return 0;
        }" HAVE_PTHREAD_SETNAME_NP)
    if (HAVE_PTHREAD_NP_H)
      check_symbol_exists(pthread_set_name_np "pthread.h;pthread_np.h" HAVE_PTHREAD_SET_NAME_NP)
      list(APPEND CONDITIONAL_DEFINES
        HAVE_PTHREAD_NP_H=1
        HAVE_PTHREAD_SET_NAME_NP=1
      )
    endif()
  endif(HAVE_PTHREAD_H)
  list(APPEND CONDITIONAL_DEFINES
    HAVE_SDL_THREADS=1
  )
endif(HAVE_PTHREADS)
cmake_pop_check_state() 

cmake_push_check_state()
check_c_source_compiles("
    #include <GL/glx.h>
    int main(int argc, char** argv) { return 0; }" HAVE_OPENGL_GLX)
cmake_pop_check_state()
if(HAVE_OPENGL_GLX AND NOT HAVE_ROCKCHIP)
  list(APPEND CONDITIONAL_DEFINES
    SDL_VIDEO_OPENGL_GLX=1
  )
endif()

pkg_check_modules(EGL IMPORTED_TARGET egl)
if(EGL_FOUND)
  cmake_push_check_state()
  list(APPEND CMAKE_REQUIRED_INCLUDES ${OPENGL_EGL_INCLUDE_DIRS})
  check_c_source_compiles("
      #define EGL_API_FB
      #define MESA_EGL_NO_X11_HEADERS
      #define EGL_NO_X11
      #include <EGL/egl.h>
      #include <EGL/eglext.h>
      int main (int argc, char** argv) { return 0; }" HAVE_OPENGL_EGL)
  cmake_pop_check_state()
  if(HAVE_OPENGL_EGL)
    list(APPEND CONDITIONAL_DEFINES
      SDL_VIDEO_OPENGL_EGL=1
    )
    list(APPEND LIB_LINUX
      PkgConfig::EGL
    )
  endif(HAVE_OPENGL_EGL)
endif(EGL_FOUND)

cmake_push_check_state()
check_c_source_compiles("
    #include <GL/gl.h>
    #include <GL/glext.h>
    int main(int argc, char** argv) { return 0; }" HAVE_OPENGL)
cmake_pop_check_state()
if(HAVE_OPENGL)
  list(APPEND CONDITIONAL_DEFINES
    SDL_VIDEO_OPENGL=1
    SDL_VIDEO_RENDER_OGL=1
  )
endif(HAVE_OPENGL)

cmake_push_check_state()
check_c_source_compiles("
    #include <GLES/gl.h>
    #include <GLES/glext.h>
    int main (int argc, char** argv) { return 0; }" HAVE_OPENGLES_V1)
check_c_source_compiles("
  #include <GLES2/gl2.h>
  #include <GLES2/gl2ext.h>
  int main (int argc, char** argv) { return 0; }" HAVE_OPENGLES_V2)
cmake_pop_check_state()
if(HAVE_OPENGLES_V1)
  list(APPEND CONDITIONAL_DEFINES
    HAVE_OPENGLES=1
    SDL_VIDEO_OPENGL_ES=1
  )
endif(HAVE_OPENGLES_V1)
if(HAVE_OPENGLES_V2)
  list(APPEND CONDITIONAL_DFEINES
    HAVE_OPENGLES=1
    SDL_VIDEO_OPENGL_ES2=1
    SDL_VIDEO_RENDER_OGL_ES2=1
  )
endif(HAVE_OPENGLES_V2)

check_c_source_compiles("
    #include <linux/videodev2.h>
    int main(int argc, char** argv) { return 0; }" HAVE_LINUX_VIDEODEV2_H)
if(HAVE_LINUX_VIDEODEV2_H)
  list(APPEND CONDITIONAL_DEFINES
    SDL_CAMERA_DRIVER_V4L2=1
    HAVE_CAMERA=1
  )
endif(HAVE_LINUX_VIDEODEV2_H)

cmake_push_check_state()
string(APPEND CMAKE_REQUIRED_FLAGS " -mlsx")
check_c_source_compiles("
    #ifndef __loongarch_sx
    #error Assembler CPP flag not enabled
    #endif
    int main(int argc, char **argv) { return 0; }" COMPILER_SUPPORTS_LSX)
check_include_file("lsxintrin.h" HAVE_LSXINTRIN_H)
cmake_pop_check_state()

if(COMPILER_SUPPORTS_LSX AND HAVE_LSXINTRIN_H)
  set_property(SOURCE
      "${SDL3_SOURCE_DIR}/src/video/yuv2rgb/yuv_rgb_lsx.c"
      "${SDL3_SOURCE_DIR}/src/video/SDL_blit_A.c"
      "${SDL3_SOURCE_DIR}/src/video/SDL_fillrect.c"
      APPEND PROPERTY COMPILE_OPTIONS "-mlsx")

  set_property(SOURCE
      "${SDL3_SOURCE_DIR}/src/video/yuv2rgb/yuv_rgb_lsx.c"
      "${SDL3_SOURCE_DIR}/src/video/SDL_blit_A.c"
      "${SDL3_SOURCE_DIR}/src/video/SDL_fillrect.c"
      PROPERTY SKIP_PRECOMPILE_HEADERS 1)
  set(HAVE_LSX TRUE)
  list(APPEND CONDITIONAL_DEFINES
    HAVE_LSX=1
  )
endif()

if(NOT HAVE_LSX)
  list(APPEND CONDITIONAL_DEFINES
    SDL_DISABLE_LSX=1
  )
endif(NOT HAVE_LSX)

cmake_push_check_state()
string(APPEND CMAKE_REQUIRED_FLAGS " -mlasx")
check_c_source_compiles("
    #ifndef __loongarch_asx
    #error Assembler CPP flag not enabled
    #endif
    int main(int argc, char **argv) { return 0; }" COMPILER_SUPPORTS_LASX)
check_include_file("lasxintrin.h" HAVE_LASXINTRIN_H)
cmake_pop_check_state()
if(COMPILER_SUPPORTS_LASX AND HAVE_LASXINTRIN_H)
  set(HAVE_LASX TRUE)
  list(APPEND CONDITIONAL_DEFINES
    HAVE_LASX=1
  )
endif()

if(NOT HAVE_LASX)
  list(APPEND CONDITIONAL_DEFINES
    SDL_DISABLE_LASX=1
  )
endif(NOT HAVE_LASX)

if(NOT HAVE_ARMNEON)
  list(APPEND CONDITIONAL_DEFINES
    SDL_DISABLE_NEON=1
  )
endif(NOT HAVE_ARMNEON)
