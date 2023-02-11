/*
This file is a part of: Lina Engine
https://github.com/inanevin/LinaEngine

Author: Inan Evin
http://www.inanevin.com

Copyright (c) [2018-] [Inan Evin]

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

#include "Core/Application.hpp"
#include "Log/Log.hpp"
#include "Profiling/Profiler.hpp"
#include "Core/CoreResourcesRegistry.hpp"
#include "Core/SystemInfo.hpp"
#include "Graphics/Core/Window.hpp"
#include "Platform/PlatformTimeIncl.hpp"

//********** DEBUG
#include "Input/Core/InputMappings.hpp"

#define DEFAULT_RATE 1.0f / 60.0f

namespace Lina
{
	void Engine::Initialize(const SystemInitializationInfo& initInfo)
	{
		LINA_TRACE("[Application] -> Initialization.");

		m_initInfo = initInfo;

		// Child systems can override for custom core resource registries.
		m_coreResourceRegistry = new CoreResourcesRegistry();
		m_coreResourceRegistry->RegisterResourceTypes(m_resourceManager);
		m_resourceManager.SetCoreResources(m_coreResourceRegistry->GetCoreResources());
		m_resourceManager.SetCoreResourcesDefaultMetadata(m_coreResourceRegistry->GetCoreResourceDefaultMetadata());

		// Window manager has priority initialization.
		m_windowManager.Initialize(initInfo);
		m_windowManager.CreateAppWindow(LINA_MAIN_SWAPCHAIN, initInfo.windowStyle, initInfo.appName, Vector2i::Zero, Vector2i(initInfo.windowWidth, initInfo.windowHeight));

		for (auto [type, sys] : m_subsystems)
		{
			if (type != SubsystemType::WindowManager)
				sys->Initialize(initInfo);
		}

		m_resourceManager.SetMode(ResourceManagerMode::File);
		auto start = PlatformTime::GetCycles64();
		m_resourceManager.LoadCoreResources();
		LINA_TRACE("[Application] -> Loading core resources took: {0} seconds", PlatformTime::GetDeltaSeconds64(start, PlatformTime::GetCycles64()));
		PostInitialize();
	}

	void Engine::DispatchSystemEvent(ESystemEvent ev, const Event& data)
	{
		IEventDispatcher::DispatchSystemEvent(ev, data);

		if (ev & EVS_ResourceLoaded)
		{
			String* path = static_cast<String*>(data.pParams[0]);
			LINA_TRACE("[Resource] -> Loaded resource: {0}", path->c_str());
		}
	}

	void Engine::PostInitialize()
	{
		DispatchSystemEvent(EVS_PostSystemInit, {});
	}

	void Engine::Shutdown()
	{
		LINA_TRACE("[Application] -> Shutdown.");

		m_gfxManager.Join();

		DispatchSystemEvent(EVS_PreSystemShutdown, {});

		m_levelManager.Shutdown();
		m_resourceManager.Shutdown();
		m_audioManager.Shutdown();
		m_physicsEngine.Shutdown();
		m_windowManager.Shutdown();
		m_gfxManager.Shutdown();
		m_input.Shutdown();

		delete m_coreResourceRegistry;
	}

	void Engine::Tick(float dt)
	{
		m_input.Tick(dt);
		m_gfxManager.Tick(dt);

		// For any listeners that fall outside the main loop.
		Event eventData;
		eventData.fParams[0] = dt;
		DispatchSystemEvent(EVS_SystemTick, eventData);

		const bool physicsSimulated = m_physicsEngine.Simulate(dt, SystemInfo::GetPhysicsDeltaTime());

		// World tick if exists.
		m_levelManager.Tick(dt);

		auto audioJob  = m_executor.Async([&]() { m_audioManager.Tick(dt); });
		auto renderJob = m_executor.Async([&]() { m_gfxManager.Render(); });

		audioJob.get();
		renderJob.get();
		if (physicsSimulated)
			m_physicsEngine.WaitForSimulation();

		// Sync.
		m_gfxManager.SyncData();
		m_physicsEngine.SyncData();

		// For any listeners that fall outside the main loop.
		DispatchSystemEvent(ESystemEvent::EVS_SyncThreads, {});

		if (m_input.GetKeyDown(LINA_KEY_1))
		{
			m_windowManager.SetVsync(VsyncMode::None);
		}

		if (m_input.GetKeyDown(LINA_KEY_2))
		{
			m_windowManager.SetVsync(VsyncMode::Adaptive);
		}

		if (m_input.GetKeyDown(LINA_KEY_3))
		{
			m_windowManager.SetVsync(VsyncMode::StrongVsync);
		}

		if (m_input.GetKeyDown(LINA_KEY_4))
		{
			m_windowManager.SetVsync(VsyncMode::TripleBuffer);
		}
	}
} // namespace Lina