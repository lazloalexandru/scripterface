#pragma once

#include <fstream>
#include <iostream>
#include <sstream>

#define _TRACE_(x) Trace::Instance().getBuffer() << x; Trace::Instance().Flush();

struct TraceConfig
{
    std::string traceFilePath;
    unsigned int target;
};

class Trace
{
private:
    std::ofstream ofs;
    std::stringstream ss;

private:
    Trace();

public:

    void Flush();
    std::stringstream& getBuffer();
    static Trace& Instance();
};

