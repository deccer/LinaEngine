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
Class: Mesh

Responsible for storing vertex array data generated into Meshs. MeshRenderers point to a particular
mesh instance in the engine.

Timestamp: 5/6/2019 4:23:45 PM
*/

#pragma once

#ifndef MODEL_HPP
#define MODEL_HPP

#include "ECS/ECS.hpp"
#include "Rendering/Texture.hpp"
#include "Rendering/Mesh.hpp"
#include "Rendering/Material.hpp"
#include "Animation/Skeleton.hpp"
#include "Core/SizeDefinitions.hpp"


namespace Lina::Graphics
{
	class VertexArray;

	class Model
	{

	public:

		Model() {};
		virtual ~Model();

		static ModelParameters LoadParameters(const std::string& path);
		static void SaveParameters(const std::string& path, ModelParameters params);
		static ModelParameters LoadParametersFromMemory(unsigned char* data, size_t dataSize);
		static Model& CreateModel(const std::string& path, const std::string& paramsPath, unsigned char* data, size_t dataSize, ModelParameters modelParams = ModelParameters());
		static Model& CreateModel(const std::string& filePath, ModelParameters meshParams = ModelParameters(), const std::string& paramsPath = "");
		static Model& GetModel(StringIDType id);
		static Model& GetModel(const std::string& path);
		static bool ModelExists(StringIDType id);
		static bool ModelExists(const std::string& path);
		static void UnloadModel(StringIDType id);
		static void UnloadAll();
		static std::map<StringIDType, Model>& GetLoadedModels() { return s_loadedModels; }

		std::vector<Mesh>& GetMeshes()
		{
			return m_meshes;
		}

		std::vector<ModelMaterial>& GetMaterialSpecs()
		{
			return m_materialSpecArray;
		}

		Skeleton& GetSkeleton()
		{
			return m_skeleton;
		}

		void SetParameters(ModelParameters params) { m_parameters = params; }
		ModelParameters& GetParameters() { return m_parameters; }
		ModelSceneParameters& GetWorldParameters() { return m_worldParameters; }
		const std::string& GetPath() const { return m_path; }
		const std::string& GetParamsPath() const { return m_paramsPath; }
		const int GetID() const { return m_meshID; }


	private:

		static std::map<StringIDType, Model> s_loadedModels;

		friend class OpenGLRenderEngine;
		int m_meshID = -1;
		std::string m_path = "";
		std::string m_paramsPath = "";

		ModelSceneParameters m_worldParameters;
		ModelParameters m_parameters;
		std::vector<Mesh> m_meshes;
		std::vector<ModelMaterial> m_materialSpecArray;
		Skeleton m_skeleton;

	};
}

#endif