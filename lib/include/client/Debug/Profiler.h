//
// Created by fionaw on 13/11/2025.
//

#ifndef CHERRYPIP_PROFILER_H
#define CHERRYPIP_PROFILER_H

struct TrackedTask
{
    std::string Name;
    std::chrono::high_resolution_clock::time_point StartTime;
};

class Profiler
{
public:
    static void AddToStack(const std::wstring& name);
    static void AddToStack(const char* name);
    static void PopAndPrint();

private:
    static std::stack<TrackedTask> m_stack;
};


#endif //CHERRYPIP_PROFILER_H