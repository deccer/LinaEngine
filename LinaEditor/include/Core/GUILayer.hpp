/*
Author: Inan Evin
www.inanevin.com

Copyright 2018 Inan Evin

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions
and limitations under the License.

Class: UILayer

*/

#include "Core/Layer.hpp"
#include "Rendering/RenderEngine.hpp"
#include "ECS/ECSSystem.hpp"
#include <vector>

namespace LinaEngine
{
	class Application;
}

namespace LinaEditor
{

	struct ContentFolder
	{
		std::vector<ContentFolder> children;
		std::string name;
		std::string path;
		ContentFolder* parent;
	};

	struct EditorEntity
	{
		std::string name = "Entity";
		entt::entity entity;
	};

	class GUILayer : public LinaEngine::Layer
	{

	public:

		GUILayer() : LinaEngine::Layer("UILayer")
		{

		}

		// OVERRIDES
		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate() override;
		void OnEvent() override;

		// CLASS METHODS
		FORCEINLINE void Setup(LinaEngine::Graphics::RenderEngine& engine, LinaEngine::Application* application, LinaEngine::ECS::ECSRegistry& ecs) { m_RenderEngine = &engine; m_Application = application; m_ECS = &ecs; }

	private:

		// Draws panel for entities.
		void DrawECSPanel(bool* isOpen);

		// Draws a central docking space.
		void DrawCentralDockingSpace();

		// Draw content browser.
		void DrawContentBrowserWindow();

		// Draw skybox settings.
		void DrawSkyboxSettingsWindow();

		// Show the file menu for content browser.
		void ShowContentBrowserFileMenu();

		// Checks the project content folder for new files.
		void ReadProjectContentsFolder();

	private:


		void CreateNewEntity();

	private:

		LinaEngine::Graphics::RenderEngine* m_RenderEngine;
		LinaEngine::ECS::ECSRegistry* m_ECS;
		LinaEngine::Application* m_Application;
		std::vector<ContentFolder> m_ContentFolders;

		std::vector<EditorEntity> m_EditorEntities;

	};
}