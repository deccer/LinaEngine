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

#include "Core/Editor.hpp"
#include "Resources/Core/CommonResources.hpp"
#include "Resources/Core/ResourceManager.hpp"
#include "Serialization/Serialization.hpp"
#include "FileSystem/FileSystem.hpp"
#include "Reflection/ReflectionSystem.hpp"
#include "Serialization/Compressor.hpp"

namespace Lina::Editor
{
    void Editor::Initialize()
    {
    }
    void Editor::Shutdown()
    {
    }

    void Editor::PackageResources(const Vector<ResourceIdentifier>& identifiers)
    {
        HashMap<PackageType, Vector<ResourceIdentifier>> resourcesPerPackage;

        for (const auto& ident : identifiers)
        {
            PackageType pt = ResourceManager::Get().GetPackageType(ident.tid);
            resourcesPerPackage[pt].push_back(ident);
        }

        for (auto [packageType, resources] : resourcesPerPackage)
        {
            const String packagePath = GGetPackagePath(packageType);
            OStream      stream;
            stream.CreateReserve(1000);

            for (auto r : resources)
            {
                // Header
                stream << r.tid;
                stream << r.sid;

                const String metacachePath = ResourceManager::GetMetacachePath(r.path, r.sid);
                if (FileSystem::FileExists(metacachePath))
                {
                    IStream      cache = Serialization::LoadFromFile(metacachePath.c_str());
                    const uint32 size  = static_cast<uint32>(cache.GetSize());
                    stream << size;
                    stream.WriteEndianSafe(cache.GetDataRaw(), cache.GetSize());
                    cache.Destroy();
                }
                else
                {
                    OStream outStream;
                    outStream.CreateReserve(512);

                    // Load into stream & destroy.
                    IResource* res = static_cast<IResource*>(ReflectionSystem::Get().Resolve(r.tid).GetFunction<void*()>("CreateMock"_hs)());
                    res->LoadFromFile(r.path.c_str());
                    res->SaveToStream(outStream);
                    ReflectionSystem::Get().Resolve(r.tid).GetFunction<void(void*)>("DestroyMock"_hs)(static_cast<void*>(res));

                    // Write stream to package & destroy.
                    const uint32 size = static_cast<uint32>(outStream.GetCurrentSize());
                    stream << size;
                    stream.WriteRaw(outStream.GetDataRaw(), outStream.GetCurrentSize());
                    outStream.Destroy();
                }
            }

            if (!FileSystem::FileExists("Resources/Packages"))
                FileSystem::CreateFolderInPath("Resources/Packages");

            Serialization::SaveToFile(packagePath.c_str(), stream);
        }
    }

} // namespace Lina::Editor
