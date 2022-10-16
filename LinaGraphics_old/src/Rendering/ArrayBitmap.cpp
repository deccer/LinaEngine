#include "Rendering/ArrayBitmap.hpp"

#include "Log/Log.hpp"
#include "Memory/Memory.hpp"
#include "Utility/UtilityFunctions.hpp"
#include "Utility/stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "Utility/stb/stb_image_write.h"

namespace Lina::Graphics
{
    ArrayBitmap::~ArrayBitmap()
    {
        if (m_pixels != nullptr)
            stbi_image_free(m_pixels);

        if (m_hdriPixels != nullptr)
            stbi_image_free(m_hdriPixels);
    }

    int ArrayBitmap::Load(const String& fileName)
    {
        m_pixels = stbi_load(fileName.c_str(), &m_width, &m_height, &m_numComps, 0);
        LINA_ASSERT(m_pixels != nullptr, "Bitmap could not be loaded!");
        return m_numComps;
    }

    int ArrayBitmap::Load(unsigned char* data, size_t dataSize)
    {
        m_pixels = stbi_load_from_memory(data, (int)dataSize, &m_width, &m_height, &m_numComps, 0);
        LINA_ASSERT(m_pixels != nullptr, "Bitmap could not be loaded!");
        return m_numComps;
    }

    void ArrayBitmap::SetImageFlip(bool flip)
    {
        stbi_set_flip_vertically_on_load(flip);
        stbi_flip_vertically_on_write(flip);
    }

    void ArrayBitmap::Save(const String& path, int quality)
    {
        if (m_isHDRI)
            stbi_write_hdr(path.c_str(), m_width, m_height, m_numComps, m_hdriPixels);
        else
        {
            const String extension = Utility::GetFileExtension(path);

            if (extension.compare("png") == 0)
                stbi_write_png(path.c_str(), m_width, m_height, m_numComps, static_cast<const void*>(m_pixels), m_width * m_numComps);
            else
                stbi_write_jpg(path.c_str(), m_width, m_height, m_numComps, m_pixels, quality);

        }
    }

    int ArrayBitmap::LoadHDRIFromFile(const char* fileName)
    {
        m_isHDRI     = true;
        m_hdriPixels = stbi_loadf(fileName, &m_width, &m_height, &m_numComps, 0);
        LINA_ASSERT(m_hdriPixels != nullptr, "Bitmap could not be loaded!");
        return m_numComps;
    }

    int ArrayBitmap::LoadHDRIFromMemory(unsigned char* data, size_t dataSize)
    {
        m_isHDRI     = true;
        m_hdriPixels = stbi_loadf_from_memory(data, (int)dataSize, &m_width, &m_height, &m_numComps, 0);
        LINA_ASSERT(m_hdriPixels != nullptr, "Bitmap could not be loaded!");
        return m_numComps;
    }

} // namespace Lina::Graphics