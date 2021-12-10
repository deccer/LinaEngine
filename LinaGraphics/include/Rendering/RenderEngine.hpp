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

/*
Class: RenderEngine

Responsible for managing the whole rendering pipeline, creating frame buffers, textures,
materials, meshes etc. Also handles most of the memory management for the rendering pipeline.

Timestamp: 4/15/2019 12:26:31 PM
*/

#pragma once

#ifndef RenderEngine_HPP
#define RenderEngine_HPP

#include "Core/Common.hpp"
#include "RenderingCommon.hpp"
#include "ECS/ECSSystem.hpp"
#include "ECS/Systems/CameraSystem.hpp"
#include "ECS/Systems/LightingSystem.hpp"
#include "ECS/Systems/AnimationSystem.hpp"
#include "ECS/Systems/MeshRendererSystem.hpp"
#include "ECS/Systems/SpriteRendererSystem.hpp"
#include "Rendering/ModelLoader.hpp"
#include "Rendering/VertexArray.hpp"
#include "Rendering/RenderBuffer.hpp"
#include "Model.hpp"
#include "UniformBuffer.hpp"
#include "Window.hpp"
#include "RenderContext.hpp"
#include "Math/Color.hpp"
#include "Core/LayerStack.hpp"
#include "RenderSettings.hpp"
#include "PostProcessEffect.hpp"
#include <functional>
#include <set>
#include <queue>

namespace Lina
{
	namespace Input
	{
		class InputEngine;
	}

	class Event;
}

namespace Lina::Graphics
{
	class Shader;

	struct BufferValueRecord
	{
		float zNear;
		float zFar;
	};

	class RenderEngine
	{
	public:

		RenderEngine();
		~RenderEngine();
		void Initialize(Lina::ECS::ECSRegistry& ecsIn, Window& appWindow);
		void Tick(float delta);
		void Render(float interpolation);
		void RenderLayers();
		void AddToRenderingPipeline(Lina::ECS::BaseECSSystem& system);
		void SetViewportDisplay(Vector2 offset, Vector2 size);
		void SetSkyboxMaterial(Material* skyboxMaterial) { m_skyboxMaterial = skyboxMaterial; }
		void PushLayer(Layer& layer);
		void PushOverlay(Layer& layer);
		void MaterialUpdated(Material& mat);
		void UpdateShaderData(Material* mat);
		void SetDrawParameters(const DrawParams& params);
		void UpdateRenderSettings();
		void* GetFinalImage();
		void* GetShadowMapImage();
		void UpdateSystems(float interpolation);
		void BindShaderToViewBuffer(Shader& shader);
		void BindShaderToDebugBuffer(Shader& shader);
		void BindShaderToLightBuffer(Shader& shader);

		// Initializes the setup process for loading an HDRI image to the scene
		void CaptureCalculateHDRI(Texture& hdriTexture);
		void SetHDRIData(Material* mat);
		void RemoveHDRIData(Material* mat);

		void DrawIcon(Vector3 p, uint32 textureID, float size = 1.0f);
		void DrawLine(Vector3 p1, Vector3 p2, Color col, float width = 1.0f);
		void DrawAABB(Vector3 center, Vector3 halfWidths, Color col = Color::White, float width = 1.0f);
		void ProcessDebugQueue();
		void CustomDrawActivation(bool activate) { m_customDrawEnabled = activate; }
		void SetCustomDrawFunction(const std::function<void()>& func) { m_customDrawFunction = func; }

		void SetPostSceneDrawCallback(std::function<void()>& cb) { m_postSceneDrawCallback = cb; }
		Vector2 GetViewportSize() { return m_viewportSize; }
		ECS::CameraSystem* GetCameraSystem() { return &m_cameraSystem; }
		ECS::LightingSystem* GetLightingSystem() { return &m_lightingSystem; }
		ECS::MeshRendererSystem* GetMeshRendererSystem() { return &m_meshRendererSystem; }
		Texture& GetHDRICubemap() { return m_hdriCubemap; }
		static RenderDevice& GetRenderDevice() { return s_renderDevice; }
		static Texture& GetDefaultTexture() { return s_defaultTexture; }
		static Material& GetDefaultUnlitMaterial() { return s_defaultUnlit; }
		static Shader& GetDefaultShader() { return *s_standardUnlitShader; }
		RenderSettings& GetRenderSettings() { return m_renderSettings; }
		DrawParams GetMainDrawParams() { return m_defaultDrawParams; }
		void SetCurrentPLightCount(int count) { m_currentPointLightCount = count; }
		void SetCurrentSLightCount(int count) { m_currentSpotLightCount = count; }
		void SetPreDrawCallback(const std::function<void()>& cb) { m_preDrawCallback = cb; };
		void SetPostDrawCallback(const std::function<void()>& cb) { m_postDrawCallback = cb; };
		void DrawSceneObjects(DrawParams& drawpParams, Material* overrideMaterial = nullptr, bool completeFlush = true);
		void DrawSkybox();
		uint32 GetScreenQuadVAO() { return m_screenQuadVAO; }
		PostProcessEffect& AddPostProcessEffect(Shader& shader);
		UniformBuffer& GetViewBuffer() { return m_globalDataBuffer; }
	private:

		void ConstructEngineShaders();
		bool ValidateEngineShaders();
		void ConstructEngineMaterials();
		void ConstructEnginePrimitives();
		void ConstructRenderTargets();
		void DumpMemory();
		void Draw();
		void DrawFinalize();
		void UpdateUniformBuffers();
		
		// Generating necessary maps for HDRI specular highlighting
		void CalculateHDRICubemap(Texture& hdriTexture, glm::mat4& captureProjection, glm::mat4 views[6]);
		void CalculateHDRIIrradiance(Matrix& captureProjection, Matrix views[6]);
		void CalculateHDRIPrefilter(Matrix& captureProjection, Matrix views[6]);
		void CalculateHDRIBRDF(Matrix& captureProjection, Matrix views[6]);


	private:

		static RenderDevice s_renderDevice;
		Window* m_appWindow;

		RenderTarget m_primaryRenderTarget;
		RenderTarget m_pingPongRenderTarget1;
		RenderTarget m_pingPongRenderTarget2;
		RenderTarget m_hdriCaptureRenderTarget;
		RenderTarget m_shadowMapTarget;
		RenderTarget m_pLightShadowTargets[MAX_POINT_LIGHTS];

#ifdef LINA_EDITOR
		RenderTarget m_secondaryRenderTarget;
		RenderBuffer m_secondaryRenderBuffer;
		Texture m_secondaryRTTexture;
#endif

		RenderBuffer m_primaryBuffer;
		RenderBuffer m_hdriCaptureRenderBuffer;

		// Frame buffer texture parameters
		SamplerParameters m_primaryRTParams;
		SamplerParameters m_pingPongRTParams;
		SamplerParameters m_shadowsRTParams;

		Mesh m_quadMesh;

		Material m_screenQuadFinalMaterial;
		Material m_screenQuadBlurMaterial;
		Material m_screenQuadOutlineMaterial;
		Material* m_skyboxMaterial = nullptr;
		Material m_debugLineMaterial;
		Material m_debugIconMaterial;
		Material m_hdriMaterial;
		Material m_shadowMapMaterial;
		Material m_defaultSkyboxMaterial;
		Material m_pLightShadowDepthMaterial;
		static Material s_defaultUnlit;

		Shader* m_hdriBRDFShader = nullptr;
		Shader* m_hdriPrefilterShader = nullptr;
		Shader* m_hdriEquirectangularShader = nullptr;
		Shader* m_hdriIrradianceShader = nullptr;
		Shader* m_sqFinalShader = nullptr;
		Shader* m_sqBlurShader = nullptr;
		Shader* m_sqShadowMapShader = nullptr;
		Shader* m_debugLineShader = nullptr;
		Shader* m_debugIconShader = nullptr;
		Shader* m_skyboxSingleColorShader = nullptr;
		Shader* m_pointShadowsDepthShader = nullptr;
		static Shader* s_standardUnlitShader;

		Texture m_primaryMSAARTTexture0;
		Texture m_primaryMSAARTTexture1;
		RenderTarget m_primaryMSAATarget;
		RenderBuffer m_primaryMSAABuffer;

		Texture m_primaryRTTexture0;
		Texture m_primaryRTTexture1;
		Texture m_pingPongRTTexture1;
		Texture m_pingPongRTTexture2;
		Texture m_hdriCubemap;
		Texture m_hdriIrradianceMap;
		Texture m_hdriPrefilterMap;
		Texture m_HDRILutMap;
		Texture m_shadowMapRTTexture;
		static Texture s_defaultTexture;
		Texture m_defaultCubemapTexture;
		Texture m_pLightShadowTextures[MAX_POINT_LIGHTS];

		DrawParams m_defaultDrawParams;
		DrawParams m_skyboxDrawParams;
		DrawParams m_fullscreenQuadDP;
		DrawParams m_shadowMapDrawParams;

		UniformBuffer m_globalDataBuffer;
		UniformBuffer m_globalLightBuffer;
		UniformBuffer m_globalDebugBuffer;

		LayerStack m_guiLayerStack;
		RenderingDebugData m_debugData;
		RenderSettings m_renderSettings;

		// Structure that keeps track of current buffer values
		BufferValueRecord m_bufferValueRecord;

		Lina::ECS::AnimationSystem m_animationSystem;
		Lina::ECS::CameraSystem m_cameraSystem;
		Lina::ECS::MeshRendererSystem m_meshRendererSystem;
		Lina::ECS::SpriteRendererSystem m_spriteRendererSystem;
		Lina::ECS::LightingSystem m_lightingSystem;
		Lina::ECS::ECSSystemList m_renderingPipeline;
		Lina::ECS::ECSSystemList m_animationPipeline;

	private:

		uint32 m_skyboxVAO = 0;
		uint32 m_screenQuadVAO = 0;
		uint32 m_hdriCubeVAO = 0;
		uint32 m_lineVAO = 0;

		int m_currentSpotLightCount = 0;
		int m_currentPointLightCount = 0;
		bool m_hdriDataCaptured = false;
		bool m_customDrawEnabled;

		Vector2 m_hdriResolution = Vector2(512, 512);
		Vector2 m_shadowMapResolution = Vector2(2048, 2048);
		Vector2 m_viewportPos = Vector2::Zero;
		Vector2 m_viewportSize = Vector2::Zero;
		Vector2 m_pLightShadowResolution = Vector2(1024, 1024);

		std::function<void()> m_postSceneDrawCallback;
		std::function<void()> m_preDrawCallback;
		std::function<void()> m_postDrawCallback;
		std::function<void()> m_customDrawFunction;
		bool m_firstFrameDrawn = false;

		std::queue<DebugLine> m_debugLineQueue;
		std::queue<DebugIcon> m_debugIconQueue;
		std::map<Shader*, PostProcessEffect> m_postProcessMap;

		DISALLOW_COPY_ASSIGN_MOVE(RenderEngine)
	};

}


#endif