/*
* Copyright (C) 2017-2018 by Godlike
* This code is licensed under the MIT license (MIT)
* (http://opensource.org/licenses/MIT)
*/
#include <pegasus/Scene.hpp>
#include <pegasus/Debug.hpp>
#include <pegasus/Force.hpp>
#include <pegasus/Integration.hpp>

namespace pegasus
{
namespace scene
{

void Scene::ComputeFrame(float duration)
{
    collision::ResolvePersistantContacts(m_assetManager, m_persistentContacts, duration);

    ApplyForces(forceDuration);

    Integrate(duration);

    m_currentContacts = collision::DetectContacts(m_assetManager);
    Debug::CollisionDetectionCall(m_currentContacts);

    collision::ResolveContacts(m_assetManager, m_persistentContacts, m_currentContacts, m_previousContacts, duration);
    m_previousContacts = std::move(m_currentContacts);
}

Handle Scene::MakeBody()
{
    return m_assetManager.MakeAsset(m_assetManager.GetBodies());
}

mechanics::Body& Scene::GetBody(Handle handle)
{
    return m_assetManager.GetAsset(m_assetManager.GetBodies(), handle);
}

void Scene::RemoveBody(Handle handle)
{
    m_assetManager.RemoveAsset(m_assetManager.GetBodies(), handle);
}

AssetManager& Scene::GetAssets()
{
    return m_assetManager;
}

void Scene::ApplyForces(float duration)
{
    //Clear previously applied forces
    for (Asset<mechanics::Body>& asset : m_assetManager.GetBodies())
    {
        asset.data.linearMotion.force = glm::vec3(0);
        asset.data.angularMotion.torque = glm::vec3(0);
    }

    //Reapply forces
    ApplyForce<force::StaticField>(duration);
    ApplyForce<force::SquareDistanceSource>(duration);
    ApplyForce<force::Drag>(duration);
    ApplyForce<force::Spring>(duration);
    ApplyForce<force::Bungee>(duration);
    ApplyForce<force::Buoyancy>(duration);
}

void Scene::Integrate(float duration)
{
    for (Asset<mechanics::Body>& asset : m_assetManager.GetBodies())
    {
        if (asset.id != ZERO_HANDLE)
        {
            integration::Integrate(asset.data, duration);
        }
    }

    UpdateShapes<DynamicBody, arion::Plane>();
    UpdateShapes<DynamicBody, arion::Sphere>();
    UpdateShapes<DynamicBody, arion::Box>();
}

} // namespace scene
} // namespace pegasus
