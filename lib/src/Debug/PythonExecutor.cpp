//
// Created by fionaw on 06/11/2025.
//


#include "Debug/PythonExecutor.h"

void PythonExecutor::ExecutePython(const char* pythonFile, const std::vector<const char*>& args)
{
    std::string command = "python ";
    command += "\"" + std::string(SOURCE_DIR) + "/Python Scripts/" + std::string(pythonFile) + "\"";
    for (int i = 0; i < args.size(); i++)
        command += " " + std::string(args[i]);
    system(command.c_str());
}
