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

// a simple rotator-component as test for exporting components as json
#pragma once

#include <Urho3D/Input/Controls.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Graphics/AnimationController.h>

using namespace Urho3D;

const bool DEFAULT_COLOR_SWITCH = true;

class TargetGroupComponent;

class TargetElementComponent : public Component
{
    URHO3D_OBJECT(TargetElementComponent, Component);

public:
    /// Construct.
    explicit TargetElementComponent(Context* context);

    /// Register object factory and attributes.
    static void RegisterObject(Context* context);

    inline bool IsColorSwitchPossible() { return color_switch;}

    inline void SetGroup(SharedPtr<TargetGroupComponent> grp){ group = grp;}
    inline SharedPtr<TargetGroupComponent> GetGroup() { return group; }

    inline void SetColorIndex(int idx) { current_color_idx=idx;}
    inline int GetColorIndex() { return current_color_idx;}

    inline void SetGoalColorIndex(int idx) { goal_color_idx=idx;}
    inline int GetGoalColorIndex() { return goal_color_idx;}

    inline void SetGoalPosition(const Vector3& vec) { goal_position = vec; }
    inline Vector3 GetGoalPosition() { return goal_position; }

    inline void SetLastPosition(const Vector3& vec) { last_position = vec; }
    inline Vector3 GetLastPosition() { return last_position; }

    inline void SetColorAnimated(bool isAnimated){color_animated=isAnimated;}
    inline bool IsColorAnimatedInProgress(){ return color_animated;}

    inline void SetPositionAnimated(bool isAnimated){trans_animated=isAnimated;}
    inline bool IsPositionAnimatedInProgress(){ return trans_animated;}

    bool IsColorOk();
    bool IsPositionOk();
    bool IsAllOk();
private:
    bool color_switch=true;
    SharedPtr<TargetGroupComponent> group;
    Vector3 goal_position; // the position this targetElement needs to have in the end
    Vector3 last_position; // the position this targetElement needs to have in the end
    int current_color_idx=-1;
    int goal_color_idx=-1;
    bool color_animated=false;
    bool trans_animated=false;
};

class TargetGroupComponent : public Component
{
    URHO3D_OBJECT(TargetGroupComponent, Component);

public:
    /// Construct.
    explicit TargetGroupComponent(Context* context);

    /// Register object factory and attributes.
    static void RegisterObject(Context* context);

    inline bool IsElementSwapAllowed() { return swap_elements; }
    inline Vector<SharedPtr<TargetElementComponent>> GetTargetElements() { return target_elements; }
    void AddTargetElement(TargetElementComponent* te);

private:
    bool swap_elements=true;
    Vector<SharedPtr<TargetElementComponent>> target_elements;
};
