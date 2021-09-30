#ifndef ERRNLOG_LOGGER_H
#define ERRNLOG_LOGGER_H
#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"

enum ErrorLevel{
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL,
};
#define NO_CAP -1

#define LOG_DEBUG(x)    logger_message(DEBUG,   (x))
#define LOG_INFO(x)     logger_message(INFO,    (x))
#define LOG_WARNING(x)  logger_message(WARNING, (x))
#define LOG_ERROR(x)    logger_message(ERROR,   (x))
#define LOG_FATAL(x)    logger_message(FATAL,   (x))

#define LOG_MESSAGE(type, x) logger_message((type), (x))
#define LOG_MESSAGE_F(type, x, ...) logger_message_f((type), (x), ##__VA_ARGS__)

#define LOG_RAISE(errorType) logger_raise(errorType)

#define LOG_DEBUG_F(format, ...) logger_print_header(DEBUG); fprintf(logger_get_file(), format, ##__VA_ARGS__)
#define LOG_DEBUG_F2(format, ...) fprintf(logger_get_file(), format, ##__VA_ARGS__)

#define LOG_ASSERT(x)   ((x) ? (void)0 : logger_assert(__FILE__, __LINE__, #x))
#define LOG_ASSERT2(x, func) ((x) ? void(0) : func, logger_assert(__FILE__, __LINE__, #x))

/*!
 * Set logging to stdErr
 */
void logger_set_stderr();

/*!
 * Set logging to file with filename
 * @param filename
 */
void logger_set_file(const char* filename);

/*!
 * Finish logging. Closes file
 */
void logger_finish();

/*!
 * Raises error with level and message (msg)
 * Logs if lvl >= LOG_LEVEL
 * Aborts if lvl >= ABORT_LEVEL
 * @param lvl - level of error
 * @param msg - message to print.
 */
void logger_message(const ErrorLevel lvl, const char *msg);

/*!
 * Same as logger_message_f, but formatted as prinf;
 * @param lvl
 * @param format
 * @param ...
 */
void logger_message_f(const int lvl, const char *format, ...);

/*!
 * Generates assert fail
 * @param filename - name of file failed
 * @param line     - line errored
 * @param error    - error
 */
[[noreturn]]void logger_assert(const char* filename, const int line, const char* error);

/*!
 * Prints head of log.
 * @param lvl
 */
void logger_print_header(const ErrorLevel lvl);

/*!
 * Translates ErrorLevel to const char*
 * @param lvl - level.
 * @return verbose of level
 */
const char* logger_get_level_info(const ErrorLevel lvl);

/*!
 * Returns log file
 * @return log file
 */
FILE* logger_get_file();

/*!
 * sets log level
 * @param lvl
 */
void logger_set_log_level(ErrorLevel lvl);

/*!
 * Sets abort level
 * @param lvl
 */
void logger_set_abort_level(ErrorLevel lvl);

/*!
 * Clears log
 */
void logger_clear_log_file();

/*!
 * Raises error. Aborts if lvl > abort_level
 * @param lvl
 */
void logger_raise(ErrorLevel lvl);
#endif //ERRNLOG_LOGGER_H
