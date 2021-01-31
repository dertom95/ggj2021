#pragma once

#include <Urho3D/Urho3DAll.h>
#include "../GameLogic.h"
#include "components/LAFComponents.h"
#include <functional>

typedef std::function<bool(float)> ProcessFunction;

struct SceneInfo {
    String scene_name="";

    int swaps = 1;
    int color_toggles = 1;
    float swap_speed = 1.0f;
    float color_toggle_speed = 1.f;
    float show_pause = 1.0f;
    String hint = "This is THE hint";

    SceneInfo(const String& scene_name,int color_toggles,int swaps,String hint=""){
        this->scene_name=scene_name;
        this->swaps=swaps;
        this->color_toggles=color_toggles;
        this->hint=hint;
    }

    SceneInfo(){
    }
};

struct Settings {
    float camera_speed = 2.0f;
    float long_press_time = 0.1f;
    float ingame_color_toggle_speed = 2.1f;
    float ingame_swap_speed = 3.0f;
    Vector<String> color_materials = {"red","green","yellow","whitegrey"};
    String dragpoint_prefab = "Objects/col_default_targetpoint.xml";

    int scene_start_easy=0;
    int scene_start_medium=1;
    int scene_start_hard=0;

    int current_level=0;
    int best_level=0;

    Vector<SceneInfo> scenes ={
//        SceneInfo("Scenes/laf_01.xml",1,0,"1 color toggle"),
//        SceneInfo("Scenes/laf_01.xml",0,1,"1 position swap"),
//        SceneInfo("Scenes/laf_01.xml",1,1,"1 color toggle / 1 position swap"),
//        SceneInfo("Scenes/laf_01.xml",1,2,"1 color toggle / 2 position swap"),
        SceneInfo("Scenes/laf_01.xml",0,2,"2 color toggle / 2 position swap"),
        //SceneInfo("Scenes/test_scene.xml",,2)
    };
};

struct TargetGroup;

struct TargetElement {
    SharedPtr<Node> node;
    bool color_switch_allowed=true;
    String ui_name;
    String type;
    TargetGroup* target_group;
//    TargetElement(Node* node){
//        this->node=node;
//    }
};

struct UIData {
    // start-screen
    SharedPtr<UIElement> root_start;
    SharedPtr<Button> btnEasy;
    SharedPtr<Button> btnMedium;
    SharedPtr<Button> btnHard;

    // ingame
    SharedPtr<UIElement> root_ingame;
    SharedPtr<Button> btnRestartLevel;
    SharedPtr<Button> btnMainMenu;
    SharedPtr<Button> btnBottomRight;
    SharedPtr<Text> txtBtnBottomRight;
    SharedPtr<Text> progress;
    SharedPtr<Window> hint_window;
    SharedPtr<Window> instructions_window;
    SharedPtr<Text> txtInstructions;
    SharedPtr<Text> hint;
};

struct TargetGroup {
    SharedPtr<Node> node;
    Vector<TargetElement> group_elements;
};

struct ProcessCtxProgress {
    float progress;
    Vector3 from=Vector3::ZERO;
    Vector3 to=Vector3::ZERO;
};

struct ProcessCtxGroup {
    Vector<ProcessFunction> group;
};

struct SceneData {
    SceneInfo scene_info;
    Vector3 camera_max_left;
    Vector3 camera_max_right;
    SharedPtr<Node> camera_lookat;
    SharedPtr<Node> camera_node;
    SharedPtr<Node> drag_plane;
    SharedPtr<Camera> camera;

    Vector<ProcessFunction> process_sequence;

    Vector<SharedPtr<TargetElementComponent>> te_all;
    Vector<SharedPtr<TargetGroupComponent>> targetGroups;
    Vector<SharedPtr<TargetGroupComponent>> targetSwapGroups;

    float camera_target=0.5f;
};

struct ClickCheck {
    bool wasDown = false;
    float downTimer = 0.0f;
};

struct ActionDrag {
    bool active=false;
    SharedPtr<TargetElementComponent> drag_te;
    Vector3 position;
};


// this component is used from within the dertom's urho3d-exporter for collection-instances.
// on load the corresponding collections are loaded and added to this node
class LAFLogic : public Object
{
    URHO3D_OBJECT(LAFLogic,Object);
public:
    enum class GameState {
        init_scene = 0,
        paused = 1,
        playing_observer = 2,
        playing_phase2 = 3,
        success=4,
    };

    enum class UIState {
        init_scene = 0,
        ingame = 1,
    };

    static void RegisterObject(Context *context);

    LAFLogic(Context* ctx);

    void Setup();
    void SetupUI();
    void LayoutUI();
    void StartScene(int idx);
    void HandleUpdate(StringHash eventType, VariantMap& data);
    void HandleScreenChange(StringHash eventType, VariantMap& data);
    void HandleUI(StringHash eventType,VariantMap& data);

private:

    void AddTargetElement(String itemtype,String groupName);
    void MoveCameraTarget(float delta);
    void SetCameraPos(float pos);

    bool IsStartScreenVisible();
    void ShowStartScreen(bool show);

    int TE_NextColor(SharedPtr<TargetElementComponent> te,bool animate=false,bool anim_in_sequence=false,float pause=0.0f);
    int TE_RandomColor(SharedPtr<TargetElementComponent> te,bool animate=false,bool anim_in_sequence=false,float pause=0.0f);
    void TE_SetColor(SharedPtr<TargetElementComponent> te,int colorIdx,bool animate=false,bool anim_in_sequence=false,float pause=0.0f);
    bool TE_CheckGoals(SharedPtr<TargetElementComponent> te);
    bool CheckSuccess();

    void NextLevel();
    void StartPhase2();
    void SetGameState(GameState state);

    void OnShortClick();
    void OnLongClick();
    void OnDrag();
    void OnDragEnd();

    void OnSuccess();

    SharedPtr<TargetElementComponent> PickBehindDragElement(bool only_valid=true);

    void ProcessInput(float dt);

    void ProcessTES(Node* start_node);

    void AddProcess(ProcessFunction proc,bool add_to_sceneseq=false,float pause=0.0f);
    void ProcessLambdas(float dt);
    ProcessFunction CreateSwapAnimation(SharedPtr<TargetElementComponent> swapA,SharedPtr<TargetElementComponent> swapB,float speed);
    ProcessFunction CreateMoveAnimation(SharedPtr<TargetElementComponent> swapA,Vector3 position,float speed);
    ProcessFunction CreateColorAnimation(SharedPtr<TargetElementComponent> te,SharedPtr<Material> targetMaterial,float speed);
    ProcessFunction CreatePause(float secs);
    ProcessFunction CreateSequence(Vector<ProcessFunction> sequence);


    SceneData sceneData;
    UIData uiData;

    bool init_scene_initialized=false;
    GameState gamestate = GameState::init_scene;
    GameState requestState = GameState::init_scene;

    UIState ui_state = UIState::init_scene;

    SharedPtr<Window> window_init_screen;

    Settings settings;

    SharedPtr<Scene>  scene;

    ClickCheck click_check;
    ActionDrag action_drag;

    Vector<ProcessFunction> processes;

    SharedPtr<Node> initialSceneNode;
    SharedPtr<Node> levelNode;
    GameLogic* gl;
    SharedPtr<ResourceCache> resCache;
    SharedPtr<Input> input;
    HashMap<String,Vector<TargetElement>> target_elements;

};
