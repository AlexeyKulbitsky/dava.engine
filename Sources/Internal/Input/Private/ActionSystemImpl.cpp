#include "Input/Private/ActionSystemImpl.h"
#include "Input/InputSystem.h"
#include "Engine/Engine.h"
#include "DeviceManager/DeviceManager.h"

namespace DAVA
{
namespace Private
{
// Calculates number of states in array which are not empty
// Used for sorting
static size_t GetNonEmptyStatesCount(const Array<DigitalElementState, MAX_DIGITAL_STATES_COUNT>& array)
{
    size_t result = 0;
    for (const DigitalElementState& deviceControlState : array)
    {
        if (deviceControlState.elementId > 0)
        {
            ++result;
        }
    }

    return result;
}

bool DigitalBindingCompare::operator()(const DigitalBinding& first, const DigitalBinding& second) const
{
    return GetNonEmptyStatesCount(first.requiredStates) > GetNonEmptyStatesCount(second.requiredStates);
}

bool AnalogBindingCompare::operator()(const AnalogBinding& first, const AnalogBinding& second) const
{
    return GetNonEmptyStatesCount(first.requiredDigitalElementStates) > GetNonEmptyStatesCount(second.requiredDigitalElementStates);
}

ActionSystemImpl::ActionSystemImpl(ActionSystem* actionSystem)
    : actionSystem(actionSystem)
{
    GetEngineContext()->inputSystem->AddInputEventHandler(MakeFunction(this, &ActionSystemImpl::OnInputEvent));
}

ActionSystemImpl::~ActionSystemImpl()
{
    // TODO: unsubscribe
}

void ActionSystemImpl::BindSet(const ActionSet& set, Vector<uint32> devices)
{
    // Check if there are sets which are already binded to any of these devices
    // Unbind them from these devices if there are
    if (devices.size() > 0)
    {
        auto iter = bindedSets.begin();
        while (iter != bindedSets.end())
        {
            BindedActionSet& bindedSet = *iter;

            // If set is binded to to specific devices
            if (bindedSet.devices.size() > 0)
            {
                // Unbind it
                for (const uint32 deviceId : devices)
                {
                    bindedSet.devices.erase(
                    std::remove(bindedSet.devices.begin(), bindedSet.devices.end(), deviceId),
                    bindedSet.devices.end());
                }

                // If it is not binded to any devices anymore - remove it from the list
                if (bindedSet.devices.size() == 0)
                {
                    iter = bindedSets.erase(iter);
                    continue;
                }
            }

            ++iter;
        }
    }

    BindedActionSet bindedSet;
    bindedSet.digitalBindings.insert(set.digitalBindings.begin(), set.digitalBindings.end());
    bindedSet.analogBindings.insert(set.analogBindings.begin(), set.analogBindings.end());
    bindedSet.devices = devices;

    bindedSets.push_back(bindedSet);
}

void ActionSystemImpl::GetUserInput(Function<void(Vector<eInputElements>)> callback)
{
    userInputCallback = callback;
}

// Helper function to check if specified states are active
bool ActionSystemImpl::CheckDigitalStates(const Array<DigitalElementState, MAX_DIGITAL_STATES_COUNT>& states, const Vector<uint32>& devices)
{
    for (const DigitalElementState& requiredState : states)
    {
        if (requiredState.elementId == 0)
        {
            break;
        }

        bool requiredStateMatches = false;

        for (const uint32 deviceId : devices)
        {
            const InputDevice* device = GetEngineContext()->deviceManager->GetInputDevice(deviceId);
            if (device->SupportsElement(requiredState.elementId))
            {
                const eDigitalElementState state = device->GetDigitalElementState(requiredState.elementId);

                if ((state & requiredState.stateMask) == requiredState.stateMask)
                {
                    requiredStateMatches = true;
                    break;
                }
            }
        }

        if (!requiredStateMatches)
        {
            // At least one control is not in the state which is required, stop
            return false;
        }
    }

    return true;
}

bool ActionSystemImpl::OnInputEvent(const InputEvent& event)
{
    // If user requested user input, process it
    // Ignore action sets
    if (userInputCallback != nullptr)
    {
        const InputElementInfo& elementInfo = GetInputElementInfo(event.elementId);
        if (elementInfo.type == eInputElementType::DIGITAL)
        {
            if ((event.digitalState & eDigitalElementState::JUST_PRESSED) == eDigitalElementState::JUST_PRESSED)
            {
                listenedInputElements.push_back(event.elementId);
            }
            else if ((event.digitalState & eDigitalElementState::JUST_RELEASED) == eDigitalElementState::JUST_RELEASED)
            {
                if (listenedInputElements.size() != 0)
                {
                    userInputCallback(listenedInputElements);
                    listenedInputElements.clear();
                    userInputCallback = nullptr;
                }
            }

            return true;
        }

        return false;
    }

    // Handle binded sets

    for (const BindedActionSet& setBinding : bindedSets)
    {
        // Check if any digital action has triggered
        for (auto it = setBinding.digitalBindings.begin(); it != setBinding.digitalBindings.end(); ++it)
        {
            DigitalBinding const& binding = *it;

            const bool triggered = CheckDigitalStates(binding.requiredStates, setBinding.devices);

            if (triggered)
            {
                Action action;
                action.actionId = binding.actionId;
                action.analogState = binding.outputAnalogState;
                action.triggeredDeviceId = event.deviceId;

                actionSystem->ActionTriggered.Emit(action);

                return true;
            }
        }
    }

    for (const BindedActionSet& setBinding : bindedSets)
    {
        // Check if any analog action has triggered
        for (auto it = setBinding.analogBindings.begin(); it != setBinding.analogBindings.end(); ++it)
        {
            AnalogBinding const& binding = *it;

            if (event.elementId != binding.analogElementId)
            {
                continue;
            }

            const bool triggered = CheckDigitalStates(binding.requiredDigitalElementStates, setBinding.devices);

            if (triggered)
            {
                Action action;
                action.actionId = binding.actionId;
                action.analogState = event.analogState;
                action.triggeredDeviceId = event.deviceId;

                actionSystem->ActionTriggered.Emit(action);

                return true;
            }
        }
    }

    return false;
}
}
}