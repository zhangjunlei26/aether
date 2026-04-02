#ifndef AETHER_LOG_H
#define AETHER_LOG_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define MAX_CALLBACKS 32

// Statistics
typedef struct {
  size_t trace_count;
  size_t debug_count;
  size_t info_count;
  size_t warn_count;
  size_t error_count;
  size_t fatal_count;
} LogSumarry;

// Log levels
typedef enum {
  LOG_TRACE = 0,
  LOG_DEBUG = 1,
  LOG_INFO = 2,
  LOG_WARN = 3,
  LOG_ERROR = 4,
  LOG_FATAL = 5
} LogLevel;

// LogEvent - defined first since it's used directly
typedef struct {
  LogLevel level;
  va_list ap;
  struct tm* time;
  const char* fmt;
  const char* file;
  const char* func;
  int line;
} LogEvent;

// Forward declaration - tells compiler that CallbackWriter is a type that will
// be defined later
typedef struct CallbackWriter CallbackWriter;

// Function pointer type can now use CallbackWriter* since we've declared the
// type
typedef void (*LogWriterCallback)(LogEvent* ev, CallbackWriter* cb);
typedef void (*logLockCallback)(bool lock, void* out);

// Full definition of CallbackWriter struct now that LogWriterCallback is
// defined
struct CallbackWriter {
  LogWriterCallback fn;
  void* fp;
  LogLevel level;
  bool is_console;
};

typedef struct {
  uint8_t initialized : 1;
  uint8_t console_out : 1;
  uint8_t show_source_location : 1;
  uint8_t show_timestamps : 1;
  uint8_t use_colors : 1;
  uint8_t level : 3;  // 0~7
} LogStatus;

// Log configuration - can use CallbackWriter now that it's fully defined
typedef struct {
  const char* format_string;  // e.g., "[{time}] {level}: {message}"
  // FILE* output_file;
  // FILE* console_file;
  LogStatus status;
  CallbackWriter callbacks[MAX_CALLBACKS];
  LogSumarry sumarry;
} LogConfig;

// Statistics
void log_print_stats();
LogSumarry* log_get_stats();
// Initialize logging system
void log_init(const char* filename, LogLevel level, bool console_out);
void log_shutdown();

// Core logging functions
void log_write(LogLevel level, const char* file, int line, const char* func,
               const char* fmt, ...);

// Configuration
void log_set_level(LogLevel level);
void log_set_colors(int enabled);
void log_set_timestamps(int enabled);
void log_set_format(const char* format);

const char* get_level_name(LogLevel level);
void log_set_lock(logLockCallback fn, void* out);
int log_add_callback(LogWriterCallback fn, void* out, int level,
                     bool is_console);
int log_add_fp(FILE* fp, LogLevel level, bool is_console);

// Convenience macros With source location
#define log_trace(...) \
  log_write(LOG_TRACE, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define log_debug(...) \
  log_write(LOG_DEBUG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define log_info(...) \
  log_write(LOG_INFO, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define log_warn(...) \
  log_write(LOG_WARN, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define log_error(...) \
  log_write(LOG_ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define log_fatal(...) \
  log_write(LOG_FATAL, __FILE__, __LINE__, __func__, __VA_ARGS__)

#endif  // AETHER_LOG_H
