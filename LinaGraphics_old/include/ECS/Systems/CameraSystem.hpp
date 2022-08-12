/*
Class: CameraSystem

Responsible for finding cameras in the level and updating the view, projection and other
matrices depending on the most active camera.

Timestamp: 5/2/2019 12:40:46 AM
*/

#pragma once

#ifndef CameraSystem_HPP
#define CameraSystem_HPP

#include "Core/CommonECS.hpp"
#include "ECS/System.hpp"
#include "Math/Matrix.hpp"

namespace Lina
{
    class Color;

    namespace Event
    {
        struct ELevelInstalled;
        struct EPlayModeChanged;
    } // namespace Event
} // namespace Lina

namespace Lina::ECS
{
    struct EntityDataComponent;
    struct CameraComponent;

    class CameraSystem : public System
    {
    public:
        CameraSystem() = default;
        void         Initialize(const String& name, float aspect);
        virtual void UpdateComponents(float delta) override;

        /// <summary>
        /// Changes the camera's view matrix to take effect next frame.
        /// </summary>
        /// <param name="view"></param>
        void InjectViewMatrix(const Matrix& view)
        {
            m_viewMatrixInjected = true;
            m_view               = view;
        }

        /// <summary>
        /// Changes the camera's projection matrix to take effect next frame.
        /// </summary>
        /// <param name="proj"></param>
        void InjectProjMatrix(const Matrix& proj)
        {
            m_projMatrixInjected = true;
            m_projection         = proj;
        }

        /// <summary>
        /// Changes the camera's view matrix for the current frame.
        /// </summary>
        /// <param name="view"></param>
        void SetViewMatrix(const Matrix& view)
        {
            m_view = view;
        }

        /// <summary>
        /// Changes the camera's projection matrix for the current frame.
        /// </summary>
        /// <param name="proj"></param>
        void SetProjectionMatrix(const Matrix& proj)
        {
            m_projection = proj;
        }

        /// <summary>
        /// Converts given screen-space coordinates to world-space coordinates.
        /// Z element is the distance from camera.
        /// (0,0) top-left, (screenSizeX, screenSizeY) bottom-right
        /// </summary>
        /// <param name="screenPos"></param>
        /// <returns></returns>
        static Vector3 ScreenToWorldCoordinates(const Vector3& screenPos);

        /// <summary>
        /// Converts given viewport-space coordinates to world-space coordinates.
        /// (0,0) top-left, (1,1) bottom-right
        /// </summary>
        /// <param name="viewport"></param>
        /// <returns></returns>
        static Vector3 ViewportToWorldCoordinates(const Vector3& viewport);

        /// <summary>
        /// Converts the given world-space coordinates to screen-space coordinates.
        /// (0,0) top-left, (screenSizeX, screenSizeY) bottom-right
        /// </summary>
        /// <param name="world"></param>
        /// <returns></returns>
        static Vector3 WorldToScreenCoordinates(const Vector3& world);

        /// <summary>
        /// Converts the given world-space coordinates to viewport coordinates.
        /// (0,0) top-left, (1,1) bottom-right
        /// </summary>
        /// <param name="world"></param>
        /// <returns></returns>
        static Vector3 WorldToViewportCoordinates(const Vector3& world);

        void SetActiveCamera(Entity cameraOwner);

        inline void SetAspectRatio(float aspect)
        {
            m_aspectRatio = aspect;
        }
        inline float GetAspectRatio()
        {
            return m_aspectRatio;
        }
        inline Entity GetActiveCamera()
        {
            return m_activeCameraEntity;
        }
        inline Matrix& GetViewMatrix()
        {
            return m_view;
        }
        inline Matrix& GetProjectionMatrix()
        {
            return m_projection;
        }
        Vector3          GetCameraLocation();
        Color&           GetCurrentClearColor();
        CameraComponent* GetActiveCameraComponent();

        void OnCameraDestroyed(entt::registry& registry, entt::entity entity)
        {
            if (entity == m_activeCameraEntity)
                m_activeCameraEntity = entt::null;
        }

        void OnLevelInstalled(const Event::ELevelInstalled& ev);
        void OnPlayModeChanged(const Event::EPlayModeChanged& ev);

    private:
        Matrix m_view               = Matrix::Identity();
        Matrix m_projection         = Matrix::Perspective(35, 1.33f, 0.01f, 1000.0f);
        float  m_aspectRatio        = 1.33f;
        bool   m_useDirLightView    = false;
        bool   m_viewMatrixInjected = false;
        bool   m_projMatrixInjected = false;
        Entity m_activeCameraEntity = entt::null;
    };
} // namespace Lina::ECS

#endif