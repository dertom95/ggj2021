#include "LAFLogic.h"
#include "../Components/GroupInstance.h"
#include "test_scene.h"
#include "start_scene.h"
#include "global_resources.h"

LAFLogic::LAFLogic(Context *ctx)
    : Object(ctx)
{}

void LAFLogic::AddTargetElement(String itemtype,String groupName)
{
    if (!target_elements.Contains(itemtype)){
        target_elements[itemtype]=Vector<TargetElement>();
    }
    target_elements[itemtype].Push((TargetElement){itemtype,groupName});
}

void LAFLogic::Setup()
{
    gl = GetSubsystem<GameLogic>();
    input = GetSubsystem<Input>();
    resCache = GetSubsystem<ResourceCache>();

    scene = gl->GetScene();
    scene->CreateComponent<PhysicsWorld>();

    levelNode = scene->CreateChild("level_node");

    initialSceneNode = scene->GetNode(res::scenes::start_scene::empties::initial_scene::id);

    lafData.lafScene = new Scene(context_);
    gl->LoadFromFile(res::global::scenes::lost_n_found_xml::path,lafData.lafScene);

    lafData.lafCamera = lafData.lafScene->GetComponent<Camera>(true);
    lafData.lafCameraNode = lafData.lafCamera->GetNode();
    auto node_lafData_start = gl->GetFirstChildWithTag(lafData.lafScene,"laf_start",true);
    if (node_lafData_start){
        lafData.lafStart = node_lafData_start->GetWorldPosition();
    }

    auto physicsWorld = scene->GetComponent<PhysicsWorld>(true);

    SubscribeToEvent(E_UPDATE,URHO3D_HANDLER(LAFLogic,HandleUpdate));
    SubscribeToEvent(E_SCREENMODE,URHO3D_HANDLER(LAFLogic,HandleScreenChange));
}



void LAFLogic::StartScene(SceneInfo scene_info)
{
    sceneData.scene_info = scene_info;
    gl->LoadFromFile(scene_info.scene_name,levelNode);

    using namespace res::scenes::test_scene;
    sceneData.camera_lookat = gl->GetFirstChildWithTag(levelNode,tags::camera_lookat::camera_lookat::name,true);
    if (!sceneData.camera_lookat){
        URHO3D_LOGERRORF("Scene:%s has no lookat object! creating one",sceneData.scene_info.scene_name.CString());
        sceneData.camera_lookat = levelNode->CreateChild("camera_look_at");
    }
    sceneData.camera_node = gl->GetFirstChildWithTag(levelNode,"camera",true);
    if (!sceneData.camera_node){
        URHO3D_LOGERRORF("Scene:%s has no camera set",scene_info.scene_name.CString());
    } else {
        gl->SetCameraNode(sceneData.camera_node);
        sceneData.camera = sceneData.camera_node->GetComponent<Camera>(true);
    }

    sceneData.drag_plane = gl->GetFirstChildWithTag(levelNode,"drag_plane",true);

    Node* temp = gl->GetFirstChildWithTag(levelNode,tags::camera_max_left::camera_max_left::name,true);
    if (temp){
        sceneData.camera_max_left = temp->GetWorldPosition();
        sceneData.camera_max_left.y_ = sceneData.camera_node->GetWorldPosition().y_;
    }
    temp = gl->GetFirstChildWithTag(levelNode,tags::camera_max_right::camera_max_right::name,true);
    if (temp){
        sceneData.camera_max_right = temp->GetWorldPosition();
        sceneData.camera_max_right.y_ = sceneData.camera_node->GetWorldPosition().y_;
    }

    PODVector<Node*> nodes;
    levelNode->GetChildrenWithTag(nodes,"target_element",true);
    for (Node* n : nodes){
        auto vType = n->GetVar("type");
        if (vType.IsEmpty()) {
            continue;
        }
        String type = vType.GetString();
        TargetElement te;
        te.type = type;
        te.node = n;
        if (type == "color"){
            TE_RandomColor(n);
        }
        auto vName = n->GetVar("name");
        te.ui_name = vName.IsEmpty() ? vName.GetString() : "unknown";
        sceneData.te_all.Push(te);
    }

    gamestate = GameState::playing_observer;
}

void LAFLogic::StartPhase2()
{
    ShowLostAndFound(true);
    lafData.lafMatches.Clear();

    Vector<TargetElement> tempAllTargetElements(sceneData.te_all);
    for (int i=0; i < sceneData.scene_info.remove_elements; i++){
        int idx = Random((int)tempAllTargetElements.Size());
        auto te_element_tobe_moved = tempAllTargetElements[idx];
        Node* current_te = te_element_tobe_moved.node;
        sceneData.te_moved.Push(te_element_tobe_moved);
        tempAllTargetElements.Erase(idx);

        auto drag_point_master = levelNode->CreateChild("drag_point");
        gl->LoadFromFile(settings.dragpoint_prefab,drag_point_master);
        drag_point_master->SetPosition(current_te->GetWorldPosition());
        auto drag_point_rigidnode = drag_point_master->GetComponent<RigidBody>(true)->GetNode();
        // create variants

        // same
        auto clone = current_te->Clone();

        // disable the one in the scene
        current_te->SetEnabled(false);

        lafData.lafMatches.Push(TEMatch(clone,current_te,drag_point_master,drag_point_rigidnode)); // this clone would match
        lafData.lafScene->AddChild(clone);
        clone->SetPosition(lafData.lafStart);
        clone->Rotate(Quaternion(0.f,0.f,-45.f));
        clone->Scale(0.25f);
    }


    gamestate = GameState::playing_phase2;
}

Node* LAFLogic::CheckSuccess(Node* dragged_node, Node* drag_point_riggid,bool remove_on_success)
{
    for (int i=0;i<lafData.lafMatches.Size();i++){
        const TEMatch& match = lafData.lafMatches[i];
        if (match.drag_point_rigidbody_node==drag_point_riggid){
            if (match.in_laf==dragged_node){
                Node* result = match.in_scene;
                if (remove_on_success){
                    match.drag_point_master->Remove();
                    match.in_laf->Remove();
                    lafData.lafMatches.Erase(i);
                }
                return result;
            }
        }
    }
    return nullptr;
}

void LAFLogic::SetCameraPos(float pos){
    auto cam_pos = sceneData.camera_max_left.Lerp(sceneData.camera_max_right,pos);
    sceneData.camera_node->SetPosition(cam_pos);
    sceneData.camera_node->LookAt(sceneData.camera_lookat->GetWorldPosition());
}

void LAFLogic::MoveCameraTarget(float dt){
    sceneData.camera_target = Clamp(sceneData.camera_target+dt*settings.camera_speed,0.0f,1.0f);
}

void LAFLogic::ShowStartScreen(bool show){
    initialSceneNode->SetEnabledRecursive(show);
}

bool LAFLogic::IsStartScreenVisible(){
    return initialSceneNode->IsEnabled();
}

void LAFLogic::ProcessInput(float dt){
    if (input->GetKeyPress(KEY_1)){
        StartScene(settings.scenes[0]);
    }
    else if (input->GetKeyPress(KEY_2)){
        StartScene(settings.scenes[1]);
    }
    else if (input->GetKeyPress(KEY_0)){
        ShowStartScreen(!IsStartScreenVisible());
    }
    else if (input->GetKeyPress(KEY_9)){
        ShowLostAndFound(!lost_and_found_visible);
    }
    else if (input->GetKeyPress(KEY_8)){
        StartPhase2();
    }

    if (gamestate > GameState::paused){

        if (input->GetKeyDown(KEY_LEFT)){
            MoveCameraTarget(dt);
        }
        else if (input->GetKeyDown(KEY_RIGHT)){
            MoveCameraTarget(-dt);
        }

        SetCameraPos(sceneData.camera_target);
    }

    if (gamestate == GameState::playing_phase2)
    {
        if (!action_drag.active){
            if (gl->IsMousePressedOrTouch()){
                Vector3 hitpos;
                Node* hitnode=nullptr;
                //if (gl->TouchOrMouseRaycast(10000.0f,hitpos,hitnode,"",gl->GetViewport())){
                if (gl->MouseOrTouchRaycast(10000.0f,hitpos,hitnode,"target_element",lafData.lafViewport)){
                    URHO3D_LOGINFOF("HIT: %s | %s",hitpos.ToString().CString(),hitnode->GetName().CString());
                    action_drag.active=true;
                    action_drag.drag_node=hitnode;
                    hitnode->SetParent(levelNode);
                    hitnode->SetScale(1.0f);
                    hitnode->SetRotation(Quaternion::IDENTITY);
                }
            }
        } else {
            if (!gl->IsMouseDownOrTouch()){
                // release
                action_drag.active=false;
                action_drag.drag_node=nullptr;
            } else {
                Vector3 hitpos;
                RigidBody* hitbody=nullptr;
                //if (gl->TouchOrMouseRaycast(10000.0f,hitpos,hitnode,"",gl->GetViewport())){

                if (gl->MouseOrTouchPhysicsRaycast(10000.0f,hitpos,hitbody,"drag_point")){
                    URHO3D_LOGINFOF("DRAGPOINT:%s",hitbody->GetNode()->GetName().CString());

                    Node* success_node = CheckSuccess(action_drag.drag_node,hitbody->GetNode());
                    if (success_node){
                        URHO3D_LOGINFOF("Success:%s",success_node->GetName().CString());
                        success_node->SetEnabled(true);
                    }

                    action_drag.position = hitpos;
                    action_drag.drag_node->SetPosition(hitpos);
                }
                else if (gl->MouseOrTouchPhysicsRaycast(10000.0f,hitpos,hitbody,"drag_plane")){
                    action_drag.position = hitpos;
                    action_drag.drag_node->SetPosition(hitpos);
                }
            }
        }
    }


}

void LAFLogic::ProcessPickray(float dt)
{
    //gl->MouseOrTouchPhysicsRaycast();
}

void LAFLogic::ShowLostAndFound(bool show, bool force){
    if (show == lost_and_found_visible && !force) return;

    auto* renderer = GetSubsystem<Renderer>();
    auto* graphics = GetSubsystem<Graphics>();

    if (show == true){
        renderer->SetNumViewports(2);
        // bottom
        /*        lafViewport = new Viewport(context_, lafScene, lafCamera,
                                    IntRect(0, graphics->GetHeight()-graphics->GetHeight()/5, graphics->GetWidth(), graphics->GetHeight()));*/

        // side

        lafData.lafWidth = (int)(graphics->GetHeight()/4);
        lafData.lafViewport = new Viewport(context_, lafData.lafScene, lafData.lafCamera,
                                    IntRect(0, 0, lafData.lafWidth, graphics->GetHeight()));
        renderer->SetViewport(1,lafData.lafViewport);
        lost_and_found_visible = true;
    } else {
        renderer->SetNumViewports(1);
        lost_and_found_visible = false;
    }
}

void LAFLogic::TE_RandomColor(Node *colorItem)
{
    auto model = colorItem->GetDerivedComponent<StaticModel>(true);
    String material_name = "Materials/"+settings.color_materials[Random((int)settings.color_materials.Size())]+".xml";
    auto random_material = resCache->GetResource<Material>(material_name);
    model->SetMaterial(random_material);
}

void LAFLogic::HandleUpdate(StringHash eventType, VariantMap& data)
{
    using namespace Update;
    float dt = data[P_TIMESTEP].GetFloat();

    ProcessInput(dt);
}

void LAFLogic::HandleScreenChange(StringHash eventType, VariantMap& data)
{
    using namespace ScreenMode;
    if (lost_and_found_visible){
        ShowLostAndFound(true,true);
    }
}
