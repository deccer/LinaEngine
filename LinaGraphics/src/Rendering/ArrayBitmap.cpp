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

#define STB_IMAGE_IMPLEMENTATION 
#include "Rendering/ArrayBitmap.hpp"  
#include "Utility/stb/stb_image.h"
#include "Memory/Memory.hpp"

namespace Lina::Graphics
{


	ArrayBitmap::~ArrayBitmap()
	{
		if (m_pixels != nullptr)
			stbi_image_free(m_pixels);
	}

	int ArrayBitmap::Load(const std::string& fileName)
	{
		int32 texWidth, texHeight, nrComps;

		m_pixels = stbi_load(fileName.c_str(), &texWidth, &texHeight, &nrComps, 4);

		if (m_pixels == nullptr) {
			LINA_ERR("Bitmap with the name {0} could not be loaded!", fileName);
			return -1;
		}

		m_width = texWidth;
		m_height = texHeight;

		return nrComps;
	}

	int ArrayBitmap::Load(unsigned char* data, size_t dataSize)
	{
		int32 texWidth, texHeight, nrComps;
		m_pixels = stbi_load_from_memory(data, dataSize, &texWidth, &texHeight, &nrComps, 4);
		LINA_ASSERT(m_pixels != nullptr, "Bitmap could not be loaded!");
		m_width = texWidth;
		m_height = texHeight;
		return nrComps;
	}

	void ArrayBitmap::SetImageFlip(bool flip)
	{
		stbi_set_flip_vertically_on_load(flip);
	}

	unsigned char* ArrayBitmap::LoadImmediate(const char* filename, int& w, int& h, int& nrChannels)
	{
		return stbi_load(filename, &w, &h, &nrChannels, 0);
	}

	float* ArrayBitmap::LoadImmediateHDRI(const char* fileName, int& w, int& h, int& nrChannels)
	{
		return stbi_loadf(fileName, &w, &h, &nrChannels, 0);
	}

	float* ArrayBitmap::LoadImmediateHDRI(unsigned char* data, size_t dataSize, int& w, int& h, int& nrChannels)
	{
		return stbi_loadf_from_memory(data, dataSize, &w, &h, &nrChannels, 0);
	}

	bool ArrayBitmap::Free(unsigned char* data)
	{
		stbi_image_free(data);
		return true;
	}

	void ArrayBitmap::Clear(int32 color)
	{
		//Memory::memset(m_pixels, color, GetPixelsSize());
	}

}

