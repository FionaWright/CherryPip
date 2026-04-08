#ifndef H_DEBUGOUTPUTREDIRECTOR_H
#define H_DEBUGOUTPUTREDIRECTOR_H

void __stdcall DebugMessageCallback(
    D3D12_MESSAGE_CATEGORY category,
    D3D12_MESSAGE_SEVERITY severity,
    D3D12_MESSAGE_ID id,
    LPCSTR description,
    void* context)
{
    std::ostream& out = *reinterpret_cast<std::ostream*>(context);

    switch (severity)
    {
    case D3D12_MESSAGE_SEVERITY_CORRUPTION:
    case D3D12_MESSAGE_SEVERITY_ERROR:
        out << "[ERROR] ";
        break;
    case D3D12_MESSAGE_SEVERITY_WARNING:
        out << "[WARN ] ";
        break;
    case D3D12_MESSAGE_SEVERITY_INFO:
        out << "[INFO ] ";
        break;
    case D3D12_MESSAGE_SEVERITY_MESSAGE:
        out << "[MSG  ] ";
        break;
    default:
        break;
    }

    out << description << std::endl;
}


#endif