/*
Author: Inan Evin
www.inanevin.com

Copyright 2018 Inan Evin

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions
and limitations under the License.

Class: SB_Application
Timestamp: 12/29/2018 11:15:41 PM

*/

#include <Lina.hpp>
#include "PackageManager/PAMWindow.hpp"
#include "PackageManager/PAMInputDevice.hpp"
#include "Rendering/RenderEngine.hpp"
#include "Core/PhysicsEngine.hpp"
#include "Core/AudioEngine.hpp"
#include "Input/InputEngine.hpp"
#include "Core/EditorApplication.hpp"
#include "ECS/Components/FreeLookComponent.hpp"
#include "ECS/Components/MeshRendererComponent.hpp"
#include "ECS/Components/ModelRendererComponent.hpp"
#include "ECS/Components/SpriteRendererComponent.hpp"
#include "ECS/Components/CameraComponent.hpp"
#include "ECS/Components/RigidbodyComponent.hpp"
#include "Game/GameManager.hpp"
#include "Editor/ClientComponentDrawer.hpp"
#include "ECS/Components/EntityDataComponent.hpp"


class SandboxApplication : public LinaEngine::Application
{
	public:

	SandboxApplication() 
	{
		LINA_CLIENT_TRACE("[Constructor] -> Sandbox Application ({0})", typeid(*this).name());

		// Init engine.
		LinaEngine::Graphics::WindowProperties props;
		props.m_width = 1920;
		props.m_height = 1080;
		props.m_windowState = Graphics::WindowState::Maximized;
		props.m_decorated = false;
		props.m_resizable = true;
		props.m_fullscreen = false;
		props.m_msaaSamples = 4;
		props.m_title = "Lina Engine - Configuration [] - Build Type [] - Project [] - Build []";
		Initialize(props);

#ifdef LINA_EDITOR
		m_editor.Setup();
		
		// Refresh after level init.
		m_editor.Refresh();
		
		// Update props.
		auto& windowProps = GetAppWindow().GetWindowProperties();

		// Set the app window size back to original.
		GetAppWindow().SetPos(Vector2::Zero);
		GetAppWindow().SetSize(Vector2(windowProps.m_workingAreaWidth, windowProps.m_workingAreaHeight));

		SetPlayMode(false);
#else
		SetPlayMode(true);
#endif

		GetMainStack().PushLayer(m_gameManager);

		// Run engine.
		Run();

	}

	~SandboxApplication()
	{
		LINA_CLIENT_TRACE("[Destructor] -> Sandbox Application ({0})", typeid(*this).name());
	}

	private:
		
#ifdef LINA_EDITOR
		LinaEditor::EditorApplication m_editor;
#endif
		ClientComponentDrawer m_componentDrawer;
		GameManager m_gameManager;

		// Inherited via Application
		virtual void SerializeRegistry(LinaEngine::ECS::ECSRegistry& registry, cereal::BinaryOutputArchive& oarchive) override
		{
			entt::snapshot{ registry }
				.entities(oarchive)
				.component<
				LinaEngine::ECS::EntityDataComponent,
				LinaEngine::ECS::CameraComponent,
				LinaEngine::ECS::FreeLookComponent,
				LinaEngine::ECS::PointLightComponent,
				LinaEngine::ECS::DirectionalLightComponent,
				LinaEngine::ECS::SpotLightComponent,
				LinaEngine::ECS::RigidbodyComponent,
				LinaEngine::ECS::MeshRendererComponent,
				LinaEngine::ECS::SpriteRendererComponent,
				LinaEngine::ECS::ModelRendererComponent
				>(oarchive);
		}

		virtual void DeserializeRegistry(LinaEngine::ECS::ECSRegistry& registry, cereal::BinaryInputArchive& iarchive) override
		{
			entt::snapshot_loader{ registry }
				.entities(iarchive)
				.component<
				LinaEngine::ECS::EntityDataComponent,
				LinaEngine::ECS::CameraComponent,
				LinaEngine::ECS::FreeLookComponent,
				LinaEngine::ECS::PointLightComponent,
				LinaEngine::ECS::DirectionalLightComponent,
				LinaEngine::ECS::SpotLightComponent,
				LinaEngine::ECS::RigidbodyComponent,
				LinaEngine::ECS::MeshRendererComponent,
				LinaEngine::ECS::SpriteRendererComponent,
				LinaEngine::ECS::ModelRendererComponent
				>(iarchive);
		}

};

LinaEngine::Application* LinaEngine::CreateApplication()
{
	return new SandboxApplication();
}

// Default platform context window.
LinaEngine::Graphics::Window* LinaEngine::CreateContextWindow()
{
	return new ContextWindow();
}

// Default platform input device.
LinaEngine::Input::InputDevice* LinaEngine::CreateInputDevice()
{
	return new InputDevice();
}

// Default engine
LinaEngine::Graphics::RenderEngine* LinaEngine::CreateRenderEngine()
{
	return new LinaEngine::Graphics::RenderEngine();
}

// Default engine
LinaEngine::Physics::PhysicsEngine* LinaEngine::CreatePhysicsEngine()
{
	return new LinaEngine::Physics::PhysicsEngine();
}

// Default engine
LinaEngine::Audio::AudioEngine* LinaEngine::CreateAudioEngine()
{
	return new LinaEngine::Audio::AudioEngine();
}

// Default engine
LinaEngine::Input::InputEngine* LinaEngine::CreateInputEngine()
{
	return new LinaEngine::Input::InputEngine();
}