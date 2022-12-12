#ifndef __STACK_TRACE_H__
#define __STACK_TRACE_H__

#include <vector>
#include <string>

struct TraceLog
{
    unsigned long line;
    std::string filePath;
    std::string fileName;
    std::string funcName;
};
using StackTrace = std::vector<TraceLog>;

StackTrace GetStackTrace();


#endif // __STACK_TRACE_H__