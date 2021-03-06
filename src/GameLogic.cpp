#include "GameLogic.h"
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Urho3DAll.h>

#include <Navigation/GameNavigation.h>
#include "Components/RenderData.h"

#ifdef GAME_ENABLE_LUA_SCRIPTING
# include "Subsystems/LuaScripting.h"
#endif

#include "laf/LAFLogic.h"

GameLogic::GameLogic(Context* ctx)
    : Object(ctx),
      mCameraNode(nullptr),
      mScene(nullptr),
      mViewport(nullptr),
      mRenderPhysics(false)
{
}

GameLogic::~GameLogic(){
}

// ------------------------------- startup ---------------------------------------------------

void GameLogic::Setup(VariantMap& engineParameters_)
{
    engineParameters_[EP_FULL_SCREEN]=false;
    engineParameters_[EP_WINDOW_RESIZABLE]=true;
    engineParameters_[EP_WINDOW_WIDTH]=1280;
    engineParameters_[EP_WINDOW_HEIGHT]=768;
    engineParameters_[EP_WINDOW_TITLE]=String(PROJECT_NAME); // get the name from the CMake ProjectName
    engineParameters_[EP_RESOURCE_PATHS]="Data;CoreData";
    engineParameters_[EP_LOG_QUIET]=true;
    SubscribeToEvents();
}

void GameLogic::Start()
{
    SetupSystems();
    SetupScene();
    SetupViewport();
    SetupInput();
    SetupUI();
    LAFLogic* laf_logic = new LAFLogic(context_);
    context_->RegisterSubsystem(laf_logic);
    laf_logic->Setup();
}


void GameLogic::SetupSystems()
{
    mGameNavigation = new GameNavigation(context_);
    context_->RegisterSubsystem(mGameNavigation);

#ifdef GAME_ENABLE_LUA_SCRIPTING
    LuaScripting* luaScripting = new LuaScripting(context_);
    luaScripting->Init("Scripts/main.lua");
    context_->RegisterSubsystem(luaScripting);

    // make sure to include angelscript... (otherwise the angelscript part would be stript away in release and editor need it)
    // TODO: Make this better :D
    new ScriptFile(context_);    
#endif

}

SoundSource* GameLogic::GetOrCreateSoundSource(Scene* scene)
{
    if (!scene){
        scene = mScene;
    }
    auto musicSource = scene->GetOrCreateComponent<SoundSource>();
    // Set the sound type to music so that master volume control works correctly
    musicSource->SetSoundType(SOUND_MUSIC);
    return musicSource;
}

void GameLogic::SetupAudio()
{

}

void GameLogic::SetupScene()
{
    mScene = new Scene(context_);
    context_->RegisterSubsystem( mScene );

    auto cache = context_->GetSubsystem<ResourceCache>();

    SetupAudio();


    LoadFromFile("Scenes/start_scene.xml");
    mPhysicsWorld = mScene->GetComponent<PhysicsWorld>();

    mCamera = mScene->GetComponent<Camera>(true);
    if (mCamera){
        SetCameraNode(mCamera->GetNode());
    } else {
        Node* cameraNode = mScene->CreateChild("CameraNode");
        mCamera = cameraNode->CreateComponent<Camera>();
    }
}

void GameLogic::SetupInput()
{
    Input* input = GetSubsystem<Input>();
    input->SetMouseMode(MM_FREE);
    input->SetMouseVisible(true);
}

void GameLogic::SetupViewport()
{
    ResourceCache* cache = context_->GetSubsystem<ResourceCache>();

    Renderer* renderer = GetSubsystem<Renderer>();

    auto renderData = mScene->GetComponent<RenderData>(true);

    if (renderData){
        mViewport = new Viewport(context_, mScene, mCameraNode->GetComponent<Camera>());
        renderer->SetViewport(0,mViewport);

        renderData->SetRenderPathOnViewport(mViewport);
    } else {
        RenderPath* rp = new RenderPath();
        rp->Load(cache->GetResource<XMLFile>("RenderPaths/PBRDeferred.xml"));
        mViewport = new Viewport(context_, mScene, mCameraNode->GetComponent<Camera>(),rp);
        renderer->SetViewport(0,mViewport);
    }


    context_->RegisterSubsystem(mViewport);
}

void GameLogic::SubscribeToEvents()
{
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(GameLogic, HandleUpdate));
    SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(GameLogic, HandlePostRenderUpdate));

    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(GameLogic, HandleKeyDown));

    SubscribeToEvent(E_PHYSICSCOLLISIONSTART, URHO3D_HANDLER(GameLogic, HandlePhysics));
    SubscribeToEvent(E_PHYSICSCOLLISIONEND, URHO3D_HANDLER(GameLogic, HandlePhysics));

#ifdef GAME_ENABLE_DEBUG_TOOLS
    SubscribeToEvent(E_CONSOLECOMMAND, URHO3D_HANDLER(GameLogic, HandleConsoleInput));
#endif
}


void GameLogic::SetCameraNode(Node *cameraNode)
{
    if (mCameraNode==cameraNode) return;

    mCameraNode = cameraNode;

    if (mCameraNode) {
        mCamera = mCameraNode->GetComponent<Camera>();
    }
    if (mViewport){
        mViewport->SetCamera(mCamera);
    }
}

// -------------------------------- handlers ----------------------------------------------

void GameLogic::HandleUpdate(StringHash eventType, VariantMap &eventData)
{
    using namespace Update;
    float dt = eventData[P_TIMESTEP].GetFloat();

    Input* input = GetSubsystem<Input>();

    //input->SetMouseVisible(!input->GetMouseButtonDown(MOUSEB_RIGHT));
    if (input->GetKeyPress(KEY_F3)){
        mRenderPhysics = !mRenderPhysics;
        if (mGameNavigation){
            mGameNavigation->ShowDebug(!mGameNavigation->IsShowDebug());
        }
    }
}

void GameLogic::HandlePhysics(StringHash eventType, VariantMap &eventData)
{
    using namespace PhysicsCollision;

    Node* nodeA = static_cast<Node*>(eventData[P_NODEA].GetPtr());
    RigidBody* bodyA = static_cast<RigidBody*>(eventData[P_BODYA].GetPtr());

    Node* nodeB = static_cast<Node*>(eventData[P_NODEB].GetPtr());
    RigidBody* bodyB = static_cast<RigidBody*>(eventData[P_BODYB].GetPtr());

    MemoryBuffer contacts(eventData[P_CONTACTS].GetBuffer());
    bool isTrigger = eventData[P_TRIGGER].GetBool();

    while (!contacts.IsEof())
    {
        Vector3 contactPosition = contacts.ReadVector3();
        Vector3 contactNormal = contacts.ReadVector3();
        float contactDistance = contacts.ReadFloat();
        float contactImpulse = contacts.ReadFloat();
        // do something
    }

    String stTrigger = isTrigger ? "[Trigger]" : "";

    if (eventType == E_PHYSICSCOLLISIONSTART){
      URHO3D_LOGINFOF("START: %s Collision between: %s(%i) <-> %s(%i)",stTrigger.CString(),nodeA->GetName().CString(),nodeA->GetID(),nodeB->GetName().CString(),nodeB->GetID());
    }
    else if (eventType == E_PHYSICSCOLLISIONEND){
//        URHO3D_LOGINFOF("END  : %s Collision between: %s(%i) <-> %s(%i)",stTrigger.CString(),nodeA->GetName().CString(),nodeA->GetID(),nodeB->GetName().CString(),nodeB->GetID());
    }
}


void GameLogic::HandlePostRenderUpdate(StringHash eventType, VariantMap &eventData)
{
    if (mRenderPhysics) {
        mScene->GetComponent<PhysicsWorld>()->DrawDebugGeometry(false);
    }
}

void GameLogic::HandleKeyDown(StringHash eventType, VariantMap &eventData)
{
    using namespace KeyDown;
    // Check for pressing ESC. Note the engine_ member variable for convenience access to the Engine object
    int key = eventData[P_KEY].GetInt();

#ifdef GAME_ENABLE_DEBUG_TOOLS
    if (key == KEY_F1){
        mScene->SetUpdateEnabled(!mScene->IsUpdateEnabled());
    }
    else if (key == KEY_F11){
        File saveFile(context_, "./scene.write.xml",FILE_WRITE);
        mScene->SaveXML(saveFile);
        File saveFileBin(context_, "./scene.bin",FILE_WRITE);
        mScene->Save(saveFileBin);
    }
    else if (key == KEY_F12){
        Editor* editor = GetSubsystem<Editor>();
        if (!editor){
            editor = new Editor(context_);
            editor->InitEditor();
            context_->RegisterSubsystem(editor);
        }
    }
#endif
}

void GameLogic::HandleControlClicked(StringHash eventType, VariantMap& eventData)
{
//    // Get the Text control acting as the Window's title
//    auto* windowTitle = mWindow->GetChildStaticCast<Text>("WindowTitle", true);

//    // Get control that was clicked
//    auto* clicked = static_cast<UIElement*>(eventData[UIMouseClick::P_ELEMENT].GetPtr());

//    String name = "...?";
//    if (clicked)
//    {
//        // Get the name of the control that was clicked
//        name = clicked->GetName();
//    }

//    // Update the Window's title text
//    windowTitle->SetText("Hello " + name + "!");
}

#ifdef GAME_ENABLE_DEBUG_TOOLS
void GameLogic::HandleConsoleInput(StringHash eventType, VariantMap& eventData)
{
    using namespace ConsoleCommand;
    String command = eventData[P_COMMAND].GetString();
    String id = eventData[P_ID].GetString();

    if (command == "GIT_HASH"){
        URHO3D_LOGINFOF("GIT-Hash: %s",String(GIT_HASH).CString());
    }
}
#endif

// ------------------------------- misc -------------------------------------------------------

void GameLogic::LoadFromFile(String sceneName, Node* loadInto)
{
    auto cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>(sceneName);
    if (file){
        loadInto->LoadXML(file->GetRoot());
    } else {
        URHO3D_LOGERRORF("no scene %s",sceneName.CString());
    }
}

void GameLogic::LoadFromFile(String sceneName, Scene* loadInto)
{
    auto cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>(sceneName);
    if (file){
        if (loadInto){
            loadInto->LoadXML(file->GetRoot());
        } else {
            mScene->LoadXML(file->GetRoot());
        }
    } else {
        URHO3D_LOGERRORF("no scene: %s",sceneName.CString());
    }
}

void GameLogic::PlaySound(String soundFile,float gain,Scene* scene)
{
    auto* cache = GetSubsystem<ResourceCache>();
    auto* sound = cache->GetResource<Sound>("Sounds/"+soundFile);
    scene = scene == nullptr?mScene:scene;

    PlaySound(sound,gain,scene);
}

void GameLogic::PlaySound(Sound* sound,float gain,Scene* scene)
{
    SoundSource* soundSource = scene->CreateComponent<SoundSource>();
    scene = scene == nullptr?mScene:scene;
    // Component will automatically remove itself when the sound finished playing
    soundSource->SetAutoRemoveMode(REMOVE_COMPONENT);
    soundSource->Play(sound->GetDecoderStream());
    // In case we also play music, set the sound volume below maximum so that we don't clip the output
    soundSource->SetGain(gain);
}

void GameLogic::PlayMusic(String musicFile,bool looped,float gain)
{
    auto* cache = GetSubsystem<ResourceCache>();
    auto* music = cache->GetResource<Sound>("Sounds/"+musicFile);
    // Set the song to loop
    PlayMusic(music,looped,gain);
}

void GameLogic::PlayMusic(Sound* music,bool looped,float gain)
{
    music->SetLooped(true);
    auto mMusicSource = GetOrCreateSoundSource();
    mMusicSource->SetGain(0.25f);
    mMusicSource->Play(music);
}

float GameLogic::GetMusicPosition()
{
    auto mMusicSource = GetOrCreateSoundSource();
    return mMusicSource->GetTimePosition();
}

void GameLogic::SetupUI()
{
    mUiRoot = GetSubsystem<UI>()->GetRoot();
    // Create the Window and add it to the UI's root node
    // Load XML file containing default UI style sheet
    auto* cache = GetSubsystem<ResourceCache>();
    auto* style = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
    // Set the loaded style as default style
    mUiRoot->SetDefaultStyle(style);


    mWindow = new Window(context_);
    mUiRoot->AddChild(mWindow);

    // Set Window size and layout settings
    mWindow->SetMinWidth(784);
    mWindow->SetLayout(LM_VERTICAL, 6, IntRect(6, 6, 6, 6));
    mWindow->SetAlignment(HA_LEFT, VA_TOP);
    mWindow->SetName("Window");

    // Create Window 'titlebar' container
    auto* titleBar = new UIElement(context_);
    titleBar->SetMinSize(0, 24);
    titleBar->SetVerticalAlignment(VA_TOP);
    titleBar->SetLayoutMode(LM_HORIZONTAL);

    // Create the Window title Text
    mWindowTitle = new Text(context_);
    mWindowTitle->SetText("Die Karawane!");
    auto t = new Text(context_);
    t->SetText("Die Karawane!");
    t->SetFontSize(200);
//    windowTitle->SetName("WindowTitle");

//    windowTitle->SetText("Hello GUI!");

    // Add the controls to the title bar
    titleBar->AddChild(mWindowTitle);


    // Add the title bar to the Window
    mWindow->AddChild(titleBar);

    // Apply styles
    mWindow->SetStyleAuto();
    mWindowTitle->SetStyleAuto();
    mWindowTitle->SetFontSize(200);
    mWindow->SetVisible(false);
    // Subscribe to buttonClose release (following a 'press') events
 //   SubscribeToEvent(buttonClose, E_RELEASED, URHO3D_HANDLER(GameLogic, HandleClosePressed));

    // Subscribe also to all UI mouse clicks just to see where we have clicked
    SubscribeToEvent(E_UIMOUSECLICK, URHO3D_HANDLER(GameLogic, HandleControlClicked));
}



void GameLogic::SetUIText(String text,Color color)
{
    if (!mWindowTitle) return;
    mWindowTitle->SetColor(color);
    mWindowTitle->SetText(text);
}

bool GameLogic::MouseRaycast(float maxDistance, Vector3& hitPos, Node*& hitnode,String tag,Viewport* vp)
{
    hitnode = nullptr;

    auto* ui = GetSubsystem<UI>();
    IntVector2 pos = ui->GetCursorPosition();

    return Raycast(pos,maxDistance,hitPos,hitnode,tag,vp);
}

bool GameLogic::TouchRaycast(int fingerIdx,float maxDistance, Vector3& hitPos, Node*& hitnode,String tag,Viewport* vp)
{
    hitnode = nullptr;

    Input* input = GetSubsystem<Input>();
    if (!input->GetNumTouches()) return false;

    IntVector2 pos = input->GetTouch(fingerIdx)->position_;

    return Raycast(pos,maxDistance,hitPos,hitnode,tag,vp);
}



bool GameLogic::Raycast(IntVector2 screennPos,float maxDistance, Vector3& hitPos, Node*& hitnode,String tag,Viewport* vp)
{
    hitnode = nullptr;

    auto* graphics = GetSubsystem<Graphics>();
    Scene* scene = vp ? vp->GetScene() : GetSubsystem<Scene>();
    Ray cameraRay = vp
                      ? vp->GetScreenRay(screennPos.x_,screennPos.y_)
                      : mCamera->GetScreenRay((float)screennPos.x_ / graphics->GetWidth(), (float)screennPos.y_ / graphics->GetHeight());

    // Pick only geometry objects, not eg. zones or lights, only get the first (closest) hit
    PODVector<RayQueryResult> results;
    RayOctreeQuery query(results, cameraRay, RAY_TRIANGLE, maxDistance, DRAWABLE_GEOMETRY);
    scene->GetComponent<Octree>()->RaycastSingle(query);

    for(auto result : results){
        if (result.drawable_ && (tag=="" || result.drawable_->GetNode()->HasTag(tag))){
            hitPos = result.position_;
            hitnode = result.drawable_->GetNode();
            return true;
        }
    }


    return false;
}

bool GameLogic::PhysicsRaycast(IntVector2 screenPos,float maxDistance, Vector3& hitPos, RigidBody*& hitDrawable,String tag,Viewport* vp)
{
    hitDrawable = nullptr;

    auto* graphics = GetSubsystem<Graphics>();
    Scene* scene = vp ? vp->GetScene() : GetSubsystem<Scene>();
    Ray cameraRay = vp
                        ? vp->GetScreenRay(screenPos.x_,screenPos.y_)
                        : mCamera->GetScreenRay((float)screenPos.x_ / graphics->GetWidth(), (float)screenPos.y_ / graphics->GetHeight());

    // Pick only geometry objects, not eg. zones or lights, only get the first (closest) hit

    PODVector<PhysicsRaycastResult> results;

    auto physics_world = scene->GetComponent<PhysicsWorld>(true);
    physics_world->Raycast(results,cameraRay,maxDistance);

    if (results.Size() == 0){
        return false;
    }

    for(auto result : results){
        if (result.body_ && (tag=="" || result.body_->GetNode()->HasTag(tag))){
            hitPos = result.position_;
            hitDrawable = result.body_;
            return true;
        }
    }

    return false;
}

bool GameLogic::TouchPhysicsRaycast(int fingerIdx, float maxDistance, Vector3 &hitPos, RigidBody *&hitRigidbody,String tag,Viewport* vp)
{
    //URHO3D_LOGINFO("TRY TOUCH!");
    hitRigidbody = nullptr;

    Input* input = GetSubsystem<Input>();

    if (!input->GetNumTouches()) return false;


    IntVector2 pos = input->GetTouch(fingerIdx)->position_;
    //URHO3D_LOGINFOF("TOUCH:  %i  vec:%s",fingerIdx,pos.ToString().CString());

    return PhysicsRaycast(pos,maxDistance,hitPos,hitRigidbody,tag,vp);
}

bool GameLogic::MousePhysicsRaycast(float maxDistance, Vector3 &hitPos, RigidBody *&hitRigidbody,String tag,Viewport* vp)
{
    hitRigidbody = nullptr;

    auto* ui = GetSubsystem<UI>();
    IntVector2 pos = ui->GetCursorPosition();

    return PhysicsRaycast(pos,maxDistance,hitPos,hitRigidbody,tag,vp);
}

bool GameLogic::MouseOrTouchPhysicsRaycast(float maxDistance, Vector3 &hitPos, RigidBody *&hitRigidbody, String tag,Viewport* vp)
{
    Input* input = GetSubsystem<Input>();

    //URHO3D_LOGINFOF("TOUCHES:%i",input->GetNumTouches());
    if (MousePhysicsRaycast(maxDistance,hitPos,hitRigidbody,tag,vp)){
        return true;
    }
    if (TouchPhysicsRaycast(0,maxDistance,hitPos,hitRigidbody,tag,vp)){
        return true;
    }
    return false;
}

bool GameLogic::MouseOrTouchRaycast(float maxDistance, Vector3& hitPos, Node*& hitnode,String tag,Viewport* vp)
{
    if (MouseRaycast(maxDistance,hitPos,hitnode,tag,vp)){
        return true;
    }
    if (TouchRaycast(0,maxDistance,hitPos,hitnode,tag,vp)){
        return true;
    }
    return false;
}


Node* GameLogic::GetFirstChildWithTag(Node* startNode,const String& tag,bool recursive)
{
    PODVector<Node*> nodes;
    startNode->GetChildrenWithTag(nodes,tag,recursive);
    if (nodes.Size()==0) return nullptr;

    return nodes[0];
}

Node* GameLogic::GetFirstParentWithTag(Node *startNode, const String &tag, bool recursive)
{
    Node* parent = startNode;
    while (parent=parent->GetParent()){
        if (parent->HasTag(tag)){
            return parent;
        }
        if (!recursive){
            return nullptr;
        }
    }
    return nullptr;
}



bool GameLogic::IsMousePressedOrTouch(MouseButton mousebtn, int fingerIdx)
{
    Input* input = GetSubsystem<Input>();
    if (input->GetMouseButtonPress(mousebtn)){
        return true;
    }
    else if (input->GetNumTouches()>0) {
        // TODO check for fingerIdx
        return true;
    }
    return false;
}

bool GameLogic::IsMouseDownOrTouch(MouseButton mousebtn, int fingerIdx)
{
    Input* input = GetSubsystem<Input>();
    if (input->GetMouseButtonDown(mousebtn)){
        return true;
    }
    else if (input->GetNumTouches()>0) {
        // TODO check for fingerIdx
        return true;
    }
    return false;
}
