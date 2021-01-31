#include "LAFLogic.h"
#include "components/LAFComponents.h"

ProcessFunction LAFLogic::CreateColorAnimation(SharedPtr<TargetElementComponent> te,SharedPtr<Material> targetMaterial,float speed)
{
    if (te->IsColorAnimatedInProgress()){
        return nullptr;
    }
    te->SetColorAnimated(true);

    auto staticModel = te->GetNode()->GetComponent<StaticModel>(true);
    SharedPtr<Material> current_mat = SharedPtr<Material>(staticModel->GetMaterial());
    if (!current_mat){
        staticModel->SetMaterial(targetMaterial->Clone());
        current_mat=targetMaterial;
    }
    auto matname=current_mat->GetName().CString();
    auto destinationColor = targetMaterial->GetShaderParameter("MatDiffColor").GetColor();
    auto currentColor = current_mat->GetShaderParameter("MatDiffColor").GetColor();
    auto ctx = new ProcessCtxProgress();

    return [currentColor,destinationColor,current_mat,speed,te,ctx](float dt){
        ctx->progress += speed*dt;


        Color newColor = currentColor.Lerp(destinationColor,ctx->progress);
        bool finished = false;
        URHO3D_LOGINFOF("%f",ctx->progress);
        if (ctx->progress>1.0f){
            newColor = destinationColor;
            finished = true;
            te->SetColorAnimated(false);
            delete ctx;
        }
        current_mat->SetShaderParameter("MatDiffColor",newColor);
        return !finished;
    };
}


ProcessFunction LAFLogic::CreatePause(float secs)
{
    auto ctx = new ProcessCtxProgress();
    ctx->progress = secs;

    return [ctx](float dt){
        ctx->progress -= dt;
        if (ctx->progress <= 0) {
            delete ctx;
            return false;
        }
        return true;
    };
}


ProcessFunction LAFLogic::CreateSequence(Vector<ProcessFunction> sequence)
{
    auto ctx = new ProcessCtxGroup();
    ctx->group = sequence;

    return [ctx](float dt){
        if (ctx->group.Size()==0) {
            delete ctx;
            return false;
        }
        auto current = ctx->group[0];
        bool keep_playing = current(dt);
        if (!keep_playing) {
            ctx->group.Erase(0);
        }
        return true;
    };
}



ProcessFunction LAFLogic::CreateSwapAnimation(SharedPtr<TargetElementComponent> swapA,SharedPtr<TargetElementComponent> swapB,float speed){
//    if (swapA->IsPositionAnimatedInProgress() || swapB->IsPositionAnimatedInProgress()){
//        return nullptr;
//    }

    Vector3 destB = swapA->GetNode()->GetWorldPosition();
    Vector3 destA = swapB->GetNode()->GetWorldPosition();


    auto anim1 = CreateMoveAnimation(swapA,destA,speed);
    auto anim2 = CreateMoveAnimation(swapB,destB,speed);

    ProcessCtxProgress* ctxA = new ProcessCtxProgress();
    ProcessCtxProgress* ctxB = new ProcessCtxProgress();

    return [ctxA,ctxB,swapA,swapB,speed](float dt){

        ctxA->progress += speed*dt;
        ctxB->progress += speed*dt;

        if (ctxA->to==Vector3::ZERO){
            ctxA->from=swapA->GetNode()->GetWorldPosition();
            ctxA->to=swapB->GetNode()->GetWorldPosition();
        }
        if (ctxB->to==Vector3::ZERO){
            ctxB->from=swapB->GetNode()->GetWorldPosition();
            ctxB->to=swapA->GetNode()->GetWorldPosition();
        }

        Vector3 newPosA = ctxA->from.Lerp(ctxA->to,ctxA->progress);
        Vector3 newPosB = ctxB->from.Lerp(ctxB->to,ctxB->progress);
        bool finishedA=false;
        bool finishedB=false;
        if (ctxA->progress>=1.0f){
            newPosA = ctxA->to;
            swapA->SetPositionAnimated(false);
            swapA->SetLastPosition(newPosA);
            finishedA=true;
        }
        swapA->GetNode()->SetWorldPosition(newPosA);

        if (ctxB->progress>=1.0f){
            newPosB = ctxB->to;
            swapB->SetPositionAnimated(false);
            swapB->SetLastPosition(newPosB);
            finishedB=true;
        }
        swapB->GetNode()->SetWorldPosition(newPosB);

        bool finishedAll = (finishedA && finishedB);
        if (finishedAll){
            delete ctxA;
            delete ctxB;
        }

        return !(finishedAll);
    };
}

ProcessFunction LAFLogic::CreateMoveAnimation(SharedPtr<TargetElementComponent> swapA,Vector3 destination,float speed)
{
    if (swapA->IsPositionAnimatedInProgress()){
        return nullptr;
    }

    swapA->SetPositionAnimated(true);

    auto ctx = new ProcessCtxProgress();

    Vector3 currentPosA = swapA->GetNode()->GetWorldPosition();
    return [currentPosA,destination,speed,swapA,ctx](float dt){
        if (swapA->GetNode()->GetWorldPosition()==destination){
            return false;
        }
        ctx->progress += speed*dt;
        Vector3 newPosA = currentPosA.Lerp(destination,ctx->progress);
        bool finishedA=false;
        if (ctx->progress>=1.0f){
            newPosA = destination;
            swapA->SetPositionAnimated(false);
            swapA->SetLastPosition(destination);
            delete ctx;
            finishedA=true;
        }
        swapA->GetNode()->SetWorldPosition(newPosA);

        return !finishedA;
    };
}

void LAFLogic::AddProcess(ProcessFunction proc,bool add_to_sseq,float pause)
{
    if (proc){
        if (add_to_sseq){
            sceneData.process_sequence.Push(proc);
            if (pause>0.0f){
                sceneData.process_sequence.Push(CreatePause(pause));
            }
        } else {
            processes.Push(proc);
            if (pause>0.0f){
                processes.Push(CreatePause(pause));
            }
        }
    }
}


void LAFLogic::ProcessLambdas(float dt)
{
    for (int i=processes.Size()-1;i>=0;i--){
        bool keepRunning = processes[i](dt);
        if (!keepRunning){
            processes.Erase(i);
        }
    }
}

