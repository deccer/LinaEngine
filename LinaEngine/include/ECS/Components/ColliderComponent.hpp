/*
Author: Inan Evin
www.inanevin.com

Copyright 2018 Inan Evin

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, 
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions 
and limitations under the License.

Class: ColliderComponent
Timestamp: 4/30/2019 7:40:43 PM

*/

#pragma once

#ifndef ColliderComponent_HPP
#define ColliderComponent_HPP

#include "Utility/Math/AABB.hpp"


namespace LinaEngine::ECS
{
	struct ColliderComponent
	{
		AABB aabb;
	};
}


#endif