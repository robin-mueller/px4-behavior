# Copyright 2024 Robin Müller
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

if(DEFINED _AUTO_APMS_BT_NODE_PLUGINS__RESOURCE_FILE)
    ament_index_register_resource("${_AUTO_APMS_BT_NODE_PLUGINS__RESOURCE_TYPE_NAME}" CONTENT "${_AUTO_APMS_BT_NODE_PLUGINS__RESOURCE_FILE}")
endif()
if(DEFINED _AUTO_APMS_BEHAVIOR_TREE__RESOURCE_FILE)
    ament_index_register_resource("${_AUTO_APMS_BEHAVIOR_TREE__RESOURCE_TYPE_NAME}" CONTENT "${_AUTO_APMS_BEHAVIOR_TREE__RESOURCE_FILE}")
endif()
