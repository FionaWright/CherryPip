//
// Created by fionaw on 06/11/2025.
//

#ifndef CHERRYPIP_PYTHONEXECUTOR_H
#define CHERRYPIP_PYTHONEXECUTOR_H

class PythonExecutor
{
public:
    static void ExecutePython(const char* pythonFile, const std::vector<const char*>& args);
};


#endif //CHERRYPIP_PYTHONEXECUTOR_H