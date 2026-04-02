#include "aether_log.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Global logging state
// static struct {
//   LogConfig config;
//   LogSumarry stats;
// }

LogConfig g_log = {.format_string = "[{time}] {level}: {message}",
                   .status =
                       {
                           .initialized = 1,
                           .console_out = 1,
                           .show_source_location = 1,
                           .show_timestamps = 1,
                           .use_colors = 1,
                           .level = LOG_INFO,
                       },
                   .sumarry = {0}};

// ANSI color codes
#ifdef _WIN32
#define LOG_COLOR_TRACE ""
#define LOG_COLOR_DEBUG ""
#define LOG_COLOR_INFO ""
#define LOG_COLOR_WARN ""
#define LOG_COLOR_ERROR ""
#define LOG_COLOR_FATAL ""
#define LOG_COLOR_RESET ""
#else
#define LOG_COLOR_TRACE "\x1b[94m"
#define LOG_COLOR_DEBUG "\x1b[36m"  // Cyan
#define LOG_COLOR_INFO "\x1b[32m"   // Green
#define LOG_COLOR_WARN "\x1b[33m"   // Yellow
#define LOG_COLOR_ERROR "\x1b[31m"  // Red
#define LOG_COLOR_FATAL "\x1b[35m"  // Bold Magenta
#define LOG_COLOR_RESET "\x1b[0m"
#endif

// Get color for log level
static const char* get_level_color(LogLevel level) {
  if (!g_log.status.use_colors) return "";

  switch (level) {
    case LOG_TRACE:
      return LOG_COLOR_TRACE;
    case LOG_DEBUG:
      return LOG_COLOR_DEBUG;
    case LOG_INFO:
      return LOG_COLOR_INFO;
    case LOG_WARN:
      return LOG_COLOR_WARN;
    case LOG_ERROR:
      return LOG_COLOR_ERROR;
    case LOG_FATAL:
      return LOG_COLOR_FATAL;
    default:
      return "";
  }
}

// Get string name for log level
const char* get_level_name(LogLevel level) {
  switch (level) {
    case LOG_TRACE:
      return "TRACE";
    case LOG_DEBUG:
      return "DEBUG";
    case LOG_INFO:
      return "INFO";
    case LOG_WARN:
      return "WARN";
    case LOG_ERROR:
      return "ERROR";
    case LOG_FATAL:
      return "FATAL";
    default:
      return "UNKN ";
  }
}

int log_add_callback(LogWriterCallback fn, void* fp, int level,
                     bool is_console) {
  for (int i = 0; i < MAX_CALLBACKS; i++) {
    if (!g_log.callbacks[i].fn) {
      g_log.callbacks[i] = (CallbackWriter){fn, fp, level, is_console};
      return 0;
    }
  }
  return -1;
}

// background log write
static void log_file_write(LogEvent* ev, CallbackWriter* cb) {
  if (!g_log.status.initialized) {
    log_init(NULL, LOG_INFO, false);
  }
  LogLevel level = (*ev).level;
  if (level < (LogLevel)g_log.status.level) {
    return;
  }
  FILE* out = cb->fp;
  if (NULL == out) return;
  const char* color = get_level_color(level);
  const char* reset = g_log.status.use_colors ? LOG_COLOR_RESET : "";

  // Print timestamp if enabled
  if (g_log.status.show_timestamps) {
    char buf[64];
    buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ev->time)] = '\0';
    fprintf(out, "%s ", buf);
  }

  // Print log level
  if (cb->is_console) {
    fprintf(out, "%s%s%s: ", color, get_level_name(level), reset);
  } else {
    fprintf(out, "%s: ", get_level_name(level));
  }

  // Print source location
  if (NULL != ev->file) {
    fprintf(out, "[%s -> %s: %d] ", ev->file, ev->func, ev->line);
  }

  // Print arguments
  vfprintf(out, ev->fmt, ev->ap);

  fprintf(out, "\n");
  fflush(out);

  // If fatal, abort
  // if (level == LOG_FATAL) {
  //   abort();
  // }
}

//
int log_add_fp(FILE* fp, LogLevel level, bool is_console) {
  return log_add_callback(log_file_write, fp, level, is_console);
}

// Initialize logging
void log_init(const char* filename, LogLevel level, bool console_out) {
  // console output
  g_log.status.console_out = console_out ? 1 : 0;
  if (console_out) {
    // g_log.console_file = stderr;
    log_add_fp(stderr, level, true);
  }
  // level
  g_log.status.level = (u_int8_t)level;

  // log to file
  if (filename) {
    FILE* fp = fopen(filename, "a");
    if (!fp) {
      fprintf(stderr, "[LOG] Failed to open log file: %s\n", filename);
      if (!console_out) log_add_fp(stderr, level, true);
    }
  }
  g_log.status.initialized = 1;
}

void log_shutdown() {
  if (!g_log.status.initialized) {
    return;
  }
  for (int i = 0; i < MAX_CALLBACKS && g_log.callbacks[i].fn; i++) {
    CallbackWriter* cb = &g_log.callbacks[i];
    if (cb->fp && cb->fp != stderr && cb->fp != stdout) {
      fclose(cb->fp);
    }
  }
  g_log.status.initialized = 0;
}

// Core Logging function
void log_write(LogLevel level, const char* file, int line, const char* func,
               const char* fmt, ...) {
  time_t t = time(NULL);
  LogEvent ev = {
      .fmt = fmt,
      .file = file,
      .line = line,
      .func = func,
      .level = level,
      .time = localtime(&t),
  };
  // Update statistics
  switch (level) {
    case LOG_TRACE:
      g_log.sumarry.trace_count++;
      break;
    case LOG_DEBUG:
      g_log.sumarry.debug_count++;
      break;
    case LOG_INFO:
      g_log.sumarry.info_count++;
      break;
    case LOG_WARN:
      g_log.sumarry.warn_count++;
      break;
    case LOG_ERROR:
      g_log.sumarry.error_count++;
      break;
    case LOG_FATAL:
      g_log.sumarry.fatal_count++;
      break;
  }
  for (int i = 0; i < MAX_CALLBACKS && g_log.callbacks[i].fn; i++) {
    CallbackWriter* cb = &g_log.callbacks[i];
    if (level >= cb->level) {
      va_start(ev.ap, fmt);
      cb->fn(&ev, cb);
      va_end(ev.ap);
    }
  }
}

// Configuration functions
void log_set_level(LogLevel level) { g_log.status.level = (u_int8_t)level; }
void log_set_colors(int enabled) { g_log.status.use_colors = enabled; }
void log_set_timestamps(int enabled) { g_log.status.show_timestamps = enabled; }
void log_set_format(const char* format) { g_log.format_string = format; }

// Statistics
LogSumarry* log_get_stats() { return &g_log.sumarry; }

void log_print_stats() {
  fprintf(stderr, "\n========== Logging Statistics ==========\n");
  fprintf(stderr, "DEBUG: %zu\n", g_log.sumarry.debug_count);
  fprintf(stderr, "INFO:  %zu\n", g_log.sumarry.info_count);
  fprintf(stderr, "WARN:  %zu\n", g_log.sumarry.warn_count);
  fprintf(stderr, "ERROR: %zu\n", g_log.sumarry.error_count);
  fprintf(stderr, "FATAL: %zu\n", g_log.sumarry.fatal_count);
  fprintf(stderr, "========================================\n\n");
}
