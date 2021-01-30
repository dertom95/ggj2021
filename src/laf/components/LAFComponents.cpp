//
// Copyright (c) 2008-2019 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "LAFComponents.h"

#include <Urho3D/Urho3DAll.h>


TargetElementComponent::TargetElementComponent(Context* context) :
    Component(context)
{
}

void TargetElementComponent::RegisterObject(Context* context)
{
    context->RegisterFactory<TargetElementComponent>("LAF");

    // These macros register the class attributes to the Context for automatic load / save handling.
    // We specify the Default attribute mode which means it will be used both for saving into file, and network replication
    URHO3D_ATTRIBUTE("Color Switch", bool, color_switch, DEFAULT_COLOR_SWITCH, AM_FILE);
}

bool TargetElementComponent::IsColorOk()
{
    return current_color_idx == goal_color_idx;
}

bool TargetElementComponent::IsPositionOk()
{
    return GetNode()->GetWorldPosition().DistanceToPoint(GetGoalPosition())<0.01f;
}

bool TargetElementComponent::IsAllOk()
{
    bool isColorOk = IsColorOk();
    bool isPositionOk = IsPositionOk();
    return isColorOk && isPositionOk;
}




// ----------------------------------------------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------

TargetGroupComponent::TargetGroupComponent(Context* context) :
    Component(context)
{
}

void TargetGroupComponent::RegisterObject(Context* context)
{
    context->RegisterFactory<TargetGroupComponent>("LAF");

    // These macros register the class attributes to the Context for automatic load / save handling.
    // We specify the Default attribute mode which means it will be used both for saving into file, and network replication
    URHO3D_ATTRIBUTE("Swap Elements", bool, swap_elements, true, AM_FILE);
}


void TargetGroupComponent::AddTargetElement(TargetElementComponent *te)
{
    target_elements.Push(SharedPtr<TargetElementComponent>(te));
}
