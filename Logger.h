#ifndef STACK_LOGGER_H
#define STACK_LOGGER_H

//! Defines LEVEL on what program must abort. Must be at least FATAL and be
#ifndef ABORT_LEVEL
#define ABORT_LEVEL 2
#endif
#if ABORT_LEVEL < 1 || ABORT_LEVEL > 4
#error ABORT_LEVEL must be between WARN and FATAL
#endif

//! Defining log level
#ifndef LOG_LEVEL
#define LOG_LEVEL 0
#endif


enum ERROR_LEVEL{
    INFO,
    WARN,
    ERROR,
    FATAL,
};

/*!
 * Inits log system
 */
void stack_logger_init();

/*!
 * Get verbose variant of errorLevel.
 * @param errorLevel
 * @return  string -> verbose variant.
 */
const char* getLevelMsg(ERROR_LEVEL errorLevel);

/*!
 * Finishes log system
 */
void stack_logger_finish();

/*!
 * Dumps stack info to log.
 * @param stack
 */
void stack_dump(const Stack* stack);

/*!
 * Return information about errors.
 * @param [in]  error - error info of that we want to get
 * @param [out] errorLevel - Level of error
 * @param [out] errorMsg   - Error message
 */
void getErrorInfo(STACK_ERROR error, ERROR_LEVEL* errorLevel, const char** errorMsg);

/*!
 * Logs error message. Logs if level >= LOG_LEVEL
 * @param level  - error level
 * @param msg - message to log.
 */
void stack_log(const ERROR_LEVEL level, const char* msg);

/*!
 * Dumps stack data to log. !Works if data is correct!.
 * @param stack
 */
void stack_dump_data(const Stack *stack);

#endif //STACK_LOGGER_H
