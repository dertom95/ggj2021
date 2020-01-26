#pragma once

#include <project_options.h>

#include <Urho3D/Core/Object.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Container/Str.h>

#include "Subsystems/Editor.h"

using namespace Urho3D;

class GameNavigation;

class GameLogic : public Object
{
    URHO3D_OBJECT(GameLogic,Object);

public:
    GameLogic(Context* ctx);
    ~GameLogic();

    void Setup(VariantMap& engineParameters_);
    void LoadFromFile(String sceneName,Node* node);
    void LoadFromFile(String sceneName,Scene* scene=nullptr);
    void Start();
    inline Scene* GetScene() { return mScene; }

    void PlaySound(String soundFile);
    void PlayMusic(String musicFile);

    void SetUIText(String text);

    bool TouchRaycast(int fingerIdx,float maxDistance, Vector3& hitPos, Drawable*& hitDrawable);
    bool MouseRaycast(float maxDistance, Vector3& hitPos, Drawable*& hitDrawable);
    bool Raycast(IntVector2 screenPos,float maxDistance, Vector3& hitPos, Drawable*& hitDrawable);

private:
    void SubscribeToEvents();
    void SetupSystems();
    void SetupViewport();
    void SetupScene();
    void SetupInput();
    void SetupUI(); // some sample ui


    void HandleUpdate(StringHash eventType, VariantMap &eventData);
    void HandlePostRenderUpdate(StringHash eventType, VariantMap &eventData);
    void HandleKeyDown(StringHash eventType, VariantMap& eventData);
    void HandleControlClicked(StringHash eventType, VariantMap& eventData);


#ifdef GAME_ENABLE_DEBUG_TOOLS
    void HandleConsoleInput(StringHash eventType, VariantMap& eventData);
#endif
    Node* mCameraNode;
    Camera* mCamera;
    Scene* mScene;
    Viewport* mViewport;

    SoundSource* mMusicSource;
    bool mRenderPhysics;

    SharedPtr<Window> mWindow;
    /// The UI's root UIElement.
    SharedPtr<UIElement> mUiRoot;

    SharedPtr<GameNavigation> mGameNavigation;
};
