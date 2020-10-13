/*
Author: Inan Evin
www.inanevin.com
https://github.com/inanevin/LinaEngine

Copyright 2020~ Inan Evin

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions
and limitations under the License.

Class: ComponentDrawer
Timestamp: 10/13/2020 2:34:33 PM

*/


#include "Drawers/ComponentDrawer.hpp"
#include "ECS/ECSComponent.hpp"
#include "ECS/Components/TransformComponent.hpp"
#include "ECS/Components/CameraComponent.hpp"
#include "ECS/Components/LightComponent.hpp"
#include "ECS/Components/FreeLookComponent.hpp"
#include "ECS/Components/MeshRendererComponent.hpp"
#include "ECS/Components/RigidbodyComponent.hpp"
#include "imgui/imgui.h"
#include "IconsFontAwesome5.h"
#include "Widgets/WidgetsUtility.hpp"

using namespace LinaEngine::ECS;
using namespace LinaEditor;

namespace LinaEditor
{
	std::map<entt::id_type, std::function<void(LinaEngine::ECS::ECSRegistry*, LinaEngine::ECS::ECSEntity)>> ComponentDrawer::s_componentDrawFuncMap;
	int ComponentDrawer::s_currentCollisionShape = 0;

	void ComponentDrawer::RegisterComponentFunctions()
	{
		s_componentDrawFuncMap[entt::type_info<TransformComponent>::id()] = std::bind(&TransformComponent::COMPONENT_DRAWFUNC, std::placeholders::_1, std::placeholders::_2);
		s_componentDrawFuncMap[entt::type_info<RigidbodyComponent>::id()] = std::bind(&RigidbodyComponent::COMPONENT_DRAWFUNC, std::placeholders::_1, std::placeholders::_2);
		s_componentDrawFuncMap[entt::type_info<CameraComponent>::id()] = std::bind(&CameraComponent::COMPONENT_DRAWFUNC, std::placeholders::_1, std::placeholders::_2);
	}

	// Use reflection for gods sake later on.
	std::vector<std::string> ComponentDrawer::GetEligibleComponents(LinaEngine::ECS::ECSRegistry* reg, LinaEngine::ECS::ECSEntity entity)
	{
		std::vector<std::string> eligibleTypes;

		if (!reg->has<TransformComponent>(entity))
			eligibleTypes.push_back("Transformation");
		if (!reg->has<MeshRendererComponent>(entity))
			eligibleTypes.push_back("MeshRenderer");
		if (!reg->has<CameraComponent>(entity))
			eligibleTypes.push_back("Camera");
		if (!reg->has<FreeLookComponent>(entity))
			eligibleTypes.push_back("FreeLook");
		if (!reg->has<PointLightComponent>(entity))
			eligibleTypes.push_back("PointLight");
		if (!reg->has<SpotLightComponent>(entity))
			eligibleTypes.push_back("SpotLight");
		if (!reg->has<DirectionalLightComponent>(entity))
			eligibleTypes.push_back("DirectionalLight");
		if (!reg->has<RigidbodyComponent>(entity))
			eligibleTypes.push_back("Rigidbody");

		return eligibleTypes;
	}

	void ComponentDrawer::AddComponentToEntity(ECSRegistry* ecs, ECSEntity entity, const std::string& comp)
	{

		LINA_CLIENT_TRACE(comp);
		if (comp.compare("Transformation") == 0)
			ecs->emplace<TransformComponent>(entity, TransformComponent());
		else if (comp.compare("MeshRenderer") == 0)
			ecs->emplace<MeshRendererComponent>(entity, MeshRendererComponent());
		else if (comp.compare("Camera") == 0)
			ecs->emplace<CameraComponent>(entity, CameraComponent());
		else if (comp.compare("FreeLook") == 0)
			ecs->emplace<FreeLookComponent>(entity, FreeLookComponent());
		else if (comp.compare("PointLight") == 0)
			ecs->emplace<PointLightComponent>(entity, PointLightComponent());
		else if (comp.compare("SpotLight") == 0)
			ecs->emplace<SpotLightComponent>(entity, SpotLightComponent());
		else if (comp.compare("DirectionalLight") == 0)
			ecs->emplace<DirectionalLightComponent>(entity, DirectionalLightComponent());
		else if (comp.compare("Rigidbody") == 0)
			ecs->emplace<RigidbodyComponent>(entity, RigidbodyComponent());

	}

}



const char* rigidbodyShapes[]
{
	"SPHERE",
	"BOX",
	"CYLINDER",
	"CAPSULE"
};


void LinaEngine::ECS::TransformComponent::COMPONENT_DRAWFUNC(LinaEngine::ECS::ECSRegistry* ecs, LinaEngine::ECS::ECSEntity entity)
{
	// Align.
	WidgetsUtility::IncrementCursorPosY(15);
	WidgetsUtility::IncrementCursorPosX(12);

	// Draw title.
	static bool open = false;
	bool removeComponent = WidgetsUtility::DrawComponentTitle("Transformation", ICON_FA_ARROWS_ALT, &open, ImGui::GetStyleColorVec4(ImGuiCol_Header));

	// Remove if requested.
	if (removeComponent)
	{
		ecs->remove<TransformComponent>(entity);
		return;
	}

	// Draw component.
	TransformComponent& transform = ecs->get<TransformComponent>(entity);
	if (open)
	{
		float cursorPosInputs = ImGui::GetWindowSize().x * 0.32f;
		float cursorPosLabels = 12;
		ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Location"); ImGui::SameLine(); ImGui::SetCursorPosX(cursorPosInputs); ImGui::DragFloat3("##loc", transform.transform.m_location.Get());
		ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Rotation"); ImGui::SameLine(); ImGui::SetCursorPosX(cursorPosInputs); WidgetsUtility::DragQuaternion("##rot", transform.transform.m_rotation);
		ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Scale");	 ImGui::SameLine();	ImGui::SetCursorPosX(cursorPosInputs); ImGui::DragFloat3("##scale", transform.transform.m_scale.Get());
		WidgetsUtility::IncrementCursorPosY(6.5f);

	}

	// Draw bevel line
	WidgetsUtility::DrawBeveledLine();
}


void LinaEngine::ECS::RigidbodyComponent::COMPONENT_DRAWFUNC(LinaEngine::ECS::ECSRegistry* ecs, LinaEngine::ECS::ECSEntity entity)
{
	// Align.
	WidgetsUtility::IncrementCursorPosY(15);
	WidgetsUtility::IncrementCursorPosX(12);

	// Draw title.
	static bool open = false;
	bool removeComponent = WidgetsUtility::DrawComponentTitle("Rigidbody", ICON_FA_BOX, &open, ImGui::GetStyleColorVec4(ImGuiCol_Header));

	// Remove if requested.
	if (removeComponent)
	{
		ecs->remove<RigidbodyComponent>(entity);
		return;
	}

	// Draw component.
	RigidbodyComponent& rb = ecs->get<RigidbodyComponent>(entity);
	if (open)
	{
		float cursorPos = ImGui::GetWindowSize().x * 0.32f;
		float cursorPosLabels = 12;

		ComponentDrawer::s_currentCollisionShape = (int)rb.m_collisionShape;

		// Draw collision shape.
		static ImGuiComboFlags flags = 0;
		static ECS::CollisionShape selectedCollisionShape = rb.m_collisionShape;
		const char* collisionShapeLabel = rigidbodyShapes[ComponentDrawer::s_currentCollisionShape];

		ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Shape"); ImGui::SameLine(); ImGui::SetCursorPosX(cursorPos);
		if (ImGui::BeginCombo("##collshape", collisionShapeLabel, flags))
		{
			for (int i = 0; i < IM_ARRAYSIZE(rigidbodyShapes); i++)
			{
				const bool is_selected = (ComponentDrawer::s_currentCollisionShape == i);
				if (ImGui::Selectable(rigidbodyShapes[i], is_selected))
				{
					selectedCollisionShape = (ECS::CollisionShape)i;
					ComponentDrawer::s_currentCollisionShape = i;
					rb.m_collisionShape = selectedCollisionShape;
				}

				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();

		}

		ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Mass"); ImGui::SameLine(); ImGui::SetCursorPosX(cursorPos);  ImGui::DragFloat("##mass", &rb.m_mass);

		if (rb.m_collisionShape == ECS::CollisionShape::BOX || rb.m_collisionShape == ECS::CollisionShape::CYLINDER)
		{
			ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Half Extents"); ImGui::SameLine(); ImGui::SetCursorPosX(cursorPos); ImGui::DragFloat3("##halfextents", &rb.m_halfExtents.x);
		}
		else if (rb.m_collisionShape == ECS::CollisionShape::SPHERE)
		{
			ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Radius"); ImGui::SameLine(); ImGui::SetCursorPosX(cursorPos); ImGui::DragFloat("##radius", &rb.m_radius);
		}
		else if (rb.m_collisionShape == ECS::CollisionShape::CAPSULE)
		{
			ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Radius"); ImGui::SameLine(); ImGui::SetCursorPosX(cursorPos); ImGui::DragFloat("##radius", &rb.m_radius);
			ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Height"); ImGui::SameLine(); ImGui::SetCursorPosX(cursorPos); ImGui::DragFloat("##height", &rb.m_capsuleHeight);
		}

		ImGui::SetCursorPosX(cursorPosLabels);
		if (ImGui::Button("Apply"))
			ecs->replace<LinaEngine::ECS::RigidbodyComponent>(entity, rb);

	}

	// Draw bevel line
	WidgetsUtility::DrawBeveledLine();
}

void LinaEngine::ECS::CameraComponent::COMPONENT_DRAWFUNC(LinaEngine::ECS::ECSRegistry* ecs, LinaEngine::ECS::ECSEntity entity)
{
	// Align.
	WidgetsUtility::IncrementCursorPosY(15);
	WidgetsUtility::IncrementCursorPosX(12);

	// Draw title.
	static bool open = false;
	bool removeComponent = WidgetsUtility::DrawComponentTitle("Camera", ICON_FA_CAMERA, &open, ImGui::GetStyleColorVec4(ImGuiCol_Header));

	// Remove if requested.
	if (removeComponent)
	{
		ecs->remove<CameraComponent>(entity);
		return;
	}

	// Draw component.
	CameraComponent& Camera = ecs->get<CameraComponent>(entity);
	if (open)
	{
		float cursorPosInputs = ImGui::GetWindowSize().x * 0.32f;
		float cursorPosLabels = 12;
		//ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Location"); ImGui::SameLine(); ImGui::SetCursorPosX(cursorPosInputs); ImGui::DragFloat3("##loc", Camera.Camera.m_location.Get());
		//ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Rotation"); ImGui::SameLine(); ImGui::SetCursorPosX(cursorPosInputs); WidgetsUtility::DragQuaternion("##rot", Camera.Camera.m_rotation);
		//ImGui::SetCursorPosX(cursorPosLabels); WidgetsUtility::AlignedText("Scale");	 ImGui::SameLine();	ImGui::SetCursorPosX(cursorPosInputs); ImGui::DragFloat3("##scale", Camera.Camera.m_scale.Get());
		WidgetsUtility::IncrementCursorPosY(6.5f);

	}

	// Draw bevel line.
	WidgetsUtility::DrawBeveledLine();
}
