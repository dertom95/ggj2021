#include "LAFLogic.h"
#include "../Components/GroupInstance.h"
#include "../Subsystems/LoaderTools/ComponentExporter.h"
#include "components/ComponentsActivator.h"
#include "components/LAFComponents.h"

#include "test_scene.h"
#include "start_scene.h"
#include "global_resources.h"

LAFLogic::LAFLogic(Context *ctx)
    : Object(ctx)
{}

void LAFLogic::Setup()
{
    LAFComponentsActivator::RegisterComponents(context_);
    LAFComponentsActivator::Export(context_,"laf_components.json");

    gl = GetSubsystem<GameLogic>();
    input = GetSubsystem<Input>();
    resCache = GetSubsystem<ResourceCache>();

    scene = gl->GetScene();
    scene->CreateComponent<PhysicsWorld>();

    levelNode = scene->CreateChild("level_node");

    initialSceneNode = scene->GetNode(res::scenes::start_scene::empties::initial_scene::id);

    auto physicsWorld = scene->GetComponent<PhysicsWorld>(true);

    gl->PlayMusic("Denkspiel_Neu_auf_Harfe.ogg");


    SubscribeToEvent(E_UPDATE,URHO3D_HANDLER(LAFLogic,HandleUpdate));
    SubscribeToEvent(E_SCREENMODE,URHO3D_HANDLER(LAFLogic,HandleScreenChange));

    SetupUI();
}

void LAFLogic::NextLevel(){
    int next_id = settings.current_level+1;
    if (next_id >= settings.scenes.Size()) {
        next_id = settings.current_level;
    }
    StartScene(next_id);
}

void LAFLogic::SetupUI()
{
    auto mUiRoot = GetSubsystem<UI>()->GetRoot();

    uiData.root_start = new UIElement(context_);
    mUiRoot->AddChild(uiData.root_start);

    // Create the Window and add it to the UI's root node
    // Load XML file containing default UI style sheet
    auto* cache = GetSubsystem<ResourceCache>();
    auto* style = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
    // Set the loaded style as default style
    mUiRoot->SetDefaultStyle(style);

    auto button = new Button(context_);
    uiData.btnEasy = button;
    uiData.root_start->AddChild(button);
    button->SetSize(300,200);
    button->SetColor(Color(0.5f,0.5f,0.5f,1.0f));
    button->SetStyleAuto();

    auto text = new Text(context_);
    text->SetStyleAuto();
    text->SetText("EASY");
    text->SetAlignment(HorizontalAlignment::HA_CENTER,VerticalAlignment::VA_CENTER);
    text->SetColor(Color::WHITE);
    text->SetSize(300,200);
    button->AddChild(text);


    button = new Button(context_);
    uiData.btnMedium = button;
    uiData.root_start->AddChild(button);
    button->SetSize(300,200);
    button->SetColor(Color(0.5f,0.5f,0.5f,1.0f));
    button->SetStyleAuto();

    auto text1 = new Text(context_);
    text1->SetStyleAuto();
    text1->SetText("MEDIUM");
    text1->SetAlignment(HorizontalAlignment::HA_CENTER,VerticalAlignment::VA_CENTER);
    text1->SetColor(Color::WHITE);
    text1->SetSize(300,200);
    button->AddChild(text1);

    button = new Button(context_);
    uiData.btnHard = button;
    uiData.root_start->AddChild(button);
    button->SetSize(300,200);
    button->SetColor(Color(0.5f,0.5f,0.5f,1.0f));
    button->SetStyleAuto();

    auto text2 = new Text(context_);
    text2->SetStyleAuto();
    text2->SetText("HARD");
    text2->SetAlignment(HorizontalAlignment::HA_CENTER,VerticalAlignment::VA_CENTER);
    text2->SetColor(Color::WHITE);
    text2->SetSize(300,200);
    button->AddChild(text2);




    // ingame
    uiData.root_ingame = new UIElement(context_);
    mUiRoot->AddChild(uiData.root_ingame);

    // restart
    button = new Button(context_);
    uiData.btnRestartLevel = button;
    uiData.root_ingame->AddChild(button);
    button->SetSize(200,90);
    button->SetColor(Color(0.5f,0.5f,0.5f,1.0f));
    button->SetStyleAuto();

    auto txtBtnRestart = new Text(context_);
    txtBtnRestart->SetStyleAuto();
    txtBtnRestart->SetText("Restart");
    txtBtnRestart->SetAlignment(HorizontalAlignment::HA_CENTER,VerticalAlignment::VA_CENTER);
    txtBtnRestart->SetColor(Color::WHITE);
    button->AddChild(txtBtnRestart);

    // start
    button = new Button(context_);
    uiData.btnBottomRight = button;
    uiData.root_ingame->AddChild(button);
    button->SetSize(200,90);
    button->SetColor(Color(0.5f,0.5f,0.5f,1.0f));
    button->SetStyleAuto();

    auto txtBtnStart = new Text(context_);
    uiData.txtBtnBottomRight = txtBtnStart;
    txtBtnStart->SetStyleAuto();
    txtBtnStart->SetText("Start");
    txtBtnStart->SetAlignment(HorizontalAlignment::HA_CENTER,VerticalAlignment::VA_CENTER);
    txtBtnStart->SetColor(Color::WHITE);
    button->AddChild(txtBtnStart);

    // window
    auto window = new Window(context_);
    uiData.hint_window = window;
    uiData.root_ingame->AddChild(window);

    auto txtHint = new Text(context_);
    uiData.hint = txtHint;
    window->AddChild(txtHint);
    txtHint->SetText("This is a hint!");
    txtHint->SetStyleAuto();
    txtHint->SetAlignment(HorizontalAlignment::HA_CENTER,VerticalAlignment::VA_CENTER);

    // window
    auto windowInstructions = new Window(context_);
    windowInstructions->SetSize(450,200);
    uiData.instructions_window = windowInstructions;
    uiData.root_ingame->AddChild(windowInstructions);

    auto instructions = new Text(context_);
    uiData.txtInstructions=instructions;
    windowInstructions->AddChild(instructions);
    instructions->SetText("\nInstructions!\n-Observe items(color/position)!\n-Start Level\n- reorganize to former state\n- Long-Click: grab/drag\n- Short-Click: Swap Color\n- Arrows-Keys: Rotate");
    instructions->SetStyleAuto();
    //instructions->SetAlignment(HorizontalAlignment::HA_CENTER,VerticalAlignment::VA_CENTER);



    mUiRoot->SetStyleAuto();
    window->SetStyleAuto();
    windowInstructions->SetStyleAuto();
    text->SetFontSize(50);
    text1->SetFontSize(50);
    text2->SetFontSize(50);
    txtBtnRestart->SetFontSize(25);
    txtHint->SetFontSize(25);
    txtBtnStart->SetFontSize(25);
    instructions->SetFontSize(15);






    SubscribeToEvent(E_UIMOUSECLICK, URHO3D_HANDLER(LAFLogic, HandleUI));

    LayoutUI();
}

void LAFLogic::LayoutUI()
{
    auto graphics = GetSubsystem<Graphics>();
    float dx = graphics->GetWidth() / 100.0f;
    float dy = graphics->GetHeight() / 100.0f;

    uiData.root_start->SetVisible(false);
    uiData.root_ingame->SetVisible(false);

    int screenWidth = graphics->GetWidth();
    int screenHeight = graphics->GetHeight();

    if (ui_state==UIState::init_scene){
        ShowStartScreen(true);
        uiData.root_start->SetVisible(true);

//        uiData.btnEasy->SetVisible(true);
//        uiData.btnHard->SetVisible(true);
//        uiData.btnMedium->SetVisible(true);

        float btnWidth=Min(300,(int)(25*dx));
        float btnHeight=Min(200,(int)(15*dy));
        uiData.btnEasy->SetSize(btnWidth,btnHeight);
        uiData.btnEasy->SetPosition(dx*10,graphics->GetHeight()-dy*40);

        uiData.btnMedium->SetSize(btnWidth,btnHeight);
        uiData.btnMedium->SetPosition(dx*40,graphics->GetHeight()-dy*40);

        uiData.btnHard->SetSize(btnWidth,btnHeight);
        uiData.btnHard->SetPosition(dx*70,graphics->GetHeight()-dy*40);
    } else {
        ShowStartScreen(false);
        uiData.root_ingame->SetVisible(true);

        float btnWidth=Min(200,(int)(15*dx));
        float btnHeight=Min(90,(int)(7*dy));
        uiData.btnRestartLevel->SetSize(btnWidth,btnHeight);
        uiData.btnRestartLevel->SetPosition(graphics->GetWidth()-btnWidth-2*dx,dy*5);

        float instructionWidth = 25*dx;
        float instructionHeight = 15*dx;
        uiData.instructions_window->SetSize(instructionWidth,instructionHeight);
        uiData.instructions_window->SetPosition(dx,screenHeight-instructionHeight);
        uiData.instructions_window->SetVisible(settings.current_level==0);

        if (gamestate==GameState::playing_observer){
            uiData.btnBottomRight->SetVisible(true);
            uiData.btnBottomRight->SetSize(btnWidth,btnHeight+2*dy);
            uiData.btnBottomRight->SetPosition(graphics->GetWidth()-btnWidth-2*dx,screenHeight - btnHeight - dy*5);
            uiData.txtBtnBottomRight->SetText("Start");
        }
        else if (gamestate==GameState::success){
            uiData.btnBottomRight->SetVisible(true);
            uiData.btnBottomRight->SetSize(btnWidth,btnHeight+2*dy);
            uiData.btnBottomRight->SetPosition(graphics->GetWidth()-btnWidth-2*dx,screenHeight - btnHeight - dy*5);
            uiData.txtBtnBottomRight->SetText("Next");
        }
        else {
            uiData.btnBottomRight->SetVisible(false);
        }


        uiData.hint_window->SetVisible(true);

        if (screenWidth < 2000) {
            uiData.hint->SetFontSize(14);
            uiData.txtInstructions->SetFontSize(14);
        } else {
            uiData.hint->SetFontSize(25);
            uiData.txtInstructions->SetFontSize(25);
        }

        uiData.hint->SetText("Level "+String((int)settings.current_level+1)+"/"+String(settings.scenes.Size())+" - "+sceneData.scene_info.hint);
        uiData.hint_window->SetPosition(dx*20,dy*2);
        uiData.hint_window->SetSize(dx*60,dy*9);


    }
}

void LAFLogic::OnSuccess(){
    gamestate=GameState::success;
    LayoutUI();
    //gl->PlaySound("Denkspiel_Neu_auf_Harfe.ogg");
}

void LAFLogic::StartScene(int idx)
{
    //gl->PlayMusic("Denkspiel_Neu_auf_Harfe.ogg");

    processes.Clear();
    SetGameState(GameState::playing_observer);


    auto scene_info = settings.scenes[idx];
    settings.current_level = idx;
    settings.best_level = Max(idx,settings.best_level);
    ui_state = UIState::ingame;

    sceneData = SceneData();
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

    ProcessTES(levelNode,sceneData.scene_info.shuffle);

    LayoutUI();
}

void LAFLogic::ProcessTES(Node* start_node,bool shuffle)
{
    // process TARGET-ELEMENTS
    PODVector<TargetElementComponent*> target_elements;
    start_node->GetComponents<TargetElementComponent>(target_elements,true);
    for (TargetElementComponent* te : target_elements){
        int colorIdx=-1;
        if (te->IsColorSwitchPossible()){
            colorIdx = TE_RandomColor(SharedPtr<TargetElementComponent>(te));
            te->SetGoalColorIndex(colorIdx);

            auto model = te->GetNode()->GetComponent<StaticModel>(true);
            model->SetMaterial(model->GetMaterial()->Clone());
        }

        te->GetNode()->AddTag("target_element");

        auto te_group = te->GetNode()->GetParentComponent<TargetGroupComponent>(true);
        if (te_group){
            te->SetGroup(SharedPtr<TargetGroupComponent>(te_group));
            te_group->AddTargetElement(te);
        }

        sceneData.te_all.Push(SharedPtr<TargetElementComponent>(te));

        te->SetColorIndex(colorIdx);
        te->SetGoalPosition(te->GetNode()->GetWorldPosition());
        te->SetLastPosition(te->GetNode()->GetWorldPosition());
    }

    // process TARGET-GROUPS
    PODVector<TargetGroupComponent*> target_groups;
    start_node->GetComponents<TargetGroupComponent>(target_groups,true);
    for (TargetGroupComponent* tg : target_groups){
        sceneData.targetGroups.Push(SharedPtr<TargetGroupComponent>(tg));
        if (tg->IsElementSwapAllowed()){
            sceneData.targetSwapGroups.Push(SharedPtr<TargetGroupComponent>(tg));
        }

        if (shuffle && tg->GetTargetElements().Size()>1){
            Vector<SharedPtr<TargetElementComponent>> tmpGroupTElements(tg->GetTargetElements());

            int idx1 = Random((int)tmpGroupTElements.Size());
            auto swapA = tmpGroupTElements[idx1];
            tmpGroupTElements.Erase(idx1);

            int idx2 = Random((int)tmpGroupTElements.Size());
            auto swapB = tmpGroupTElements[idx2];
            tmpGroupTElements.Erase(idx2);

            Vector3 posA = swapA->GetNode()->GetWorldPosition();
            Vector3 posB = swapB->GetNode()->GetWorldPosition();
            swapA->GetNode()->SetWorldPosition(posB);
            swapB->GetNode()->SetWorldPosition(posA);

            swapA->SetGoalPosition(posB);
            swapA->SetLastPosition(posB);
            swapB->SetGoalPosition(posA);
            swapB->SetLastPosition(posA);
        }
    }

}

void LAFLogic::StartPhase2()
{
    Vector<SharedPtr<TargetElementComponent>> tempAllTargetElements(sceneData.te_all);
    Vector<SharedPtr<TargetGroupComponent>> tempAllTargetSwapGroups(sceneData.targetSwapGroups);

    // toggle colors
    for (int i=0; i < sceneData.scene_info.color_toggles; i++){
        int idx = Random((int)tempAllTargetElements.Size());
        SharedPtr<TargetElementComponent> te_element_tobe_recolored(tempAllTargetElements[idx]);
        TE_RandomColor(te_element_tobe_recolored,false,true,sceneData.scene_info.show_pause);
        tempAllTargetElements.Erase(idx);
    }

    // swap
    for (unsigned i=0,created_swaps=0; created_swaps < sceneData.scene_info.swaps; i++){
        int idx = Random((int)tempAllTargetSwapGroups.Size());
        auto tg_swap = tempAllTargetSwapGroups[idx];

        if (tg_swap->GetTargetElements().Size()<2) {
            continue;
        }

        Vector<SharedPtr<TargetElementComponent>> tmpGroupTElements(tg_swap->GetTargetElements());
        int idx1 = Random((int)tmpGroupTElements.Size());
        auto swapA = tmpGroupTElements[idx1];
        tmpGroupTElements.Erase(idx1);

        int idx2 = Random((int)tmpGroupTElements.Size());
        auto swapB = tmpGroupTElements[idx2];
        tmpGroupTElements.Erase(idx2);

        AddProcess(CreateSwapAnimation(swapA,swapB,sceneData.scene_info.swap_speed),true,sceneData.scene_info.show_pause);

        created_swaps++;
    }

    AddProcess(CreateSequence(sceneData.process_sequence));
    sceneData.process_sequence.Clear();


    SetGameState(GameState::playing_phase2);
    LayoutUI();
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

void LAFLogic::OnShortClick(){
    URHO3D_LOGINFO("SHORT CLICK");
    Vector3 hitpos;
    RigidBody* hitbody;
    if (gl->MouseOrTouchPhysicsRaycast(10000.0f,hitpos,hitbody,"target_element")){
        SharedPtr<TargetElementComponent> te(hitbody->GetNode()->GetComponent<TargetElementComponent>(true));
        if (te){
            TE_NextColor(te,true);
        } else {
            URHO3D_LOGERRORF("No TargetElementComponent at node:%s",hitbody->GetNode()->GetName().CString());
        }
    }
}

void LAFLogic::OnLongClick(){
    URHO3D_LOGINFO("LongClickStart");
    Vector3 hitpos;
    RigidBody* hitbody;
    if (gl->MouseOrTouchPhysicsRaycast(10000.0f,hitpos,hitbody,"target_element")){
        SharedPtr<TargetElementComponent> te(hitbody->GetNode()->GetComponent<TargetElementComponent>(true));
        if (te->GetGroup()){
            action_drag.drag_te = te;
            // move drag_plane to the groups_layer
            auto position_drag_plane = sceneData.drag_plane->GetWorldPosition();
            auto position_te = te->GetNode()->GetWorldPosition();
            position_drag_plane.x_=position_te.x_;
            sceneData.drag_plane->SetWorldPosition(position_drag_plane);
            action_drag.active=true;
            te->GetNode()->RemoveTag("target_element");
        } else {
            action_drag.drag_te = nullptr;
            action_drag.active=false;
        }
    } else {
        action_drag.drag_te = nullptr;
        action_drag.active=false;
    }
}

SharedPtr<TargetElementComponent> LAFLogic::PickBehindDragElement(bool only_valid)
{
    Vector3 hitpos;
    RigidBody* hitbody;

    if (gl->MouseOrTouchPhysicsRaycast(10000.0f,hitpos,hitbody,"target_element")){
        SharedPtr<TargetElementComponent> behind_te (hitbody->GetNode()->GetComponent<TargetElementComponent>(true));

        auto behind_te_group = behind_te->GetGroup();
        if (behind_te_group && behind_te_group==action_drag.drag_te->GetGroup()){
            // same group!
            URHO3D_LOGINFO("SAME GROUP!");
            return behind_te;
        } else if (!only_valid){
            return behind_te;
        }
        URHO3D_LOGINFOF("HIT TE:%s",behind_te->GetNode()->GetName().CString());
    } else {
        URHO3D_LOGINFOF("NO HIT TE");
    }
    return nullptr;
}


void LAFLogic::OnDrag(){

    if (action_drag.active && action_drag.drag_te){
        URHO3D_LOGINFO("DRAG");
        Vector3 hitpos;
        RigidBody* hitbody;
        if (gl->MouseOrTouchPhysicsRaycast(10000.0f,hitpos,hitbody,"drag_plane")){
            action_drag.drag_te->GetNode()->SetWorldPosition(hitpos);
        }

        auto behind_te = PickBehindDragElement();
        if (behind_te){
            URHO3D_LOGINFOF("Valid behind:%s",behind_te->GetNode()->GetName().CString());
        }

//        Vector3 pos = sceneData.camera->ScreenToWorldPoint(input->GetMouseMoveX())
//        action_drag.drag_node
    }

}

void LAFLogic::OnDragEnd(){
    URHO3D_LOGINFO("DragEnd");

    if (!action_drag.active || !action_drag.drag_te){
        return;
    }

    auto drag_te = action_drag.drag_te;
    auto behind_te = PickBehindDragElement();

    if (behind_te){
        URHO3D_LOGINFOF("Valid behind:%s",behind_te->GetNode()->GetName().CString());
        AddProcess(CreateMoveAnimation(behind_te,drag_te->GetLastPosition(),settings.ingame_swap_speed));

        drag_te->GetNode()->AddTag("target_element");
        drag_te->GetNode()->SetWorldPosition(behind_te->GetNode()->GetWorldPosition());
        drag_te->SetLastPosition(behind_te->GetNode()->GetWorldPosition());
    } else {
        AddProcess(CreateMoveAnimation(action_drag.drag_te,action_drag.drag_te->GetLastPosition(),settings.ingame_swap_speed));
        drag_te->GetNode()->AddTag("target_element");
    }

}

void LAFLogic::ProcessInput(float dt){
    if (gamestate==GameState::playing_observer && input->GetKeyPress(KEY_P)){
        NextLevel();
    }
//    else if (input->GetKeyPress(KEY_2)){
//        StartScene(1);
//    }
//    else

    if (input->GetKeyPress(KEY_0)){
        ShowStartScreen(!IsStartScreenVisible());
    }
    else if (input->GetKeyPress(KEY_9)){
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
        if (gl->IsMouseDownOrTouch()){
            click_check.wasDown=true;
            if (!action_drag.active){
                click_check.downTimer+=dt;
                if (click_check.downTimer >= settings.long_press_time){
                    OnLongClick();
                }
            } else {
                OnDrag();
            // drag
            }
        } else {
            if (click_check.wasDown){
                if (action_drag.active){
                    OnDragEnd();
                } else {
                    OnShortClick();
                }
                action_drag.active=false;
                click_check.wasDown=false;
                click_check.downTimer=0.0f;
            }
        }
    }


}



int LAFLogic::TE_NextColor(SharedPtr<TargetElementComponent> te,bool animate,bool anim_in_sequence,float pause)
{
    int color_idx = te->GetColorIndex();
    color_idx++;
    if (color_idx >= settings.color_materials.Size()){
        color_idx=0;
    }
    TE_SetColor(te,color_idx,animate,anim_in_sequence,pause);
    return color_idx;
}

bool LAFLogic::CheckSuccess()
{
    for (auto te : sceneData.te_all){
        if (!te->IsAllOk()) {
            return false;
        }
    }
    return true;
}

bool LAFLogic::TE_CheckGoals(SharedPtr<TargetElementComponent> te)
{
    URHO3D_LOGINFOF("Check[%s]: color-ok:%d position-ok:%d all-ok:%d",te->GetNode()->GetName().CString(),te->IsColorOk(),te->IsPositionOk(),te->IsAllOk());
    return te->IsAllOk();
}

int LAFLogic::TE_RandomColor(SharedPtr<TargetElementComponent> te,bool animate,bool anim_in_sequence,float pause)
{
    int selection = te->GetColorIndex();
    while (selection == te->GetColorIndex()){
        selection = Random((int)settings.color_materials.Size());
    }
    TE_SetColor(te,selection,animate,anim_in_sequence,pause);

    return selection;
}

void LAFLogic::TE_SetColor(SharedPtr<TargetElementComponent> te,int color_idx,bool animate,bool anim_in_sequence,float pause)
{
    auto model = te->GetNode()->GetDerivedComponent<StaticModel>(true);
    te->SetColorIndex(color_idx);

    TE_CheckGoals(te);

    String material_name = "Materials/"+settings.color_materials[color_idx]+".xml";
    SharedPtr<Material> color_material(resCache->GetResource<Material>(material_name));
    URHO3D_LOGINFOF("SetColorIdx:%i",color_idx);
    if (animate||anim_in_sequence){
        float anim_speed = gamestate==GameState::playing_phase2
                                ? settings.ingame_color_toggle_speed
                                :sceneData.scene_info.color_toggle_speed;
        AddProcess(CreateColorAnimation(te,color_material,anim_speed),anim_in_sequence,pause);
    } else {
        model->SetMaterial(color_material->Clone());
    }
}

String GetGameState(LAFLogic::GameState gs){
    if (gs==LAFLogic::GameState::init_scene){
        return "init_scene";
    }
    if (gs==LAFLogic::GameState::paused){
        return "paused";
    }
    if (gs==LAFLogic::GameState::playing_observer){
        return "playing_observer";
    }
    if (gs==LAFLogic::GameState::playing_phase2){
        return "playing_phase2";
    }
    if (gs==LAFLogic::GameState::success){
        return "success";
    }
    return "unknown";
}

void LAFLogic::SetGameState(GameState state){
    if (gamestate==state){
        return;
    }
    gamestate=state;
    requestState=state;
}

void LAFLogic::HandleUpdate(StringHash eventType, VariantMap& data)
{
    using namespace Update;
    float dt = data[P_TIMESTEP].GetFloat();

    if (gamestate!=requestState){
        gamestate=requestState;
        LayoutUI();
    }

    if (gamestate==GameState::init_scene){
        if (!init_scene_initialized){
            sceneData = SceneData();
            ProcessTES(scene);
            init_scene_initialized=true;
        }
        if (processes.Size()==0){
            Vector<SharedPtr<TargetElementComponent>> tempTES(sceneData.te_all);
            int idx = Random((int)tempTES.Size());
            SharedPtr<TargetElementComponent> te1 = tempTES[idx];
            tempTES.Erase(idx);
            idx = Random((int)tempTES.Size());
            SharedPtr<TargetElementComponent> te2 = tempTES[idx];
            tempTES.Erase(idx);
            idx = Random((int)tempTES.Size());
            SharedPtr<TargetElementComponent> te3 = tempTES[idx];
            tempTES.Erase(idx);
            idx = Random((int)tempTES.Size());
            SharedPtr<TargetElementComponent> te4 = tempTES[idx];
            tempTES.Erase(idx);
            idx = Random((int)tempTES.Size());
            SharedPtr<TargetElementComponent> te5 = tempTES[idx];
            tempTES.Erase(idx);
            idx = Random((int)tempTES.Size());
            SharedPtr<TargetElementComponent> te6 = tempTES[idx];
            tempTES.Erase(idx);
            idx = Random((int)tempTES.Size());
            SharedPtr<TargetElementComponent> te7 = tempTES[idx];
            tempTES.Erase(idx);

            AddProcess(CreateSwapAnimation(te1,te2,0.25f+Random()));
            AddProcess(CreateSwapAnimation(te3,te5,0.25f+Random()));
            TE_RandomColor(te1,true);
            TE_RandomColor(te3,true);
            TE_RandomColor(te5,true);
            TE_RandomColor(te6,true);
            TE_RandomColor(te7,true);
            AddProcess(CreatePause(6.0f));
        }
    }

    ProcessInput(dt);
    ProcessLambdas(dt);

    if (gamestate==GameState::playing_phase2 && CheckSuccess()){
        OnSuccess();
    }


    URHO3D_LOGINFOF("SUCCESS? :%s state:%s",CheckSuccess()?"True":"false",GetGameState(gamestate).CString());
}

void LAFLogic::HandleUI(StringHash eventType, VariantMap& data)
{
    if (eventType==E_UIMOUSECLICK){
        using namespace UIMouseClick;
        auto v_ui_elem = data[P_ELEMENT];
        auto ui_elem = v_ui_elem.GetPtr();

        if (ui_state == UIState::init_scene){
            if (ui_elem==uiData.btnEasy){
                URHO3D_LOGINFO("EASY");
                StartScene(settings.scene_start_easy);
            } else if (ui_elem==uiData.btnMedium){
                URHO3D_LOGINFO("MEDIUM");
                StartScene(settings.scene_start_medium);
            } else if (ui_elem==uiData.btnHard){
                URHO3D_LOGINFO("HARD");
                StartScene(settings.scene_start_hard);
            }
        }
        else if (ui_state==UIState::ingame){
            if (ui_elem==uiData.btnRestartLevel){
                //NextLevel();
                StartScene(settings.current_level);
            }
            else if (ui_elem==uiData.btnBottomRight){
                if (gamestate==GameState::playing_observer){
                    StartPhase2();
                }
                else if (gamestate==GameState::success){
                    NextLevel();
                }
            }
        }
    }
}


void LAFLogic::HandleScreenChange(StringHash eventType, VariantMap& data)
{
    LayoutUI();
}
