
#pragma once
#include "Core/Window.h"

namespace Silex
{
    struct WindowsWindowHandle
    {
        HWND      windowHandle = nullptr;
        HINSTANCE instanceHandle = nullptr;
    };


    class WindowsWindow : public Window
    {
        SL_CLASS(WindowsWindow, Window)

    public:

        WindowsWindow(const char* title, uint32 width, uint32 height);
        ~WindowsWindow();

        bool Initialize() override;
        void Finalize()   override;

        void PumpMessage()override;

        glm::ivec2 GetSize()      const override;
        glm::ivec2 GetWindowPos() const override;

        virtual void Maximize() override;
        virtual void Minimize() override;
        virtual void Restore()  override;
        virtual void Show()     override;
        virtual void Hide()     override;

        virtual const char* GetTitle() const            override;
        virtual void        SetTitle(const char* title) override;

        void*             GetWindowHandle() const override;
        GLFWwindow*       GetGLFWWindow()   const override;
        const WindowData& GetWindowData()   const override;

    private:

        // レンダリングコンテキスト
        RenderingContext* renderingContext = nullptr;
        RenderingDevice*  renderingDevice  = nullptr;

        // ウィンドウデータ
        GLFWwindow* window = nullptr;
        WindowData  windowData;

        // Windows 固有ハンドル
        WindowsWindowHandle handle;
    };
}

