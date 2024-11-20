
#include "PCH.h"

#include "Core/Engine.h"
#include "Core/ThreadPool.h"
#include "Asset/Asset.h"
#include "Rendering/Renderer.h"
#include "Rendering/OpenGL/GLEditorUI.h"
#include "Editor/EditorSplashImage.h"


namespace Silex
{
    static Engine* engine = nullptr;
    static Window* window = nullptr;


    bool LaunchEngine()
    {
        // OS初期化
        OS::Get()->Initialize();

        // スプラッシュイメージ表示
        EditorSplashImage::Show();

        // コア機能初期化
        Logger::Initialize();
        Memory::Initialize();
        Input::Initialize();
        ThreadPool::Initialize();

        SL_LOG_INFO("***** Launch Engine *****");

        // エンジン初期化
        engine = Memory::Allocate<Engine>();
        if (!engine->Initialize())
        {
            EditorSplashImage::Hide();
            return false;
        }

        // スプラッシュイメージ非表示
        EditorSplashImage::Hide();

        return true;
    }

    void ShutdownEngine()
    {
        if (engine)
        {
            engine->Finalize();
            Memory::Deallocate(engine);
        }

        SL_LOG_INFO("***** Shutdown Engine *****");

        ThreadPool::Finalize();
        Input::Finalize();
        Memory::Finalize();
        Logger::Finalize();

        OS::Get()->Finalize();
    }




    //=========================================
    // Engine
    //=========================================
    Engine* Engine::Get()
    {
        return engine;
    }

    bool Engine::Initialize()
    {
        bool result = false;

        // ウィンドウ
        window = Window::Create("Silex", 1280, 720);
        result = window->Initialize();
        SL_CHECK(!result, false);

        // コールバック登録
        window->BindWindowCloseEvent(this,  &Engine::OnWindowClose);
        window->BindWindowResizeEvent(this, &Engine::OnWindowResize);
        window->BindMouseMoveEvent(this,    &Engine::OnMouseMove);
        window->BindMouseScrollEvent(this,  &Engine::OnMouseScroll);

        // レンダラー
        Renderer::Get()->Init();

        // アセットマネージャー
        AssetManager::Get()->Init();

        // エディターUI (ImGui)
        editorUI = EditorUI::Create();
        editorUI->Init();

        // エディター
        editor = Memory::Allocate<Editor>();
        editor->Init();

        // ウィンドウ表示
        window->Show();

        return true;
    }

    bool Engine::MainLoop()
    {
        CalcurateFrameTime();

        if (!minimized)
        {
            Renderer::Get()->BeginFrame();
            editorUI->BeginFrame();

            editor->Update(deltaTime);
            editor->Render();

            editorUI->EndFrame();
            Renderer::Get()->EndFrame();

            Input::Flush();
        }

        PerformanceProfiler::Get().GetFrameData(&performanceData, true);

        // メインループ抜け出し確認
        return isRunning;
    }

    void Engine::Finalize()
    {
        if (editor)
        {
            editor->Shutdown();
        }

        if (editorUI)
        {
            editorUI->Shutdown();
        }

        AssetManager::Get()->Shutdown();
        Renderer::Get()->Shutdown();

        // ウィンドウ破棄
        if (window)
        {
            window->Finalize();
            Memory::Deallocate(window);
        }
    }

    // OnWindowCloseイベント（Xボタン / Alt + F4）以外で、エンジンループを終了させる場合に使用
    void Engine::Close()
    {
        isRunning = false;
    }

    void Engine::OnWindowResize(WindowResizeEvent& e)
    {
        if (e.width == 0 || e.height == 0)
        {
            minimized = true;
            return;
        }

        minimized = false;
        Renderer::Get()->Resize(e.width, e.height);
    }

    void Engine::OnWindowClose(WindowCloseEvent& e)
    {
        isRunning = false;
    }

    void Engine::OnMouseMove(MouseMoveEvent& e)
    {
        editor->OnMouseMove(e);
    }

    void Engine::OnMouseScroll(MouseScrollEvent& e)
    {
        editor->OnMouseScroll(e);
    }


    void Engine::CalcurateFrameTime()
    {
        uint64 time = OS::Get()->GetTickSeconds();
        deltaTime     = (double)(time - lastFrameTime) / 1'000'000;
        lastFrameTime = time;

        static float  secondLeft = 0.0f;
        static uint32 frame = 0;
        secondLeft += deltaTime;
        frame++;

        if (secondLeft >= 1.0f)
        {
            frameRate  = frame;
            frame      = 0;
            secondLeft = 0.0f;
        }
    }
}


