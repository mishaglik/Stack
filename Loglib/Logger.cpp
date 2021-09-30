#include "Logger.h"
#include <stdio.h>
#include <time.h>
#include "stdarg.h"
FILE* LOG_FILE  = stderr;

ErrorLevel LOG_LEVEL = DEBUG;
ErrorLevel ABORT_LEVEL = FATAL;

void logger_set_stderr(){
    if(LOG_FILE != stderr){
        logger_finish();
    }
    LOG_FILE = stderr;
}

//----------------------------------------------------------------------------------------------------------------------

void logger_set_file(const char *filename){
    if(filename == NULL){
        fprintf(stderr, "Failed to init logger. Filename == NULL\n");
        abort();
    }
    LOG_FILE = fopen(filename, "a");
    setbuf(LOG_FILE, NULL);
    if(LOG_FILE == NULL){
        fprintf(stderr, "Failed to init logger. Failed to open log file\n");
        LOG_FILE = stderr;
    }
}

//----------------------------------------------------------------------------------------------------------------------

void logger_finish(){
    if(LOG_FILE != NULL && LOG_FILE != stderr){
        fclose(LOG_FILE);
        LOG_FILE = stderr;
    }
}

//----------------------------------------------------------------------------------------------------------------------

void logger_message(const ErrorLevel lvl, const char *msg){
    if(lvl < LOG_LEVEL)
        return;
    if(LOG_FILE == NULL){
        LOG_FILE = stderr;
    }

    logger_print_header(lvl);
    fprintf(LOG_FILE,"%s\n", msg);
}

//----------------------------------------------------------------------------------------------------------------------

const char *logger_get_level_info(const ErrorLevel lvl){
    switch(lvl){
    case DEBUG:
        return "DEBUG";
    case INFO:
        return "INFO";
    case WARNING:
        return "WARNING";
    case ERROR:
        return "ERROR";
    case FATAL:
        return "FATAL";
    default:
        return "OTHER";
    }
}

//----------------------------------------------------------------------------------------------------------------------

void logger_assert(const char *filename, const int line, const char *error){
    if(LOG_FILE == NULL)
        LOG_FILE = stderr;

    logger_print_header(FATAL);
    fprintf(LOG_FILE, "Assertion %s failed at line %d in file %s", error, line, filename);
    abort();
}

//----------------------------------------------------------------------------------------------------------------------

void logger_print_header(const ErrorLevel lvl){
    if(LOG_FILE == NULL)
        LOG_FILE = stderr;

    time_t curTime = 0;
    curTime = time(NULL);
    char timeStamp[10] = "";
    strftime(timeStamp, 10, "%H:%M:%S", localtime(&curTime));

    fprintf(LOG_FILE, "%s [%s]\t", timeStamp, logger_get_level_info(lvl));
}

//----------------------------------------------------------------------------------------------------------------------

FILE* logger_get_file(){
    return LOG_FILE;
}

//----------------------------------------------------------------------------------------------------------------------

void logger_set_log_level(ErrorLevel lvl){
    LOG_LEVEL = lvl;
}

//----------------------------------------------------------------------------------------------------------------------

void logger_set_abort_level(ErrorLevel lvl){
    ABORT_LEVEL = lvl;
}

//----------------------------------------------------------------------------------------------------------------------

void logger_clear_log_file(){
    if(LOG_FILE != stderr){
        freopen(NULL,"w", LOG_FILE);
        setbuf(LOG_FILE, NULL);
    }
}

void logger_message_f(const int lvl, const char* format, ...){
    if(LOG_FILE == NULL){
        LOG_FILE = stderr;
    }
    va_list args;
    va_start(args, format);

    if(lvl < LOG_LEVEL && !(lvl == NO_CAP && LOG_LEVEL <= DEBUG))
        return;

    if(lvl != NO_CAP){
        logger_print_header((ErrorLevel) lvl);
    }

    vfprintf(LOG_FILE, format, args);
    va_end(args);
}

void logger_raise(ErrorLevel lvl){
    if(lvl >= ABORT_LEVEL){
        logger_finish();
        abort();
    }
}
