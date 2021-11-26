#pragma once
#include <chrono>
#include <map>
#include <string>
#include <string_view>

struct binary_log
{
  std::FILE* m_index_file;
  std::FILE* m_log_file;

  // Format string table
  std::map<std::string_view, std::size_t> m_format_string_table;
  std::size_t m_format_string_index {0};

  enum class fmt_arg_type : uint8_t
  {
    type_size_t,
    type_char,
    type_int,
    type_float,
    type_double
  };

  constexpr static inline uint8_t string_length(const char* str)
  {
    return *str ? 1 + string_length(str + 1) : 0;
  }

  binary_log(std::string_view path)
  {
    // Create the log file
    // All the log contents go here
    m_log_file = fopen(path.data(), "wb");
    if (m_log_file == nullptr) {
      throw std::invalid_argument("fopen failed");
    }

    // Create the index file
    std::string index_file_path = std::string {path} + ".index";
    m_index_file = fopen(index_file_path.c_str(), "wb");
    if (m_index_file == nullptr) {
      throw std::invalid_argument("fopen failed");
    }
  }

  enum class level
  {
    debug,
    info,
    warn,
    error,
    fatal
  };
};

#define BINARY_LOG(logger, log_level, format_string, ...) \
  [&logger]<typename... Args>(Args && ... args) \
  { \
    if (logger.m_format_string_table.find(format_string) \
        == logger.m_format_string_table.end()) \
    { \
      logger.m_format_string_table[format_string] = \
          logger.m_format_string_index++; \
\
      constexpr uint8_t log_level_byte = static_cast<uint8_t>(log_level); \
      fwrite(&log_level_byte, 1, 1, logger.m_index_file); \
\
      constexpr uint8_t format_string_length = \
          binary_log::string_length(format_string); \
      fwrite(&format_string_length, 1, 1, logger.m_index_file); \
\
      fwrite(format_string, 1, format_string_length, logger.m_index_file); \
\
      constexpr uint8_t num_args = sizeof...(args); \
      fwrite(&num_args, 1, 1, logger.m_index_file); \
    } \
  } \
  (__VA_ARGS__);

#define LOG_DEBUG(logger, format_string, ...) \
  BINARY_LOG(logger, binary_log::level::debug, format_string, __VA_ARGS__)

#define LOG_INFO(logger, format_string, ...) \
  BINARY_LOG(logger, binary_log::level::info, format_string, __VA_ARGS__)

#define LOG_WARN(logger, format_string, ...) \
  BINARY_LOG(logger, binary_log::level::warn, format_string, __VA_ARGS__)

#define LOG_ERROR(logger, format_string, ...) \
  BINARY_LOG(logger, binary_log::level::error, format_string, __VA_ARGS__)

#define LOG_FATAL(logger, format_string, ...) \
  BINARY_LOG(logger, binary_log::level::fatal, format_string, __VA_ARGS__)
