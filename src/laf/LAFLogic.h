#pragma once

#include <Urho3D/Urho3DAll.h>
#include "../GameLogic.h"

struct SceneInfo {
    String scene_name="";
    int variations=2;
    int remove_elements=1;

    SceneInfo(const String& scene_name,int variations,int remove_elements){
        this->scene_name=scene_name;
        this->variations=variations;
        this->remove_elements=remove_elements;
    }

    SceneInfo(){
    }
};

struct Settings {
    float camera_speed = 2.0f;
    Vector<String> color_materials = {"red","green","yellow","whitegrey"};
    String dragpoint_prefab = "Objects/col_default_targetpoint.xml";
    Vector<SceneInfo> scenes ={
        SceneInfo("Scenes/test_scene.xml",1,1),
        SceneInfo("Scenes/test_scene.xml",1,1)
    };
};

struct TargetElement {
    String ui_name;
    String type;
    SharedPtr<Node> node;
};

struct TEMatch {
    SharedPtr<Node> in_scene;
    SharedPtr<Node> drag_point_master;
    SharedPtr<Node> drag_point_rigidbody_node;
    SharedPtr<Node> in_laf;
    TEMatch(){};
    TEMatch(Node* in_laf,Node* in_scene,Node* drag_point_master,Node* drag_point_rigidbody_node){
        this->in_scene=in_scene;
        this->in_laf=in_laf;
        this->drag_point_master=drag_point_master;
        this->drag_point_rigidbody_node=drag_point_rigidbody_node;
    }
};

struct SceneData {
    SceneInfo scene_info;
    Vector3 camera_max_left;
    Vector3 camera_max_right;
    SharedPtr<Node> camera_lookat;
    SharedPtr<Node> camera_node;
    SharedPtr<Node> drag_plane;
    SharedPtr<Camera> camera;

    Vector<TargetElement> te_all;
    Vector<TargetElement> te_moved;
    Vector<Node> drag_points;

    float camera_target=0.5f;
};

struct LAFData {
    SharedPtr<Scene>  lafScene;
    SharedPtr<Viewport> lafViewport;
    SharedPtr<Node>   lafCameraNode;
    SharedPtr<Camera> lafCamera;
    Vector<TEMatch>   lafMatches;
    Vector3           lafStart;
    int               lafWidth;
};

struct ActionDrag {
    bool active=false;
    SharedPtr<Node> drag_node;
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

    void ShowLostAndFound(bool show, bool force=false);

    void TE_RandomColor(Node* colorItem);
    void StartPhase2();
    Node* CheckSuccess(Node* dragged_node, Node* drag_point,bool removeOnSuccess=true);

    void ProcessInput(float dt);
    void ProcessPickray(float dt);

    SceneData sceneData;
    LAFData lafData;

    GameState gamestate = GameState::paused;
    Settings settings;

    bool lost_and_found_visible = false;

    SharedPtr<Scene>  scene;

    ActionDrag action_drag;


    SharedPtr<Node> initialSceneNode;
    SharedPtr<Node> levelNode;
    GameLogic* gl;
    SharedPtr<ResourceCache> resCache;
    SharedPtr<Input> input;
    HashMap<String,Vector<TargetElement>> target_elements;

};
