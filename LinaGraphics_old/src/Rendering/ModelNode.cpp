#include "Rendering/ModelNode.hpp"
#include "Core/RenderEngine.hpp"
#include "Log/Log.hpp"
#include "Rendering/Material.hpp"
#include "Rendering/Model.hpp"
#include "Rendering/SkinnedMesh.hpp"
#include "Rendering/StaticMesh.hpp"
#include "Utility/AssimpUtility.hpp"
#include "Utility/ModelLoader.hpp"

#include <assimp/matrix4x4.h>
#include <assimp/scene.h>
#include "Data/Vector.hpp"

namespace Lina::Graphics
{
    ModelNode::~ModelNode()
    {
        LINA_TRACE("Deleting Node {0}", m_name);
        for (uint32 i = 0; i < m_meshes.size(); i++)
            delete m_meshes[i];

        m_children.clear();
        m_meshes.clear();
    }

    void ModelNode::FillNodeHierarchy(const aiNode* node, const aiScene* scene, Model* parentModel)
    {
        m_name                    = String(node->mName.C_Str());
        const String sidName = parentModel->GetPath() + m_name;
        m_localTransform          = AssimpToLinaMatrix(node->mTransformation);
        parentModel->m_allNodes.push_back(this);
        parentModel->m_numNodes++;
        m_nodeIndexInParentHierarchy = parentModel->m_numNodes - 1;

        const unsigned int numMeshes        = node->mNumMeshes;
        Vector3            currentMinBounds = Vector3(1000, 1000, 1000);
        Vector3            currentMaxBounds = Vector3(-1000, -1000, -1000);

        for (uint32 i = 0; i < numMeshes; i++)
        {
            aiMesh* aimesh    = scene->mMeshes[node->mMeshes[i]];
            Mesh*   addedMesh = nullptr;

            if (aimesh->HasBones())
            {
                SkinnedMesh* skinned = new SkinnedMesh();
                m_meshes.push_back(skinned);
                addedMesh = skinned;
            }
            else
            {
                StaticMesh* staticMesh = new StaticMesh();
                m_meshes.push_back(staticMesh);
                addedMesh = staticMesh;
            }

            parentModel->m_numVertices += aimesh->mNumVertices;
            parentModel->m_numBones += aimesh->mNumBones;
            ModelLoader::FillMeshData(aimesh, addedMesh);

            m_totalVertexCenter += addedMesh->GetVertexCenter();

            const Vector3 meshBoundsMin = addedMesh->GetAABB().m_boundsMin;
            const Vector3 meshBoundsMax = addedMesh->GetAABB().m_boundsMax;

            if (meshBoundsMin.x < currentMinBounds.x)
                currentMinBounds.x = meshBoundsMin.x;
            if (meshBoundsMax.x > currentMaxBounds.x)
                currentMaxBounds.x = meshBoundsMax.x;

            if (meshBoundsMin.y < currentMinBounds.y)
                currentMinBounds.y = meshBoundsMin.y;
            if (meshBoundsMax.y > currentMaxBounds.y)
                currentMaxBounds.y = meshBoundsMax.y;

            if (meshBoundsMin.z < currentMinBounds.z)
                currentMinBounds.z = meshBoundsMin.z;
            if (meshBoundsMax.z > currentMaxBounds.z)
                currentMaxBounds.z = meshBoundsMax.z;

            // Finally, construct the vertex array object for the mesh.
            addedMesh->CreateVertexArray(BufferUsage::USAGE_DYNAMIC_DRAW);
        }

        // Take the average of total bounds if we have any meshes for this node.
        if (numMeshes > 0)
        {

            const float numMeshesFloat = (float)numMeshes;
            m_totalVertexCenter /= numMeshesFloat;
            m_aabb.m_boundsMin         = currentMinBounds;
            m_aabb.m_boundsMax         = currentMaxBounds;
            m_aabb.m_boundsHalfExtents = (currentMaxBounds - currentMinBounds) / 2.0f;

            m_aabb.m_positions.resize(8);
            m_aabb.m_positions[0] = m_aabb.m_boundsMin;                                                        // bottom-left-back
            m_aabb.m_positions[1] = Vector3(m_aabb.m_boundsMin.x, m_aabb.m_boundsMin.y, m_aabb.m_boundsMax.z); // bottom-left-front
            m_aabb.m_positions[2] = Vector3(m_aabb.m_boundsMax.x, m_aabb.m_boundsMin.y, m_aabb.m_boundsMin.z); // bottom-right-back
            m_aabb.m_positions[3] = Vector3(m_aabb.m_boundsMax.x, m_aabb.m_boundsMin.y, m_aabb.m_boundsMax.z); // bottom-right-front
            m_aabb.m_positions[4] = Vector3(m_aabb.m_boundsMin.x, m_aabb.m_boundsMax.y, m_aabb.m_boundsMin.z); // top - left - back
            m_aabb.m_positions[5] = Vector3(m_aabb.m_boundsMin.x, m_aabb.m_boundsMax.y, m_aabb.m_boundsMax.z); // top - left - front
            m_aabb.m_positions[6] = Vector3(m_aabb.m_boundsMax.x, m_aabb.m_boundsMax.y, m_aabb.m_boundsMin.z); // top - right - back
            m_aabb.m_positions[7] = m_aabb.m_boundsMax;                                                        // top - right - front
        }

        // Recursively fill the other nodes.
        for (uint32 i = 0; i < node->mNumChildren; i++)
        {
            ModelNode* newNode = new ModelNode();
            m_children.push_back(newNode);
            newNode->FillNodeHierarchy(node->mChildren[i], scene, parentModel);
        }
    }
} // namespace Lina::Graphics