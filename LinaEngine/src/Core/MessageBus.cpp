/*
This file is a part of: Lina Engine
https://github.com/inanevin/LinaEngine

Author: Inan Evin
http://www.inanevin.com

Copyright (c) [2018-2020] [Inan Evin]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "Core/MessageBus.hpp"
#include "EventSystem/EventSystem.hpp"
#include "Core/PhysicsBackend.hpp"
#include "Rendering/Model.hpp"

namespace Lina
{
	void MessageBus::Initialize(ApplicationMode appMode)
	{
		auto* eventSystem = Event::EventSystem::Get();
		m_appMode = appMode;
		eventSystem->Connect<Event::EResourceLoadCompleted, &MessageBus::OnResourceLoadCompleted>(this);

	}

	void CookModelNodeVertices(Graphics::ModelNode& node, Graphics::Model& model)
	{
		std::vector<Vector3> vertices;

		if (node.m_meshIndexes.size() > 0)
		{
			// Get model meshes.
			auto& meshes = model.GetMeshes();

			// For each mesh attach to this node.
			for (uint32 i = 0; i < node.m_meshIndexes.size(); i++)
			{
				// Get the mesh & it's vertex position data.
				auto& mesh = meshes[node.m_meshIndexes[i]];
				auto& data = mesh.GetVertexPositions();

				// Add each of the 3 positions to the vertices array.
				for (uint32 j = 0; j < data.m_floatElements.size(); j += 3)
					vertices.push_back(Vector3(data.m_floatElements[j], data.m_floatElements[j + 1], data.m_floatElements[j + 2]));
			}

			// Now we have a vertex vector, that contains all the vertices from a model node's all meshes.
			// This is the combined vertex data from sub-meshes.
			// Now we cook this data & assign a model & node id to it.
			Physics::PhysicsEngineBackend::Get()->CookConvexMesh(vertices, model.GetParameters().m_convexMeshData[node.m_nodeID], model.GetID(), node.m_nodeID);
		}

		for (auto& child : node.m_children)
			CookModelNodeVertices(child, model);
	}
	void MessageBus::OnResourceLoadCompleted(Event::EResourceLoadCompleted ev)
	{
		// Handle convex mesh cooking if the loaded model does not contain any.
		if (ev.m_type == Resources::ResourceType::Model)
		{
			auto& model = Graphics::Model::GetModel(ev.m_sid);
			auto& meshes = model.GetMeshes();

			// If the model parameters does not contain a convex mesh data for the current mesh and if we are in editor mode.
			if (model.GetParameters().m_convexMeshData.size() == 0)
			{
				if (m_appMode == Lina::ApplicationMode::Editor)
				{

					Graphics::ModelNode& root = model.GetRoot();
					model.SaveParameters(model.GetParamsPath(), model.GetParameters());

				}
				else
					LINA_ERR("You are running in Standalone mode but your loaded models does not contain any convex mesh data. This might result in inaccurate collision simulation! {0}", typeid(*this).name());
			}
			else
			{
				//auto& convesMeshes = model.GetParameters().m_convexMeshData;
				//// Create convex mesh based on cooked data.
				//for (auto& convexMeshPair : convesMeshes)
				//{
				//	Physics::PhysicsEngineBackend::Get()->CreateConvexMesh(convexMeshPair.second, model.GetID(), convexMeshPair.first);
				//}
			}

		}
	}
}