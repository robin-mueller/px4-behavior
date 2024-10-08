// Copyright 2024 Robin Müller
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "auto_apms/behavior_tree/behavior_tree.hpp"

namespace auto_apms {

BT::Tree CreateBehaviorTree(BTNodePluginLoader& node_plugin_loader,
                            const std::string& tree_xml,
                            BT::BehaviorTreeFactory* const factory_ptr,
                            BT::Blackboard::Ptr parent_blackboard_ptr)
{
    // Check if factory pointer is valid
    if (!factory_ptr) throw std::runtime_error("factory_ptr is nullptr");

    // Create empty blackboard if none was provided
    if (!parent_blackboard_ptr) parent_blackboard_ptr = BT::Blackboard::create();

    node_plugin_loader.Load(*factory_ptr);

    // The main_tree_to_execute attribute will be used to determine the entry point
    return factory_ptr->createTreeFromText(tree_xml, parent_blackboard_ptr);
}

BT::Tree CreateBehaviorTree(rclcpp::Node::SharedPtr node_ptr,
                            const BehaviorTreeResource& resource,
                            BT::BehaviorTreeFactory* const factory_ptr,
                            BT::Blackboard::Ptr parent_blackboard_ptr)
{
    BTNodePluginLoader loader{node_ptr, BTNodePluginLoader::Manifest::FromFiles(resource.node_manifest_paths)};
    return CreateBehaviorTree(loader, BehaviorTreeXML{resource}.WriteToString(), factory_ptr, parent_blackboard_ptr);
}

}  // namespace auto_apms