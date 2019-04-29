#pragma once

#include <string>

enum TraceTarget
{
    NONE = 0x0,
    TRACE_TO_FILE = 0x1,
    TRACE_TO_STDOUT = 0x2
};

void SetTraceConfig(std::string traceFilePath, TraceTarget target);
