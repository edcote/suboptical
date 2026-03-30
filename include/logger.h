#pragma once

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <array>
#include <source_location>
#include <string>

// NDEBUG is not defined by default.
#ifndef NDEBUG
#define NDEBUG 0
#endif

// Common logging back-end.
inline void _Log(FILE* stream, const char* level,
                 const std::source_location location, const char* format,
                 va_list args) {
#if NDEBUG == 0
  std::array<char, 1024> buffer;
  vsnprintf(buffer.data(), buffer.size(), format, args);
  fprintf(stream, "[%s] %s:%u %s\n", level, location.file_name(),
          location.line(), buffer.data());
#else
  (void)stream;
  (void)level;
  (void)location;
  (void)format;
  (void)args;
#endif
}

// Log a message at the INFO level.
inline void _LogInfo(const std::source_location location, const char* format,
                     ...) {
  va_list args;
  va_start(args, format);
  _Log(stderr, "INFO", location, format, args);
  va_end(args);
}

// Log a message at the ERROR level.
inline void _LogError(const std::source_location location, const char* format,
                      ...) {
  va_list args;
  va_start(args, format);
  _Log(stderr, "ERROR", location, format, args);
  va_end(args);
}

// Log a message at the DEBUG level.
inline void _LogDebug(const std::source_location location, const char* format,
                      ...) {
  va_list args;
  va_start(args, format);
  _Log(stderr, "DEBUG", location, format, args);
  fflush(stderr);
  va_end(args);
}
// Log a message at the FATAL level and exit.
inline void _LogFatal(const std::source_location location, const char* format,
                      ...) {
  va_list args;
  va_start(args, format);
  _Log(stderr, "FATAL", location, format, args);
  va_end(args);
  exit(1);
}

#define LogInfo(format, ...) \
  _LogInfo(std::source_location::current(), format, ##__VA_ARGS__)

#define LogError(format, ...) \
  _LogError(std::source_location::current(), format, ##__VA_ARGS__)

#define LogDebug(format, ...) \
  _LogDebug(std::source_location::current(), format, ##__VA_ARGS__)

#define LogFatal(format, ...) \
  _LogFatal(std::source_location::current(), format, ##__VA_ARGS__)
