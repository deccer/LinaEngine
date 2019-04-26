/*
Author: Inan Evin
www.inanevin.com

Copyright 2018 Inan Evin

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions
and limitations under the License.

Class: GLRenderEngine
Timestamp: 4/15/2019 12:37:37 PM

*/

#include "LinaPch.hpp"
#include "PackageManager/OpenGL/GLRenderEngine.hpp"  
#include "glad/glad.h"


#include "ECS/EntityComponentSystem.hpp"
#include "ECS/Components/MovementControlComponent.hpp"
#include "ECS/Components/RenderableMeshComponent.hpp"
#include "ECS/Components/TransformComponent.hpp"
#include "ECS/Systems/ECSMovementControlSystem.hpp"
#include "PackageManager/PAMInputEngine.hpp"
#include "Core/Application.hpp"

namespace LinaEngine::Graphics
{

	// ---------------------------------------------------------------------
	// ---------------------------------------------------------------------
	// GLOBALS DECLARATIONS
	// ---------------------------------------------------------------------
	// ---------------------------------------------------------------------

	GLint GetOpenGLFormat(PixelFormat dataFormat);
	GLint GetOpenGLInternalFormat(PixelFormat internalFormat, bool compress);
	static bool AddShader(GLuint shaderProgram, const LinaString& text, GLenum type, LinaArray<GLuint>* shaders);
	static void AddAllAttributes(GLuint program, const LinaString& vertexShaderText, uint32 version);
	static bool CheckShaderError(GLuint shader, int flag, bool isProgram, const LinaString& errorMessage);
	static void AddShaderUniforms(GLuint shaderProgram, const LinaString& shaderText, LinaMap<LinaString, GLint>& uniformMap, LinaMap<LinaString, GLint>& samplerMap);

	using namespace ECS;


	EntityComponentSystem ecs;
	EntityHandle entity;
	TransformComponent transformComponent;
	MovementControlComponent movementComponent;
	ECSSystemList mainSystems;
	ECSMovementControlSystem movementControlSystem;
	Transform* workingTransformation;

	// ---------------------------------------------------------------------
	// ---------------------------------------------------------------------
	// CORE OPERATIONS
	// ---------------------------------------------------------------------
	// ---------------------------------------------------------------------

	GLRenderEngine::GLRenderEngine() : RenderEngine()
	{
		LINA_CORE_TRACE("[Constructor] -> GLRenderEngine ({0})", typeid(*this).name());
		m_MainWindow = std::make_unique<PAMWindow>();
	}

	GLRenderEngine::~GLRenderEngine()
	{
		LINA_CORE_TRACE("[Destructor] -> GLRenderEngine ({0})", typeid(*this).name());
	}

	void GLRenderEngine::Initialize_Impl()
	{
		transformComponent.transform.SetPosition(Vector3F(0.0f, 0.0f, 10.0f));

		movementComponent.movementControls.push_back(LinaMakePair(Vector3F(1.0f, 0.0f, 0.0f) * 3, Application::Get().GetInputDevice().GetHorizontalKeyAxis()));
		movementComponent.movementControls.push_back(LinaMakePair(Vector3F(0.0f, 1.0f, 0.0f) * 3, Application::Get().GetInputDevice().GetVerticalKeyAxis()));

		entity = ecs.MakeEntity(transformComponent, movementComponent);
		workingTransformation = &ecs.GetComponent<TransformComponent>(entity)->transform;

		mainSystems.AddSystem(movementControlSystem);

	}

	void GLRenderEngine::Tick_Impl()
	{
		m_MainWindow->Tick();
		glClearColor(0.5f, 0.5f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ecs.UpdateSystems(mainSystems, 0.01f);
		LINA_CORE_TRACE("Position: {0} " , ecs.GetComponent<TransformComponent>(entity)->transform.GetPosition().ToString());
	}


	// ---------------------------------------------------------------------
	// ---------------------------------------------------------------------
	// TEXTURE OPERATIONS
	// ---------------------------------------------------------------------
	// ---------------------------------------------------------------------

	uint32 GLRenderEngine::CreateTexture2D_Impl(int32 width, int32 height, const void * data, PixelFormat pixelDataFormat, PixelFormat internalPixelFormat, bool generateMipMaps, bool compress)
	{
		// Declare formats, target & handle for the texture.
		GLint format = GetOpenGLFormat(pixelDataFormat);
		GLint internalFormat = GetOpenGLInternalFormat(internalPixelFormat, compress);
		GLenum textureTarget = GL_TEXTURE_2D;
		GLuint textureHandle;

		// Generate texture & bind to program.
		glGenTextures(1, &textureHandle);
		glBindTexture(textureTarget, textureHandle);

		// OpenGL texture params.
		glTexParameterf(textureTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(textureTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(textureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(textureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(textureTarget, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);

		// Enable mipmaps if needed.
		if (generateMipMaps) 
			glGenerateMipmap(textureTarget);
		else 
		{
			glTexParameteri(textureTarget, GL_TEXTURE_BASE_LEVEL, 0);
			glTexParameteri(textureTarget, GL_TEXTURE_MAX_LEVEL, 0);
		}

		return textureHandle;
	}

	uint32 GLRenderEngine::CreateDDSTexture2D_Impl(uint32 width, uint32 height, const unsigned char * buffer, uint32 fourCC, uint32 mipMapCount)
	{
		// Define the necessary format.
		GLint format;
		switch (fourCC)
		{
		case FOURCC_DXT1:
			format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			break;
		case FOURCC_DXT3:
			format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			break;
		case FOURCC_DXT5:
			format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			break;
		default:
			LINA_CORE_ERR("Invalid compression format for DDS texture\n");
			return 0;
		}

		// Generate texture & bind to program.
		GLuint textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// OpenGL Texture Params.
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// Define blocksize to be used for mipmap blocks.
		unsigned int blockSize = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
		unsigned int offset = 0;

		// Set mipmap levels.
		for (unsigned int level = 0; level < mipMapCount && (width || height); ++level)
		{
			unsigned int size = ((width + 3) / 4)*((height + 3) / 4)*blockSize;
			glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height,	0, size, buffer + offset);

			offset += size;
			width /= 2;
			height /= 2;
		}

		return textureID;
	}

	uint32 GLRenderEngine::ReleaseTexture2D_Impl(uint32 texture2D)
	{
		// Delete the texture binding if exists.
		if (texture2D == 0) return 0;
		glDeleteTextures(1, &texture2D);
		return 0;
	}

	// ---------------------------------------------------------------------
	// ---------------------------------------------------------------------
	// VERTEX ARRAY OPERATIONS
	// ---------------------------------------------------------------------
	// ---------------------------------------------------------------------

	uint32 GLRenderEngine::CreateVertexArray_Impl(const float** vertexData, const uint32* vertexElementSizes, uint32 numVertexComponents, uint32 numInstanceComponents, uint32 numVertices, const uint32* indices, uint32 numIndices, BufferUsage bufferUsage)
	{
		// Define vertex array object, buffers, buffer count & their sizes.
		unsigned int numBuffers = numVertexComponents + numInstanceComponents + 1;
		GLuint VAO;
		GLuint* buffers = new GLuint[numBuffers];
		uintptr* bufferSizes = new uintptr[numBuffers];

		// Generate vertex array object and activate it, then generate necessary buffers.
		glGenVertexArrays(1, &VAO);
		SetVAO(VAO);
		glGenBuffers(numBuffers, buffers);

		// Define attribute for each buffer.
		for (uint32 i = 0, attribute = 0; i < numBuffers - 1; i++) 
		{
			// Check vertex component count and switch to dynamic draw if current attribute exceeds. This means we are supposed to do instanced rendering.
			BufferUsage attribUsage = bufferUsage;
			bool inInstancedMode = false;
			if (i >= numVertexComponents) 
			{
				attribUsage = BufferUsage::USAGE_DYNAMIC_DRAW;
				inInstancedMode = true;
			}

			// Define element size for the current buffers, as well as buffer data if applicable.
			uint32 elementSize = vertexElementSizes[i];
			const void* bufferData = inInstancedMode ? nullptr : vertexData[i];
			uintptr dataSize = inInstancedMode ? elementSize * sizeof(float) : elementSize * sizeof(float) * numVertices;

			// Bind the current array buffer & set the data.
			glBindBuffer(GL_ARRAY_BUFFER, buffers[i]);
			glBufferData(GL_ARRAY_BUFFER, dataSize, bufferData, attribUsage);
			bufferSizes[i] = dataSize;

			// Define element sizes to pass the required part of the array to the attrib pointer call.
			uint32 elementSizeDiv = elementSize / 4;
			uint32 elementSizeRem = elementSize % 4;

			// Attribute pointer for each block of elements.
			for (uint32 j = 0; j < elementSizeDiv; j++)
			{
				glEnableVertexAttribArray(attribute);
				glVertexAttribPointer(attribute, 4, GL_FLOAT, GL_FALSE,	elementSize * sizeof(GLfloat), (const GLvoid*)(sizeof(GLfloat) * j * 4));

				if (inInstancedMode) 
					glVertexAttribDivisor(attribute, 1);

				attribute++;
			}

			// Last elements.
			if (elementSizeRem != 0)
			{
				glEnableVertexAttribArray(attribute);
				glVertexAttribPointer(attribute, elementSize, GL_FLOAT, GL_FALSE, elementSize * sizeof(GLfloat), (const GLvoid*)(sizeof(GLfloat) * elementSizeDiv * 4));
				
				if (inInstancedMode) 
					glVertexAttribDivisor(attribute, 1);
				
				attribute++;
			}
		}
		
		// Finally bind the element array buffer.
		uintptr indicesSize = numIndices * sizeof(uint32);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[numBuffers - 1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize, indices, bufferUsage);
		bufferSizes[numBuffers - 1] = indicesSize;

		// Create vertex array based on our calculated data.
		VertexArray vaoData;
		vaoData.buffers = buffers;
		vaoData.bufferSizes = bufferSizes;
		vaoData.numBuffers = numBuffers;
		vaoData.numElements = numIndices;
		vaoData.bufferUsage = bufferUsage;
		vaoData.instanceComponentsStartIndex = numVertexComponents;

		// Store the array in our map & return the modified vertex array object.
		m_VAOMap[VAO] = vaoData;
		return VAO;
	}

	uint32 GLRenderEngine::ReleaseVertexArray_Impl(uint32 vao)
	{
		// Terminate if vao is null or does not exist in our mapped objects.
		if (vao == 0) return 0;
		LinaMap<uint32, VertexArray>::iterator it = m_VAOMap.find(vao);
		if (it == m_VAOMap.end()) return 0;

		// Get the vertex array object data from the map.
		const VertexArray* vaoData = &it->second;

		// Delete the VA & buffers, then data.
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(vaoData->numBuffers, vaoData->buffers);
		delete[] vaoData->buffers;
		delete[] vaoData->bufferSizes;

		// Remove from the map.
		m_VAOMap.erase(it);
		return 0;
	}

	// ---------------------------------------------------------------------
	// ---------------------------------------------------------------------
	// TEXTURE SAMPLER OPERATIONS
	// ---------------------------------------------------------------------
	// ---------------------------------------------------------------------

	uint32 GLRenderEngine::CreateSampler_Impl(SamplerFilter minFilter, SamplerFilter magFilter, SamplerWrapMode wrapU, SamplerWrapMode wrapV, float anisotropy)
	{
		// OpenGL Texture Sampler parameters.
		uint32 result = 0;
		glGenSamplers(1, &result);
		glSamplerParameteri(result, GL_TEXTURE_WRAP_S, wrapU);
		glSamplerParameteri(result, GL_TEXTURE_WRAP_T, wrapV);
		glSamplerParameteri(result, GL_TEXTURE_MAG_FILTER, magFilter);
		glSamplerParameteri(result, GL_TEXTURE_MIN_FILTER, minFilter);

		// Set anisotropy if applicable.
		if (anisotropy != 0.0f && minFilter != FILTER_NEAREST && minFilter != FILTER_LINEAR)
			glSamplerParameterf(result, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);

		return result;
	}

	uint32 GLRenderEngine::ReleaseSampler_Impl(uint32 sampler)
	{
		// Delete the sampler binding if exists.
		if (sampler == 0) return 0;
		glDeleteSamplers(1, &sampler);
		return 0;
	}

	// ---------------------------------------------------------------------
	// ---------------------------------------------------------------------
	// UNFORM BUFFER OPERATIONS
	// ---------------------------------------------------------------------
	// ---------------------------------------------------------------------

	uint32 GLRenderEngine::CreateUniformBuffer_Impl(const void* data, uintptr dataSize, BufferUsage usage)
	{
		// Bind a new uniform buffer to GL.
		uint32 ubo;
		glGenBuffers(1, &ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		glBufferData(GL_UNIFORM_BUFFER, dataSize, data, usage);
		return ubo;
	}

	uint32 GLRenderEngine::ReleaseUniformBuffer_Impl(uint32 buffer)
	{
		// Delete the buffer if exists.
		if (buffer == 0) return 0;
		glDeleteBuffers(1, &buffer);
		return 0;
	}

	// ---------------------------------------------------------------------
	// ---------------------------------------------------------------------
	// SHADER PROGRAM OPERATIONS
	// ---------------------------------------------------------------------
	// ---------------------------------------------------------------------

	uint32 GLRenderEngine::CreateShaderProgram_Impl(const LinaString& shaderText)
	{
		// Shader program instance.
		GLuint shaderProgram = glCreateProgram();

		if (shaderProgram == 0)
		{
			LINA_CORE_ERR("Error creating shader program!");
			return (uint32)-1;
		}

		// Modify the shader text to include the version data.
		LinaString version = GetShaderVersion();
		LinaString vertexShaderText = "#version " + version + "\n#define VS_BUILD\n#define GLSL_VERSION " + version + "\n" + shaderText;
		LinaString fragmentShaderText = "#version " + version + "\n#define FS_BUILD\n#define GLSL_VERSION " + version + "\n" + shaderText;


		// Add the shader program, terminate if fails.
		ShaderProgram programData;
		if (!AddShader(shaderProgram, vertexShaderText, GL_VERTEX_SHADER, &programData.shaders)) 
			return (uint32)-1;

		if (!AddShader(shaderProgram, fragmentShaderText, GL_FRAGMENT_SHADER, &programData.shaders))
			return (uint32)-1;

		// Link program & check link errors.
		glLinkProgram(shaderProgram);

		if (CheckShaderError(shaderProgram, GL_LINK_STATUS, true, "Error linking shader program")) 
			return (uint32)-1;

		// Validate program & check validation errors.
		glValidateProgram(shaderProgram);

		if (CheckShaderError(shaderProgram, GL_VALIDATE_STATUS,	true, "Invalid shader program")) 
			return (uint32)-1;

		// Bind attributes for GL & add shader uniforms.
		AddAllAttributes(shaderProgram, vertexShaderText, GetVersion());
		AddShaderUniforms(shaderProgram, shaderText, programData.uniformMap, programData.samplerMap);

		// Store the program in our map & return it.
		m_ShaderProgramMap[shaderProgram] = programData;
		return shaderProgram;
	}

	uint32 GLRenderEngine::ReleaseShaderProgram_Impl(uint32 shader)
	{
		// Terminate if shader is not valid or does not exist in our map.
		if (shader == 0) return 0;
		LinaMap<uint32, ShaderProgram>::iterator programIt = m_ShaderProgramMap.find(shader);
		if (programIt == m_ShaderProgramMap.end()) return 0;
	
		// Get the program from the map.
		const struct ShaderProgram* shaderProgram = &programIt->second;

		// Detach & delete each shader assigned to our program.
		for (LinaArray<uint32>::const_iterator it = shaderProgram->shaders.begin();	it != shaderProgram->shaders.end(); ++it)
		{
			glDetachShader(shader, *it);
			glDeleteShader(*it);
		}

		// Delete the program, erase from our map & return.
		glDeleteProgram(shader);
		m_ShaderProgramMap.erase(programIt);
		return 0;
	}

	// ---------------------------------------------------------------------
	// ---------------------------------------------------------------------
	// FRAME BUFFER OBJECT OPERATIONS
	// ---------------------------------------------------------------------
	// ---------------------------------------------------------------------

	uint32 GLRenderEngine::CreateRenderTarget_Impl(uint32 texture, int32 width, int32 height, FramebufferAttachment attachment, uint32 attachmentNumber, uint32 mipLevel)
	{
		// Generate frame buffers & set the current object.
		uint32 fbo;
		glGenFramebuffers(1, &fbo);
		SetFBO(fbo);

		// Define attachment type & use the buffer.
		GLenum attachmentTypeGL = attachment + attachmentNumber;
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentTypeGL, GL_TEXTURE_2D, texture, mipLevel);

		// Define frame buffer object data and store it in our map.
		FBOData data;
		data.width = width;
		data.height = height;
		m_FBOMap[fbo] = data;
		return fbo;
	}

	uint32 GLRenderEngine::ReleaseRenderTarget_Impl(uint32 fbo)
	{
		// Terminate if fbo is not valid or does not exist in our map.
		if (fbo == 0) return 0;
		LinaMap<uint32, FBOData>::iterator it = m_FBOMap.find(fbo);
		if (it == m_FBOMap.end()) return 0;

		// Delete the frame buffer object, erase from our map & return.
		glDeleteFramebuffers(1, &fbo);
		m_FBOMap.erase(it);
		return 0;
	}

	// ---------------------------------------------------------------------
	// ---------------------------------------------------------------------
	// VERTEX ARRAY OPERATIONS
	// ---------------------------------------------------------------------
	// ---------------------------------------------------------------------

	void GLRenderEngine::UpdateVertexArray_Impl(uint32 vao, uint32 bufferIndex, const void* data, uintptr dataSize)
	{
		// Terminate if fbo is not valid or does not exist in our map.
		if (vao == 0)  return;
		LinaMap<uint32, VertexArray>::iterator it = m_VAOMap.find(vao);
		if (it == m_VAOMap.end()) return;

		// Get the vertex array object data from the map.
		const VertexArray* vaoData = &it->second;

		// Define a usage according to the VAO data.
		BufferUsage usage;
		if (bufferIndex >= vaoData->instanceComponentsStartIndex)
			usage = BufferUsage::USAGE_DYNAMIC_DRAW;
		else
			usage = vaoData->bufferUsage;
		
		// Use VAO & bind its corresponding buffer.
		SetVAO(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vaoData->buffers[bufferIndex]);

		// If buffer size exceeds data size use it as subdata.
		if (vaoData->bufferSizes[bufferIndex] >= dataSize) 
			glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, data);
		else
		{
			glBufferData(GL_ARRAY_BUFFER, dataSize, data, usage);
			vaoData->bufferSizes[bufferIndex] = dataSize;
		}
	}

	// ---------------------------------------------------------------------
	// ---------------------------------------------------------------------
	// SHADER OPERATIONS
	// ---------------------------------------------------------------------
	// ---------------------------------------------------------------------

	void GLRenderEngine::SetShader_Impl(uint32 shader)
	{
		// Use the target shader if exists.
		if (shader == m_BoundShader) return;
		glUseProgram(shader);
		m_BoundShader = shader;
	}

	void GLRenderEngine::SetShaderSampler_Impl(uint32 shader, const LinaString & samplerName, uint32 texture, uint32 sampler, uint32 unit)
	{
		// Use shader first.
		SetShader_Impl(shader);

		// Activate the sampler data.
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D, texture);
		glBindSampler(unit, sampler);
		glUniform1i(m_ShaderProgramMap[shader].samplerMap[samplerName], unit);
	}

	void GLRenderEngine::SetShaderUniformBuffer_Impl(uint32 shader, const LinaString& uniformBufferName, uint32 buffer)
	{
		// Use shader first.
		SetShader(shader);

		// Update the uniform data.
		glBindBufferBase(GL_UNIFORM_BUFFER, m_ShaderProgramMap[shader].uniformMap[uniformBufferName], buffer);
	}

	// ---------------------------------------------------------------------
	// ---------------------------------------------------------------------
	// UNIFORM BUFFER OPERATIONS
	// ---------------------------------------------------------------------
	// ---------------------------------------------------------------------

	void GLRenderEngine::UpdateVertexArrayBuffer_Impl(uint32 vao, uint32 bufferIndex, const void* data, uintptr dataSize)
	{
		// Terminate if VAO is not valid or does not exist in our map.
		if (vao == 0) return;
		LinaMap<uint32, VertexArray>::iterator it = m_VAOMap.find(vao);
		if (it == m_VAOMap.end()) return;

		// Get VAO data from the map.
		const VertexArray* vaoData = &it->second;

		BufferUsage usage;

		// Check usage & enable dynamic draw if is needed to be instanced.
		if (bufferIndex >= vaoData->instanceComponentsStartIndex) 
			usage = BufferUsage::USAGE_DYNAMIC_DRAW;
		else 
			usage = vaoData->bufferUsage;
		

		// Use VAO & bind buffer.
		SetVAO(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vaoData->buffers[bufferIndex]);

		// If buffer size exceeds data size use it as subdata.
		if (vaoData->bufferSizes[bufferIndex] >= dataSize)
			glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, data);
		else 
		{
			glBufferData(GL_ARRAY_BUFFER, dataSize, data, usage);
			vaoData->bufferSizes[bufferIndex] = dataSize;
		}
	}

	void GLRenderEngine::UpdateUniformBuffer_Impl(uint32 buffer, const void* data, uintptr dataSize)
	{
		// Get buffer & set data.
		glBindBuffer(GL_UNIFORM_BUFFER, buffer);
		void* dest = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
		Memory::memcpy(dest, data, dataSize);
		glUnmapBuffer(GL_UNIFORM_BUFFER);
	}

	// ---------------------------------------------------------------------
	// ---------------------------------------------------------------------
	// DRAWING OPERATIONS
	// ---------------------------------------------------------------------
	// ---------------------------------------------------------------------

	void GLRenderEngine::SetVAO(uint32 vao)
	{
		// Use VAO if exists.
		if (vao == m_BoundVAO) 	return;
		glBindVertexArray(vao);
		m_BoundVAO = vao;
	}

	void GLRenderEngine::SetFBO(uint32 fbo)
	{
		// Use FBO if exists.
		if (fbo == m_BoundFBO) return;
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		m_BoundFBO = fbo;
	}

	void GLRenderEngine::Draw_Impl(uint32 fbo, uint32 shader, uint32 vao, const DrawParams& drawParams, uint32 numInstances, uint32 numElements)
	{
		// No need to draw nothin dude.
		if (numInstances == 0) return;

		// Bind the render targets.
		SetFBO(fbo);

		// Ensure viewport is ok.
		SetViewport(fbo);

		// Set blend mode for each render target.
		SetBlending(drawParams.sourceBlend, drawParams.destBlend);

		// Set scissors tests if required, face culling modes as well as depth tests.
		SetScissorTest(drawParams.useScissorTest, drawParams.scissorStartX, drawParams.scissorStartY, drawParams.scissorWidth, drawParams.scissorHeight);
		SetFaceCulling(drawParams.faceCulling);
		SetDepthTest(drawParams.shouldWriteDepth, drawParams.depthFunc);

		// Bind & use the target shader.
		SetShader_Impl(shader);

		// use array buffer & attributes.
		SetVAO(vao);

		// 1 object or instanced draw calls?
		if (numInstances == 1) 
			glDrawElements(drawParams.primitiveType, (GLsizei)numElements, GL_UNSIGNED_INT, 0);
		else 
			glDrawElementsInstanced(drawParams.primitiveType, (GLsizei)numElements, GL_UNSIGNED_INT, 0, numInstances);
		
	}

	void GLRenderEngine::Clear_Impl(uint32 fbo, bool shouldClearColor, bool shouldClearDepth, bool shouldClearStencil, const Color& color, uint32 stencil)
	{
		// Make sure frame buffer objects are used.
		SetFBO(fbo);
		uint32 flags = 0;

		// Set flags according to options.
		if (shouldClearColor) 
		{
			flags |= GL_COLOR_BUFFER_BIT;
			glClearColor(color[0], color[1], color[2], color[3]);
		}
		if (shouldClearDepth) 
			flags |= GL_DEPTH_BUFFER_BIT;
		if (shouldClearStencil) 
		{
			flags |= GL_STENCIL_BUFFER_BIT;
			SetStencilWriteMask(stencil);
		}

		// Clear the desired flags.
		glClear(flags);
	}

	void GLRenderEngine::SetViewport(uint32 fbo)
	{
		// Update viewport according to the render targets if exist.
		if (fbo == m_ViewportFBO) return;
		glViewport(0, 0, m_FBOMap[fbo].width, m_FBOMap[fbo].height);
		m_ViewportFBO = fbo;
	}

	void GLRenderEngine::SetFaceCulling(FaceCulling faceCulling)
	{
		if (faceCulling == m_UsedFaceCulling) return;

		// If target is enabled, then disable face culling.
		// If current is disabled, then enable faceculling.
		// Else switch cull state.
		if (faceCulling == FACE_CULL_NONE) 
			glDisable(GL_CULL_FACE);
		else if (m_UsedFaceCulling == FACE_CULL_NONE) { // Face culling is disabled but needs to be enabled
			glEnable(GL_CULL_FACE);
			glCullFace(faceCulling);
		}
		else  
			glCullFace(faceCulling);
		m_UsedFaceCulling = faceCulling;
	}

	void GLRenderEngine::SetDepthTest(bool shouldWrite, DrawFunc depthFunc)
	{

		// Toggle dept writing.
		if (shouldWrite != m_ShouldWriteDepth)
		{
			glDepthMask(shouldWrite ? GL_TRUE : GL_FALSE);
			m_ShouldWriteDepth = shouldWrite;
		}

		// Update if change is needed.
		if (depthFunc == m_UsedDepthFunction)	return;
		glDepthFunc(depthFunc);
		m_UsedDepthFunction = depthFunc;
	}

	void GLRenderEngine::SetBlending(BlendFunc sourceBlend, BlendFunc destBlend)
	{
		// If no change is needed return.
		if (sourceBlend == m_UsedSourceBlending && destBlend == m_UsedDestinationBlending) return;
		else if (sourceBlend == BLEND_FUNC_NONE || destBlend == BLEND_FUNC_NONE) 
			glDisable(GL_BLEND);
		else if (m_UsedSourceBlending == BLEND_FUNC_NONE || m_UsedDestinationBlending == BLEND_FUNC_NONE)
		{
			glEnable(GL_BLEND);
			glBlendFunc(sourceBlend, destBlend);
		}
		else 
			glBlendFunc(sourceBlend, destBlend);
		

		m_UsedSourceBlending = sourceBlend;
		m_UsedDestinationBlending = destBlend;
	}

	void GLRenderEngine::SetStencilTest(bool enable, DrawFunc stencilFunc, uint32 stencilTestMask, uint32 stencilWriteMask, int32 stencilComparisonVal, StencilOp stencilFail, StencilOp stencilPassButDepthFail, StencilOp stencilPass)
	{
		// If change is needed toggle enabled state & enable/disable stencil test.
		if (enable != m_IsStencilTestEnabled)
		{
			if (enable) 
				glEnable(GL_STENCIL_TEST);
			else 
				glDisable(GL_STENCIL_TEST);
			
			m_IsStencilTestEnabled = enable;
		}

		// Set stencil params.
		if (stencilFunc != m_UsedStencilFunction || stencilTestMask != m_UsedStencilTestMask	|| stencilComparisonVal != m_UsedStencilComparisonValue)
		{
			glStencilFunc(stencilFunc, stencilTestMask, stencilComparisonVal);
			m_UsedStencilComparisonValue = stencilComparisonVal;
			m_UsedStencilTestMask = stencilTestMask;
			m_UsedStencilFunction = stencilFunc;
		}

		if (stencilFail != m_usedStencilFail || stencilPass != m_UsedStencilPass || stencilPassButDepthFail != m_UsedStencilPassButDepthFail) 
		{
			glStencilOp(stencilFail, stencilPassButDepthFail, stencilPass);
			m_usedStencilFail = stencilFail;
			m_UsedStencilPass = stencilPass;
			m_UsedStencilPassButDepthFail = stencilPassButDepthFail;
		}

		SetStencilWriteMask(stencilWriteMask);
	}

	void GLRenderEngine::SetStencilWriteMask(uint32 mask)
	{
		// Set write mask if a change is needed.
		if (m_UsedStencilWriteMask == mask) return;
		glStencilMask(mask);
		m_UsedStencilWriteMask = mask;

	}

	void GLRenderEngine::SetScissorTest(bool enable, uint32 startX, uint32 startY, uint32 width, uint32 height)
	{
		// Disable if enabled.
		if (!enable)
		{
			if (!m_IsScissorsTestEnabled) return;
			else
			{
				glDisable(GL_SCISSOR_TEST);
				m_IsScissorsTestEnabled = false;
				return;
			}
		}

		// Enable if disabled, then bind it.
		if (!m_IsScissorsTestEnabled) glEnable(GL_SCISSOR_TEST);
		glScissor(startX, startY, width, height);
		m_IsScissorsTestEnabled = true;
	}

	LinaString GLRenderEngine::GetShaderVersion()
	{
		// Return if not valid.
		if (!m_ShaderVersion.empty()) return m_ShaderVersion;

		// Check & set version according to data.
		uint32 version = GetVersion();

		if (version >= 330) 
			m_ShaderVersion = LinaEngine::Internal::ToString(version);
		else if (version >= 320) 
			m_ShaderVersion = "150";
		else if (version >= 310) 
			m_ShaderVersion = "140";
		else if (version >= 300) 
			m_ShaderVersion = "130";
		else if (version >= 210) 
			m_ShaderVersion = "120";
		else if (version >= 200) 
			m_ShaderVersion = "110";
		else 
		{
			int32 majorVersion = version / 100;
			int32 minorVersion = (version / 10) % 10;
			LINA_CORE_ERR( "Error: OpenGL Version {0}.{1} does not support shaders.\n", majorVersion, minorVersion);
			return "";
		}

		return m_ShaderVersion;
	}

	uint32 GLRenderEngine::GetVersion()
	{
		// Get version data from GL.
		if (m_GLVersion != 0) return m_GLVersion;
		int32 majorVersion;
		int32 minorVersion;

		glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
		glGetIntegerv(GL_MINOR_VERSION, &minorVersion);

		m_GLVersion = (uint32)(majorVersion * 100 + minorVersion * 10);
		return m_GLVersion;
	}

	// ---------------------------------------------------------------------
	// ---------------------------------------------------------------------
	// GLOBALS
	// ---------------------------------------------------------------------
	// ---------------------------------------------------------------------

	static GLint GetOpenGLFormat(PixelFormat format)
	{
		// Get target Open GL Pixel Format
		switch (format)
		{

		case PixelFormat::FORMAT_R: return GL_RED;
		case PixelFormat::FORMAT_RG: return GL_RG;
		case PixelFormat::FORMAT_RGB: return GL_RGB;
		case PixelFormat::FORMAT_RGBA: return GL_RGBA;
		case PixelFormat::FORMAT_DEPTH: return GL_DEPTH_COMPONENT;
		case PixelFormat::FORMAT_DEPTH_AND_STENCIL: return 0; // GL_DEPTH_STENCIL;

		default:
			LINA_CORE_ERR("PixelFormat {0} is not a valid PixelFormat.", format);
			return 0;
		};
	}

	static GLint GetOpenGLInternalFormat(PixelFormat format, bool compress)
	{
		// Get target OpenGL Internal Pixel Format
		switch (format)
		{

		case PixelFormat::FORMAT_R: return GL_RED;
		case PixelFormat::FORMAT_RG: return  GL_RG;
		case PixelFormat::FORMAT_RGB:
			if (compress) return GL_COMPRESSED_RGB_S3TC_DXT1_EXT; // GL_COMPRESSED_SRGB_S3TC_DXT1_EXT;
			else return GL_RGB;
		case PixelFormat::FORMAT_RGBA:
			if (compress)  return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT; // GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
			else  return GL_SRGB_ALPHA;
		case PixelFormat::FORMAT_DEPTH: return GL_DEPTH_COMPONENT;
		case PixelFormat::FORMAT_DEPTH_AND_STENCIL: return GL_DEPTH_STENCIL;
		default:
			LINA_CORE_ERR("PixelFormat {0} is not a valid PixelFormat.", format);
			return 0;
		};
	}


	static bool AddShader(GLuint shaderProgram, const LinaString& text, GLenum type, LinaArray<GLuint>* shaders)
	{
		// Create shader object.
		GLuint shader = glCreateShader(type);

		if (shader == 0)
		{
			LINA_CORE_ERR("Error creating shader type {0}", type);
			return false;
		}

		// Set appropriate length data.
		const GLchar* p[1];
		p[0] = text.c_str();
		GLint lengths[1];
		lengths[0] = (GLint)text.length();

		// Add the source & compile.
		glShaderSource(shader, 1, p, lengths);
		glCompileShader(shader);

		// Check error & report if exists.
		GLint success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			GLchar InfoLog[1024];
			glGetShaderInfoLog(shader, 1024, NULL, InfoLog);
			LINA_CORE_ERR("Error compiling shader type {0}: '{1}'\n", shader, InfoLog);
			return false;
		}

		// Attach the shader to program, store it & return.
		glAttachShader(shaderProgram, shader);
		shaders->push_back(shader);
		return true;
	}

	static bool CheckShaderError(GLuint shader, int flag, bool isProgram, const LinaString& errorMessage)
	{
		// Check shader errors from OpenGl.
		GLint success = 0;
		GLchar error[1024] = { 0 };

		if (isProgram) 
			glGetProgramiv(shader, flag, &success);
		else 
			glGetShaderiv(shader, flag, &success);
		
		// Get error info if exists.
		if (!success) 
		{
			if (isProgram) 
				glGetProgramInfoLog(shader, sizeof(error), NULL, error);
			else 
				glGetShaderInfoLog(shader, sizeof(error), NULL, error);

			LINA_CORE_ERR("{0}: '{1}'\n", errorMessage.c_str(), error);
			return true;
		}

		return false;
	}

	static void AddAllAttributes(GLuint program, const LinaString& vertexShaderText, uint32 version)
	{
		// Terminate if attribute layout feature is enabled.
		if (version >= 320) return;


		// FIXME: This code assumes attributes are listed in order, which isn't
		// true for all compilers. It's safe to ignore for now because OpenGL versions
		// requiring this aren't being used.
		GLint numActiveAttribs = 0;
		GLint maxAttribNameLength = 0;

		// Load attributes.
		glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &numActiveAttribs);
		glGetProgramiv(program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxAttribNameLength);

		// Iterate through the attributes.
		LinaArray<GLchar> nameData(maxAttribNameLength);
		for (GLint attrib = 0; attrib < numActiveAttribs; ++attrib)
		{
			GLint arraySize = 0;
			GLenum type = 0;
			GLsizei actualLength = 0;

			// Use appropriate attribute location.
			glGetActiveAttrib(program, attrib, nameData.size(), &actualLength, &arraySize, &type, &nameData[0]);
			glBindAttribLocation(program, attrib, (char*)&nameData[0]);
			
		}
	}

	static void AddShaderUniforms(GLuint shaderProgram, const LinaString& shaderText, LinaMap<LinaString, GLint>& uniformMap, LinaMap<LinaString, GLint>& samplerMap)
	{
		// Load uniform sets.
		GLint numBlocks;
		glGetProgramiv(shaderProgram, GL_ACTIVE_UNIFORM_BLOCKS, &numBlocks);

		// Iterate through sets.
		for (int32 block = 0; block < numBlocks; ++block) 
		{
			// Get uniform set data to store it in our map.
			GLint nameLen;
			glGetActiveUniformBlockiv(shaderProgram, block, GL_UNIFORM_BLOCK_NAME_LENGTH, &nameLen);
			LinaArray<GLchar> name(nameLen);
			glGetActiveUniformBlockName(shaderProgram, block, nameLen, NULL, &name[0]);
			LinaString uniformBlockName((char*)&name[0], nameLen - 1);
			uniformMap[uniformBlockName] = glGetUniformBlockIndex(shaderProgram, &name[0]);
		}

		// Load uniforms.
		GLint numUniforms = 0;
		glGetProgramiv(shaderProgram, GL_ACTIVE_UNIFORMS, &numBlocks);

		// Iterate through uniforms.
		LinaArray<GLchar> uniformName(256);
		for (int32 uniform = 0; uniform < numUniforms; ++uniform) 
		{
			GLint arraySize = 0;
			GLenum type = 0;
			GLsizei actualLength = 0;

			// Get sampler uniform data & store it on our sampler map.
			glGetActiveUniform(shaderProgram, uniform, uniformName.size(), &actualLength, &arraySize, &type, &uniformName[0]);

			if (type != GL_SAMPLER_2D)
			{
				LINA_CORE_ERR("Non-sampler2d uniforms currently unsupported!");
				continue;
			}

			LinaString name((char*)&uniformName[0], actualLength - 1);
			samplerMap[name] = glGetUniformLocation(shaderProgram, (char*)&uniformName[0]);
		}
	}


}

