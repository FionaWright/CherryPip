#include "System/pch.h"

#include "Apps/SceneStudio/Headers/SceneStudio.h"
#include "MathUtils.h"

void SceneStudio::InitializeScenes()
{
    SceneConfig customScene = {
        "(" + std::to_string(m_sceneConfigs.size()) +") " + "Custom"
    };
    m_sceneConfigs.emplace_back(customScene);

    Transform t = {};

    t.SetScale(2.0f);
    SceneConfig sceneCornell = {
        "(" + std::to_string(m_sceneConfigs.size()) +") " + "Cornell Box",
        L"Cornell/scene.gltf",
        t,
        XMFLOAT3(0, 1.5f, 4.5f),
        XMFLOAT2(0, PI),
        true
    };
    m_sceneConfigs.emplace_back(sceneCornell);

    t = {};
    SceneConfig sceneSphere = {
        "(" + std::to_string(m_sceneConfigs.size()) +") " + "Sphere",
        L"Sphere/Sphere.gltf",
        t,
        XMFLOAT3(0, 0, -4.3f),
        XMFLOAT2(0, 0),
        true
    };
    m_sceneConfigs.emplace_back(sceneSphere);

    t = {};
    SceneConfig scenePlane = {
        "(" + std::to_string(m_sceneConfigs.size()) +") " + "FloatPlane",
        L"floatplane.glb",
        t,
        XMFLOAT3(0, 0, -4.3f),
        XMFLOAT2(0, PI),
        true
    };
    m_sceneConfigs.emplace_back(scenePlane);

    t = {};
    t.SetScale(0.3f);
    SceneConfig sceneTeapot = {
        "(" + std::to_string(m_sceneConfigs.size()) +") " + "Utah Teapot",
        L"Utah Teapot/scene.gltf",
        t,
        XMFLOAT3(0, 0, -4.3f),
        XMFLOAT2(0, PI),
        true
    };
    m_sceneConfigs.emplace_back(sceneTeapot);

    t = {};
    t.SetScale(5.0f);
    SceneConfig sceneChess = {
        "(" + std::to_string(m_sceneConfigs.size()) +") " + "Chess",
        L"Chess/Chess.gltf",
        t,
        XMFLOAT3(0.45f, 0.919f, -1.434f),
        XMFLOAT2(0.66f, -0.36f),
        false
    };
    m_sceneConfigs.emplace_back(sceneChess);

    t = {};
    t.SetScale(2.0f);
    SceneConfig sceneLantern = {
        "(" + std::to_string(m_sceneConfigs.size()) +") " + "Lantern",
        L"Lantern/Lantern.gltf",
        t,
        XMFLOAT3(0.967f, 11.963f, 50.213f),
        XMFLOAT2(0, PI),
        true
    };
    m_sceneConfigs.emplace_back(sceneLantern);

    t = {};
    SceneConfig sceneBistro = {
        "(" + std::to_string(m_sceneConfigs.size()) +") " + "Bistro",
        L"GitIgnored/Bistro/bistro.gltf",
        t,
        XMFLOAT3(0.967f, 11.963f, 50.213f),
        XMFLOAT2(0, PI),
        false
    };
    m_sceneConfigs.emplace_back(sceneBistro);

    t = {};
    SceneConfig sceneSponza = {
        "(" + std::to_string(m_sceneConfigs.size()) +") " + "Sponza",
        L"GitIgnored/Sponza/sponza.gltf",
        t,
        XMFLOAT3(1.753f, 1.274f, -0.23f),
        XMFLOAT2(0.105f, 4.747f),
        false
    };
    m_sceneConfigs.emplace_back(sceneSponza);

    t = {};
    SceneConfig sceneMrSpheres = {
        "(" + std::to_string(m_sceneConfigs.size()) +") " + "MetalRough Spheres",
        L"MetalRoughSpheres/MetalRoughSpheres.gltf",
        t,
        XMFLOAT3(-0.5f, -0.25f, 8.5f),
        XMFLOAT2(0, PI),
        true
    };
    m_sceneConfigs.emplace_back(sceneMrSpheres);

    t = {};
    SceneConfig sceneAnisoDiscs = {
        "(" + std::to_string(m_sceneConfigs.size()) +") " + "Aniso Discs",
        L"AnisoDiscs/AnisotropyDiscTest.gltf",
        t,
        XMFLOAT3(0.17f, 1.471f, 3.542f),
        XMFLOAT2(0.0f, PI),
        true
    };
    m_sceneConfigs.emplace_back(sceneAnisoDiscs);

    t = {};
    t.SetScale(6.0f);
    SceneConfig sceneBarnLamp = {
        "(" + std::to_string(m_sceneConfigs.size()) +") " + "Barn Lamp",
        L"BarnLamp/AnisotropyBarnLamp.gltf",
        t,
        XMFLOAT3(0.366f, -0.104f, 0.552f),
        XMFLOAT2(0.135f, 4.012f),
        true
    };
    m_sceneConfigs.emplace_back(sceneBarnLamp);

    t = {};
    SceneConfig sceneWhiteLands = {
        "(" + std::to_string(m_sceneConfigs.size()) +") " + "White Lands",
        L"GitIgnored/WhiteLands/WhiteLands.gltf",
        t,
        XMFLOAT3(0,0,0),
        XMFLOAT2(0, PI),
        false
    };
    m_sceneConfigs.emplace_back(sceneWhiteLands);

    t = {};
    t.SetScale(3.0f);
    SceneConfig sceneCozyKitchen = {
        "(" + std::to_string(m_sceneConfigs.size()) +") " + "CozyKitchen",
        L"GitIgnored/CozyKitchen/CozyKitchen.gltf",
        t,
        XMFLOAT3(1.339f, 1.728f, 0.989f),
        XMFLOAT2(0.12f, 3.997f),
        false
    };
    m_sceneConfigs.emplace_back(sceneCozyKitchen);

    t = {};
    t.SetScale(2.0f);
    SceneConfig sceneAutumn = {
        "(" + std::to_string(m_sceneConfigs.size()) +") " + "Autumn",
        L"GitIgnored/Autumn/Autumn.gltf",
        t,
        XMFLOAT3(1.339f, 1.728f, 0.989f),
        XMFLOAT2(0.12f, 3.997f),
        false
    };
    m_sceneConfigs.emplace_back(sceneAutumn);

    t = {};
    t.SetScale(2.0f);
    SceneConfig sceneJunkshop = {
        "(" + std::to_string(m_sceneConfigs.size()) +") " + "Junkshop",
        L"GitIgnored/Junkshop/Junkshop.gltf",
        t,
        XMFLOAT3(1.339f, 1.728f, 0.989f),
        XMFLOAT2(0.12f, 3.997f),
        false
    };
    m_sceneConfigs.emplace_back(sceneJunkshop);

    t = {};
    t.SetScale(2.0f);
    SceneConfig sceneLoneMonk = {
        "(" + std::to_string(m_sceneConfigs.size()) +") " + "LoneMonk",
        L"GitIgnored/LoneMonk/LoneMonk.gltf",
        t,
        XMFLOAT3(1.339f, 1.728f, 0.989f),
        XMFLOAT2(0.12f, 3.997f),
        false
    };
    m_sceneConfigs.emplace_back(sceneLoneMonk);

    t = {};
    t.SetScale(2.0f);
    SceneConfig sceneBottleShip = {
        "(" + std::to_string(m_sceneConfigs.size()) +") " + "Bottle Ship",
        L"BottleShip/scene.gltf",
        t,
        XMFLOAT3(1.339f, 1.728f, 0.989f),
        XMFLOAT2(0.12f, 3.997f),
        false
    };
    m_sceneConfigs.emplace_back(sceneBottleShip);
}