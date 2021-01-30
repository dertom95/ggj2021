#pragma once

#include <Urho3D/Urho3DAll.h>
#include "../GameLogic.h"
#include "components/LAFComponents.h"
#include <functional>

struct SceneInfo {
    String scene_name="";

    int swaps = 1;
    int color_toggles = 1;
    float swap_speed = 3.0f;
    float color_toggle_speed = 3.0f;

    SceneInfo(const String& scene_name,int color_toggles,int swaps){
        this->scene_name=scene_name;
        this->swaps=swaps;
        this->color_toggles=color_toggles;
    }

    SceneInfo(){
    }
};

struct Settings {
    float camera_speed = 2.0f;
    float long_press_time = 0.2f;
    Vector<String> color_materials = {"red","green","yellow","whitegrey"};
    String dragpoint_prefab = "Objects/col_default_targetpoint.xml";
    Vector<SceneInfo> scenes ={
        SceneInfo("Scenes/test_scene.xml",1,1),
        SceneInfo("Scenes/test_scene.xml",1,1)
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

struct TargetGroup {
    SharedPtr<Node> node;
    Vector<TargetElement> group_elements;
};

struct SceneData {
    SceneInfo scene_info;
    Vector3 camera_max_left;
    Vector3 camera_max_right;
    SharedPtr<Node> camera_lookat;
    SharedPtr<Node> camera_node;
    SharedPtr<Node> drag_plane;
    SharedPtr<Camera> camera;

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
        paused = 0,
        playing_observer = 1,
        playing_phase2 = 2
    };

    static void RegisterObject(Context *context);

    LAFLogic(Context* ctx);

    void Setup();
    void StartScene(SceneInfo scene_info);
    void HandleUpdate(StringHash eventType, VariantMap& data);
    void HandleScreenChange(StringHash eventType, VariantMap& data);
private:

    void AddTargetElement(String itemtype,String groupName);
    void MoveCameraTarget(float delta);
    void SetCameraPos(float pos);

    bool IsStartScreenVisible();
    void ShowStartScreen(bool show);

    int TE_NextColor(SharedPtr<TargetElementComponent> te,bool animate=false);
    int TE_RandomColor(SharedPtr<TargetElementComponent> te,bool animate=false);
    void TE_SetColor(SharedPtr<TargetElementComponent> te,int colorIdx,bool animate=false);
    bool TE_CheckGoals(SharedPtr<TargetElementComponent> te);
    bool CheckSuccess();


    void StartPhase2();

    void AddProcess(std::function<bool(float)> proc);
    void ProcessLambdas(float dt);

    void OnShortClick();
    void OnLongClick();
    void OnDrag();
    void OnDragEnd();

    SharedPtr<TargetElementComponent> PickBehindDragElement(bool only_valid=true);

    void ProcessInput(float dt);

    std::function<bool(float)> CreateSwapAnimation(SharedPtr<TargetElementComponent> swapA,SharedPtr<TargetElementComponent> swapB,float speed);
    std::function<bool(float)> CreateMoveAnimation(SharedPtr<TargetElementComponent> swapA,Vector3 position,float speed);
    std::function<bool(float)> CreateColorAnimation(SharedPtr<TargetElementComponent> te,SharedPtr<Material> targetMaterial,float speed);



    SceneData sceneData;

    GameState gamestate = GameState::paused;
    Settings settings;

    SharedPtr<Scene>  scene;

    ClickCheck click_check;
    ActionDrag action_drag;

    Vector<std::function<bool(float)>> processes;

    SharedPtr<Node> initialSceneNode;
    SharedPtr<Node> levelNode;
    GameLogic* gl;
    SharedPtr<ResourceCache> resCache;
    SharedPtr<Input> input;
    HashMap<String,Vector<TargetElement>> target_elements;

};
