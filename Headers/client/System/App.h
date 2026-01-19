//
// Created by fiona on 25/09/2025.
//

#ifndef PT_APP_H
#define PT_APP_H

class D3D;

class App
{
protected:
    ~App() = default;

public:
    virtual void OnInit(D3D* d3d);
    virtual void OnUpdate(D3D* d3d, ID3D12GraphicsCommandList* cmdList) = 0;
    virtual void OnPostUpdate(D3D* d3d) = 0;
    virtual void RenderGUI() = 0;

    virtual const char* GetName() const = 0;
    bool GetIsInitialized() const { return m_initialized; }

private:
    bool m_initialized = false;
};

#endif //PT_APP_H