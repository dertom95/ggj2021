
#pragma once

#include <Urho3D/Core/Object.h>
#include "LAFComponents.h"
#include "../../Subsystems/LoaderTools/ComponentExporter.h"

using namespace Urho3D;

class LAFComponentsActivator{
public:
    static void RegisterComponents(Context* context);
    static void Export(Context* context_,String path);
};

void LAFComponentsActivator::RegisterComponents(Context *context)
{
    // mandataory for some exporter features
    TargetElementComponent::RegisterObject(context);
    TargetGroupComponent::RegisterObject(context);
}

void LAFComponentsActivator::Export(Context* context_,String path)
{
    Urho3DNodeTreeExporter exporter(context_);
    exporter.SetExportMode(Urho3DNodeTreeExporter::WhiteList);
    exporter.AddComponentHashToFilterList(TargetElementComponent::GetTypeStatic());
    exporter.AddComponentHashToFilterList(TargetGroupComponent::GetTypeStatic());
    exporter.Export(path,true,false);
}
