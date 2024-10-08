// Copyright 2024 Robin Müller
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <ament_index_cpp/get_package_share_directory.hpp>
#include <auto_apms_examples/msg/contingency_event.hpp>
#include <chrono>
#include <definitions.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <px4_msgs/msg/vehicle_global_position.hpp>
#include <rclcpp/rclcpp.hpp>

#define NODE_NAME "flight_recorder"
#define PARAM_NAME_SAMPLE_INTERVAL "sample_interval_ms"

using VehiclePositionMsg = px4_msgs::msg::VehicleGlobalPosition;
using ContingencyEventMsg = auto_apms_examples::msg::ContingencyEvent;

namespace auto_apms::ops_engine {

struct RecordSample
{
    std::shared_ptr<VehiclePositionMsg> position_ptr;
    std::shared_ptr<ContingencyEventMsg> event_ptr;

    void reset()
    {
        position_ptr.reset();
        event_ptr.reset();
    }
    operator bool() const { return position_ptr && event_ptr; }
};

class FlightRecorderNode : public rclcpp::Node
{
   public:
    FlightRecorderNode(const rclcpp::NodeOptions& options);
    ~FlightRecorderNode();

   private:
    rclcpp::Subscription<VehiclePositionMsg>::SharedPtr position_sub_ptr_;
    rclcpp::Subscription<ContingencyEventMsg>::SharedPtr event_sub_ptr_;
    rclcpp::TimerBase::SharedPtr sample_timer_ptr_;

    std::filesystem::path save_path_;
    RecordSample sample_;
    std::map<std::chrono::system_clock::time_point, RecordSample> record_;
};

FlightRecorderNode::FlightRecorderNode(const rclcpp::NodeOptions& options) : Node{NODE_NAME, options}
{
    // Sample interval parameter
    this->declare_parameter(PARAM_NAME_SAMPLE_INTERVAL, 250);
    auto sample_interval = std::chrono::milliseconds(this->get_parameter(PARAM_NAME_SAMPLE_INTERVAL).as_int());

    // Path name for recording file
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::string datetime_str{std::ctime(&now)};
    datetime_str.erase(std::remove(datetime_str.begin(), datetime_str.end(), '\n'), datetime_str.cend());
    std::replace(datetime_str.begin(), datetime_str.end(), ' ', '_');
    save_path_ = std::filesystem::path(ament_index_cpp::get_package_share_directory("auto_apms")) / NODE_NAME /
                 (datetime_str + ".csv");

    // Record data subscribers
    position_sub_ptr_ = this->create_subscription<VehiclePositionMsg>(
        "/fmu/out/vehicle_global_position",
        rclcpp::QoS(1).best_effort(),
        [this](std::unique_ptr<VehiclePositionMsg> msg) { sample_.position_ptr = std::move(msg); });
    event_sub_ptr_ = this->create_subscription<ContingencyEventMsg>(
        std::string(this->get_namespace()) + CONTINGENCY_EVENT_TOPIC_NAME,
        10,
        [this](std::unique_ptr<ContingencyEventMsg> msg) { sample_.event_ptr = std::move(msg); });

    sample_timer_ptr_ = this->create_wall_timer(sample_interval, [this]() {
        // If no event was received, set to undefined
        if (!sample_.event_ptr) {
            ContingencyEventMsg undefined_event_msg;
            undefined_event_msg.event_id = ContingencyEventMsg::EVENT_UNDEFINED;
            sample_.event_ptr = std::make_shared<ContingencyEventMsg>(undefined_event_msg);
        }

        // Skip sampling when information is not fully available
        if (!sample_) { return; }

        // Store and reset sample
        record_[std::chrono::system_clock::now()] = sample_;
        sample_.reset();
    });

    RCLCPP_INFO(this->get_logger(), "Recording with %lims sample time...", sample_interval.count());
}

FlightRecorderNode::~FlightRecorderNode()
{
    if (record_.empty()) {
        RCLCPP_WARN(this->get_logger(), "Did not assemble any data, so nothing will be written");
        return;
    }

    RCLCPP_INFO(this->get_logger(), "Saving flight record to '%s'", save_path_.c_str());
    auto dir = save_path_.parent_path();
    std::filesystem::create_directories(dir);
    std::ofstream out_file{save_path_};
    if (!out_file.is_open()) {
        RCLCPP_ERROR(this->get_logger(), "Failed to open out_file");
        return;
    }

    // Header
    out_file << "time,lon,lat,alt,event_id\n";

    auto double_to_string = [](double num, int decimals) {
        std::stringstream stream;
        stream << std::fixed << std::setprecision(decimals) << num;
        return stream.str();
    };

    // Body
    for (const auto& rec : record_) {
        out_file << std::chrono::duration_cast<std::chrono::milliseconds>(rec.first.time_since_epoch()).count() << ','
                 << double_to_string(rec.second.position_ptr->lon, 14) << ','
                 << double_to_string(rec.second.position_ptr->lat, 14) << ','
                 << double_to_string(rec.second.position_ptr->alt, 14) << ','
                 << std::to_string(rec.second.event_ptr->event_id) << '\n';
    }

    out_file.close();
}

}  // namespace auto_apms::ops_engine

#include "rclcpp_components/register_node_macro.hpp"
RCLCPP_COMPONENTS_REGISTER_NODE(auto_apms::ops_engine::FlightRecorderNode)
