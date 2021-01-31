#pragma once

#include <project_options.h>

#include <Urho3D/Core/Object.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Container/Str.h>

#include <Editor/Editor.h>

using namespace Urho3D;

// todo: solve this differently. some include?:
namespace Urho3D {
    class Drawable;
    class RigidBody;
    class SoundSource;
    class Window;
    class Text;
    class PhysicsWorld;
    class UIElement;
}

class GameNavigation;
class Caravaner;

class GameLogic : public Object
{
    URHO3D_OBJECT(GameLogic,Object)

public:
    GameLogic(Context* ctx);
    ~GameLogic() override;

    void Setup(VariantMap& engineParameters_);
    void LoadFromFile(String sceneName,Node* node);
    void LoadFromFile(String sceneName,Scene* scene=nullptr);
    void Start();
    inline Scene* GetScene() { return mScene; }
    inline void   SetScene(SharedPtr<Scene> scene) { mScene = scene; }

    void PlaySound(Sound* soundFile,float gain=0.75f);
    void PlaySound(String soundFile,float gain=0.75f);
    void PlayMusic(Sound* musicFile,bool looped=true,float gain=0.25f);
    void PlayMusic(String musicFile,bool looped=true,float gain=0.25f);
    float GetMusicPosition();

    void SetupAudio();
    SoundSource* GetOrCreateSoundSource(Scene* scene=nullptr);

    void SetUIText(String text,Color color = Color::WHITE);

    bool TouchRaycast(int fingerIdx,float maxDistance, Vector3& hitPos, Node*& hitnode,String tag="",Viewport* vp=nullptr);
    bool MouseRaycast(float maxDistance, Vector3& hitPos, Node*& hitnode,String tag="",Viewport* vp=nullptr);
    bool Raycast(IntVector2 screenPos,float maxDistance, Vector3& hitPos, Node*& hitnode,String tag="",Viewport* vp=nullptr);
    bool MouseOrTouchRaycast(float maxDistance, Vector3& hitPos, Node*& hitnode,String tag="",Viewport* vp=nullptr);

    bool TouchPhysicsRaycast(int fingerIdx,float maxDistance, Vector3& hitPos, RigidBody*& hitRigidbody,String tag="",Viewport* vp=nullptr);
    bool MousePhysicsRaycast(float maxDistance, Vector3& hitPos, RigidBody*& hitRigidbody,String tag="",Viewport* vp=nullptr);
    bool PhysicsRaycast(IntVector2 screenPos,float maxDistance, Vector3& hitPos, RigidBody*& hitRigidbody,String tag="",Viewport* vp=nullptr);
    bool MouseOrTouchPhysicsRaycast(float maxDistance, Vector3& hitPos, RigidBody*& hitRigidbody,String tag="",Viewport* vp=nullptr);

    bool IsMousePressedOrTouch(MouseButton mousebtn=MOUSEB_LEFT,int fingerIdx=0); // TODO: real touch-pressed
    bool IsMouseDownOrTouch(MouseButton mousebtn=MOUSEB_LEFT,int fingerIdx=0);

    void SetCameraNode(Node* cameraNode);
    inline Node* GetCameraNode() { return mCameraNode; }

    Node* GetFirstChildWithTag(Node* startNode,const String& tag,bool recursive=false);
    Node* GetFirstParentWithTag(Node* startNode,const String& tag,bool recursive=false);

    inline Viewport* GetViewport(){ return mViewport;}

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
    void HandlePhysics(StringHash eventType,VariantMap& eventData);


#ifdef GAME_ENABLE_DEBUG_TOOLS
    void HandleConsoleInput(StringHash eventType, VariantMap& eventData);
#endif
    Node* mCameraNode;
    Camera* mCamera;
    Scene* mScene;
    Viewport* mViewport;

    Input* input;

    bool mRenderPhysics;

    SharedPtr<Window> mWindow;
    SharedPtr<Text> mWindowTitle;
    /// The UI's root UIElement.
    SharedPtr<UIElement> mUiRoot;

    SharedPtr<GameNavigation> mGameNavigation;
    SharedPtr<PhysicsWorld> mPhysicsWorld;

};

