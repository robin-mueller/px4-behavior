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

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

#include "auto_apms/resource/node.hpp"
#include "behaviortree_cpp/xml_parsing.h"
#include "rcpputils/split.hpp"

int main(int argc, char** argv)
{
    using namespace auto_apms;

    if (argc < 3) {
        std::cerr
            << "generate_node_model: Missing inputs! The program requires: \n\t1.) the yaml configuration files to "
               "pass to auto_apms::RegisterBTNodePlugins (separated by ';')\n\t2.) the xml file to store the "
               "model\n";
        std::cerr << "Usage: generate_node_model <registration_config_files> <output_file>\n";
        return EXIT_FAILURE;
    }
    std::vector<std::string> registration_config_files = rcpputils::split(argv[1], ';');
    std::filesystem::path output_file{std::filesystem::absolute(argv[2])};

    // Ensure that arguments are not empty
    if (registration_config_files.empty()) {
        throw std::runtime_error("Argument registration_config_files must not be empty");
    }
    if (output_file.empty()) { throw std::runtime_error("Argument output_file must not be empty"); }

    // Ensure correct extensions
    if (output_file.extension().compare(".xml") != 0) {
        throw std::runtime_error("Output file '" + output_file.string() + "' has wrong extension. Must be '.xml'");
    }

    rclcpp::init(argc, argv);
    auto node = std::make_shared<rclcpp::Node>("_generate_node_model_temp_node");

#ifdef DEBUG_LOGGING
    // Set logging severity
    auto ret = rcutils_logging_set_logger_level(node->get_logger().get_name(), RCUTILS_LOG_SEVERITY_DEBUG);
    if (ret != RCUTILS_RET_OK) {
        RCLCPP_ERROR(node->get_logger(), "Error setting severity: %s", rcutils_get_error_string().str);
        rcutils_reset_error();
    }
#endif

    BT::BehaviorTreeFactory factory;

    // Register all plugins specified in the plugin registration config files
    if (!RegisterBTNodePlugins(node, factory, registration_config_files)) {
        std::cerr << "generate_node_model: Error registering node plugins\n";
        return EXIT_FAILURE;
    }

    // Write the behavior tree model file
    std::ofstream out_stream{output_file};
    if (out_stream.is_open()) {
        out_stream << BT::writeTreeNodesModelXML(factory);
        out_stream.close();
    }
    else {
        std::cerr << "generate_node_model: Error opening tree nodes model output file '" << output_file << "'\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}