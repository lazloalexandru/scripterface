#include "trace.h"
#include "../include/trace_config.h"
#include "../include/script_interpreter.h"

struct TraceConfig g_traceConf
{
    "trace_log.txt",
        TraceTarget::NONE
        //| TraceTarget::TRACE_TO_FILE
        //|TraceTarget::TRACE_TO_STDOUT
};


Trace::Trace()
{
    if (g_traceConf.target)
    {
        if (TraceTarget::TRACE_TO_FILE & g_traceConf.target)
        {
            ofs.open(g_traceConf.traceFilePath);

            if (!ofs || !ofs.good())
            {
                std::cout << "Unable to create TRACE file: " << g_traceConf.traceFilePath << std::endl;
            }

            std::cout << "[TRACE Output File: " << g_traceConf.traceFilePath << "]" << std::endl;
        }
    }
    else
    {
        std::cout << "[TRACE OFF]" << std::endl;
    }
}

void Trace::Flush()
{
    if (TraceTarget::TRACE_TO_FILE & g_traceConf.target) ofs << ss.str();
    if (TraceTarget::TRACE_TO_STDOUT & g_traceConf.target) std::cout << ss.str();

    ss.str("");         // clear buffer
}

std::stringstream& Trace::getBuffer()
{
    return ss;
}

Trace& Trace::Instance()
{
    static Trace singleton;
    return singleton;
}

void SetTraceConfig(std::string traceFilePath, TraceTarget target)
{
    g_traceConf.target = target;
    g_traceConf.traceFilePath = traceFilePath;
}

