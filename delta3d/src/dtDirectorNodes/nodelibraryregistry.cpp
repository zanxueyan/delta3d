/*
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2008 MOVES Institute
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Author: Jeff P. Houde
 */

#include <dtDirector/nodetype.h>
#include <dtDirectorNodes/nodelibraryregistry.h>

// Events
#include <dtDirectorNodes/inputnode.h>
#include <dtDirectorNodes/remoteevent.h>
#include <dtDirectorNodes/triggervolumeevent.h>
#include <dtDirectorNodes/gamemessageevent.h>
#include <dtDirectorNodes/gameeventmessageevent.h>

// Actions
#include <dtDirectorNodes/outputnode.h>
#include <dtDirectorNodes/referencescriptaction.h>
#include <dtDirectorNodes/callremoteeventaction.h>
#include <dtDirectorNodes/setvalueaction.h>
#include <dtDirectorNodes/getactorpropertyaction.h>
#include <dtDirectorNodes/setactorpropertyaction.h>
#include <dtDirectorNodes/operationaction.h>
#include <dtDirectorNodes/comparevalueaction.h>
#include <dtDirectorNodes/delayaction.h>
#include <dtDirectorNodes/toggleaction.h>
#include <dtDirectorNodes/scheduleraction.h>
#include <dtDirectorNodes/lerpactorrotationaction.h>
#include <dtDirectorNodes/lerpactortranslationaction.h>
#include <dtDirectorNodes/sendmessageaction.h>
#include <dtDirectorNodes/sendeventmessageaction.h>

// Values
#include <dtDirectorNodes/externalvaluenode.h>
#include <dtDirectorNodes/booleanvalue.h>
#include <dtDirectorNodes/intvalue.h>
#include <dtDirectorNodes/floatvalue.h>
#include <dtDirectorNodes/doublevalue.h>
#include <dtDirectorNodes/stringvalue.h>
#include <dtDirectorNodes/stringarrayvalue.h>
#include <dtDirectorNodes/vecvalue.h>
#include <dtDirectorNodes/referencevalue.h>
#include <dtDirectorNodes/actorvalue.h>
#include <dtDirectorNodes/playervalue.h>
#include <dtDirectorNodes/staticmeshvalue.h>


using dtCore::RefPtr;

namespace dtDirector
{
   // Category naming convention:
   //  Core        - All Core nodes are nodes that are specifically referenced
   //                in Director and are special cases.
   //
   //  General     - General nodes provide general functionality that can be used
   //                in most, if not all, script types.
   //
   //  Value Ops   - Value Operation nodes are any nodes that perform an operation
   //                on values.
   //
   //  Conditional - Conditional nodes have multiple outputs that get triggered
   //                when a condition is met.
   //
   //  Cinematic   - Cinematic nodes are nodes that are auto-generated by the
   //                cinematic editor tool.

   // Events
   RefPtr<NodeType> NodeLibraryRegistry::INPUT_NODE_TYPE(                        new dtDirector::NodeType(dtDirector::NodeType::LINK_NODE,   "Input Link",                "Core",        "Links",       "This node creates an input link connector in its parent graph."));
   RefPtr<NodeType> NodeLibraryRegistry::REMOTE_EVENT_NODE_TYPE(                 new dtDirector::NodeType(dtDirector::NodeType::EVENT_NODE,  "Remote Event",              "Core",        "Events",      "A remote event."));
   RefPtr<NodeType> NodeLibraryRegistry::TRIGGER_VOLUME_EVENT_NODE_TYPE(         new dtDirector::NodeType(dtDirector::NodeType::EVENT_NODE,  "Trigger Volume Event",      "General",     "Events",      "A trigger volume event."));
   RefPtr<NodeType> NodeLibraryRegistry::GAME_MESSAGE_EVENT_NODE_TYPE(           new dtDirector::NodeType(dtDirector::NodeType::EVENT_NODE,  "Game Message Event",        "Messages",    "Messages",    "An event that responds to game messages."));
   RefPtr<NodeType> NodeLibraryRegistry::GAME_EVENT_MESSAGE_EVENT_NODE_TYPE(     new dtDirector::NodeType(dtDirector::NodeType::EVENT_NODE,  "Game Event Message Event",  "Messages",    "Messages",    "An event that responds to game event messages."));

   // Actions
   RefPtr<NodeType> NodeLibraryRegistry::OUTPUT_NODE_TYPE(                       new dtDirector::NodeType(dtDirector::NodeType::LINK_NODE,   "Output Link",               "Core",        "Links",       "This node creates an output link connector in its parent graph."));
   RefPtr<NodeType> NodeLibraryRegistry::REFERENCE_SCRIPT_ACTION_NODE_TYPE(      new dtDirector::NodeType(dtDirector::NodeType::MACRO_NODE,  "Reference Script",          "Core",        "Macros",      "References an external Director script resource."));
   RefPtr<NodeType> NodeLibraryRegistry::CALL_REMOTE_EVENT_ACTION_NODE_TYPE(     new dtDirector::NodeType(dtDirector::NodeType::ACTION_NODE, "Call Remote Event",         "Core",        "Actions",     "Calls all remote event nodes with a given name in parallel, and waits until their entire chains are finished before continuing."));
   RefPtr<NodeType> NodeLibraryRegistry::DELAY_ACTION_NODE_TYPE(                 new dtDirector::NodeType(dtDirector::NodeType::ACTION_NODE, "Delay",                     "General",     "Actions",     "Performs a time delay."));
   RefPtr<NodeType> NodeLibraryRegistry::SET_VALUE_ACTION_NODE_TYPE(             new dtDirector::NodeType(dtDirector::NodeType::ACTION_NODE, "Set Value",                 "General",     "Operations",  "Copies the value of the Source value into the Destination value."));
   RefPtr<NodeType> NodeLibraryRegistry::GET_ACTOR_PROPERTY_ACTION_NODE_TYPE(    new dtDirector::NodeType(dtDirector::NodeType::ACTION_NODE, "Get Actor Property",        "General",     "Operations",  "Retrieves the value of a property from an actor."));
   RefPtr<NodeType> NodeLibraryRegistry::SET_ACTOR_PROPERTY_ACTION_NODE_TYPE(    new dtDirector::NodeType(dtDirector::NodeType::ACTION_NODE, "Set Actor Property",        "General",     "Operations",  "Sets the value of a property from an actor."));
   RefPtr<NodeType> NodeLibraryRegistry::OPERATION_ACTION_NODE_TYPE(             new dtDirector::NodeType(dtDirector::NodeType::ACTION_NODE, "Binary Operation",          "General",     "Operations",  "Performs a simple operation between two values A and B and outputs to Result."));
   RefPtr<NodeType> NodeLibraryRegistry::TOGGLE_ACTION_NODE_TYPE(                new dtDirector::NodeType(dtDirector::NodeType::ACTION_NODE, "Toggle",                    "General",     "Operations",  "Sets a boolean value."));
   RefPtr<NodeType> NodeLibraryRegistry::COMPARE_VALUE_ACTION_NODE_TYPE(         new dtDirector::NodeType(dtDirector::NodeType::ACTION_NODE, "Compare Value",             "General",     "Condition",   "Compares two values A and B."));
   RefPtr<NodeType> NodeLibraryRegistry::SCHEDULER_ACTION_NODE_TYPE(             new dtDirector::NodeType(dtDirector::NodeType::ACTION_NODE, "Scheduler",                 "Cinematic",   "Cinematic",   "Schedules a sequence of timed events."));
   RefPtr<NodeType> NodeLibraryRegistry::LERP_ACTOR_ROTATION_ACTION_NODE_TYPE(   new dtDirector::NodeType(dtDirector::NodeType::ACTION_NODE, "Lerp Actor Rotation",       "Cinematic",   "Cinematic",   "Linearly interpolates the rotation of an actor."));
   RefPtr<NodeType> NodeLibraryRegistry::LERP_ACTOR_TRANSLATION_ACTION_NODE_TYPE(new dtDirector::NodeType(dtDirector::NodeType::ACTION_NODE, "Lerp Actor Translation",    "Cinematic",   "Cinematic",   "Linearly interpolates the position of an actor."));
   RefPtr<NodeType> NodeLibraryRegistry::SEND_MESSAGE_ACTION_NODE_TYPE(          new dtDirector::NodeType(dtDirector::NodeType::ACTION_NODE, "Send Message Action",       "Messages",    "Messages",    "Sends a Game Message."));
   RefPtr<NodeType> NodeLibraryRegistry::SEND_EVENT_MESSAGE_ACTION_NODE_TYPE(    new dtDirector::NodeType(dtDirector::NodeType::ACTION_NODE, "Send Event Message Action", "Messages",    "Messages",    "Sends a Game Event Message."));

   // Values
   RefPtr<NodeType> NodeLibraryRegistry::EXTERNAL_VALUE_NODE_TYPE(               new dtDirector::NodeType(dtDirector::NodeType::LINK_NODE,   "Value Link",                "Core",        "Links",       "This node creates a value link connector in its parent graph."));
   RefPtr<NodeType> NodeLibraryRegistry::BOOLEAN_VALUE_NODE_TYPE(                new dtDirector::NodeType(dtDirector::NodeType::VALUE_NODE,  "Boolean",                   "General",     "Variables",   "A boolean value."));
   RefPtr<NodeType> NodeLibraryRegistry::INT_VALUE_NODE_TYPE(                    new dtDirector::NodeType(dtDirector::NodeType::VALUE_NODE,  "Int",                       "General",     "Variables",   "An integer value."));
   RefPtr<NodeType> NodeLibraryRegistry::FLOAT_VALUE_NODE_TYPE(                  new dtDirector::NodeType(dtDirector::NodeType::VALUE_NODE,  "Float",                     "General",     "Variables",   "A float value."));
   RefPtr<NodeType> NodeLibraryRegistry::DOUBLE_VALUE_NODE_TYPE(                 new dtDirector::NodeType(dtDirector::NodeType::VALUE_NODE,  "Double",                    "General",     "Variables",   "A double value."));
   RefPtr<NodeType> NodeLibraryRegistry::STRING_VALUE_NODE_TYPE(                 new dtDirector::NodeType(dtDirector::NodeType::VALUE_NODE,  "String",                    "General",     "Variables",   "A string value."));
   RefPtr<NodeType> NodeLibraryRegistry::STRING_ARRAY_VALUE_NODE_TYPE(           new dtDirector::NodeType(dtDirector::NodeType::VALUE_NODE,  "String Array",              "General",     "Variables",   "An array of string values."));
   RefPtr<NodeType> NodeLibraryRegistry::VEC_VALUE_NODE_TYPE(                    new dtDirector::NodeType(dtDirector::NodeType::VALUE_NODE,  "Vector",                    "General",     "Variables",   "A vector value."));
   RefPtr<NodeType> NodeLibraryRegistry::REFERENCE_VALUE_NODE_TYPE(              new dtDirector::NodeType(dtDirector::NodeType::VALUE_NODE,  "Reference",                 "Core",        "Variables",  "This will reference another value in the script."));
   RefPtr<NodeType> NodeLibraryRegistry::ACTOR_VALUE_NODE_TYPE(                  new dtDirector::NodeType(dtDirector::NodeType::VALUE_NODE,  "Actor",                     "General",     "Actors",      "An actor value."));
   RefPtr<NodeType> NodeLibraryRegistry::PLAYER_VALUE_NODE_TYPE(                 new dtDirector::NodeType(dtDirector::NodeType::VALUE_NODE,  "Player",                    "Core",        "Actors",      "This player actor."));
   RefPtr<NodeType> NodeLibraryRegistry::STATIC_MESH_VALUE_NODE_TYPE(            new dtDirector::NodeType(dtDirector::NodeType::VALUE_NODE,  "Static Mesh",               "General",     "Resources",   "A Static Mesh resource value."));


   //////////////////////////////////////////////////////////////////////////
   extern "C" NODE_LIBRARY_EXPORT dtDirector::NodePluginRegistry* CreatePluginRegistry()
   {
      return new NodeLibraryRegistry;
   }

   //////////////////////////////////////////////////////////////////////////
   extern "C" NODE_LIBRARY_EXPORT void DestroyPluginRegistry(dtDirector::NodePluginRegistry* registry)
   {
      delete registry;
   }

   //////////////////////////////////////////////////////////////////////////
   NodeLibraryRegistry::NodeLibraryRegistry()
      : dtDirector::NodePluginRegistry("dtDirectorNodes", "Core set of nodes that can be used anywhere.")
   {
   }

   //////////////////////////////////////////////////////////////////////////
   void NodeLibraryRegistry::RegisterNodeTypes()
   {
      // Events
      mNodeFactory->RegisterType<InputNode>(INPUT_NODE_TYPE.get());
      mNodeFactory->RegisterType<RemoteEvent>(REMOTE_EVENT_NODE_TYPE.get());
      mNodeFactory->RegisterType<TriggerVolumeEvent>(TRIGGER_VOLUME_EVENT_NODE_TYPE.get());
      mNodeFactory->RegisterType<GameMessageEvent>(GAME_MESSAGE_EVENT_NODE_TYPE.get());
      mNodeFactory->RegisterType<GameEventMessageEvent>(GAME_EVENT_MESSAGE_EVENT_NODE_TYPE.get());

      // Actions
      mNodeFactory->RegisterType<OutputNode>(OUTPUT_NODE_TYPE.get());
      mNodeFactory->RegisterType<ReferenceScriptAction>(REFERENCE_SCRIPT_ACTION_NODE_TYPE.get());
      mNodeFactory->RegisterType<CallRemoteEventAction>(CALL_REMOTE_EVENT_ACTION_NODE_TYPE.get());
      mNodeFactory->RegisterType<DelayAction>(DELAY_ACTION_NODE_TYPE.get());
      mNodeFactory->RegisterType<SetValueAction>(SET_VALUE_ACTION_NODE_TYPE.get());
      mNodeFactory->RegisterType<GetActorPropertyAction>(GET_ACTOR_PROPERTY_ACTION_NODE_TYPE.get());
      mNodeFactory->RegisterType<SetActorPropertyAction>(SET_ACTOR_PROPERTY_ACTION_NODE_TYPE.get());
      mNodeFactory->RegisterType<OperationAction>(OPERATION_ACTION_NODE_TYPE.get());
      mNodeFactory->RegisterType<ToggleAction>(TOGGLE_ACTION_NODE_TYPE.get());
      mNodeFactory->RegisterType<CompareValueAction>(COMPARE_VALUE_ACTION_NODE_TYPE.get());
      mNodeFactory->RegisterType<SchedulerAction>(SCHEDULER_ACTION_NODE_TYPE.get());
      mNodeFactory->RegisterType<LerpActorRotationAction>(LERP_ACTOR_ROTATION_ACTION_NODE_TYPE.get());
      mNodeFactory->RegisterType<LerpActorTranslationAction>(LERP_ACTOR_TRANSLATION_ACTION_NODE_TYPE.get());
      mNodeFactory->RegisterType<SendMessageAction>(SEND_MESSAGE_ACTION_NODE_TYPE.get());
      mNodeFactory->RegisterType<SendEventMessageAction>(SEND_EVENT_MESSAGE_ACTION_NODE_TYPE.get());

      // Values
      mNodeFactory->RegisterType<ExternalValueNode>(EXTERNAL_VALUE_NODE_TYPE.get());
      mNodeFactory->RegisterType<BooleanValue>(BOOLEAN_VALUE_NODE_TYPE.get());
      mNodeFactory->RegisterType<IntValue>(INT_VALUE_NODE_TYPE.get());
      mNodeFactory->RegisterType<FloatValue>(FLOAT_VALUE_NODE_TYPE.get());
      mNodeFactory->RegisterType<DoubleValue>(DOUBLE_VALUE_NODE_TYPE.get());
      mNodeFactory->RegisterType<StringValue>(STRING_VALUE_NODE_TYPE.get());
      mNodeFactory->RegisterType<StringArrayValue>(STRING_ARRAY_VALUE_NODE_TYPE.get());
      mNodeFactory->RegisterType<VecValue>(VEC_VALUE_NODE_TYPE.get());
      mNodeFactory->RegisterType<ReferenceValue>(REFERENCE_VALUE_NODE_TYPE.get());
      mNodeFactory->RegisterType<ActorValue>(ACTOR_VALUE_NODE_TYPE.get());
      mNodeFactory->RegisterType<PlayerValue>(PLAYER_VALUE_NODE_TYPE.get());
      mNodeFactory->RegisterType<StaticMeshValue>(STATIC_MESH_VALUE_NODE_TYPE.get());
   }
}
