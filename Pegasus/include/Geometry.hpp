/*
* Copyright (C) 2017 by Godlike
* This code is licensed under the MIT license (MIT)
* (http://opensource.org/licenses/MIT)
*/
#ifndef PEGASUS_GEOMETRY_HPP
#define PEGASUS_GEOMETRY_HPP

#include <Pegasus/include/Math.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
#include <glm/gtx/optimum_pow.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>

#include <algorithm>
#include <utility>
#include <limits>
#include <memory>
#include <array>
#include <vector>
#include <unordered_map>

namespace pegasus
{
namespace geometry
{
/**
 * @brief Base shape class
 */
class Shape
{
public:
    glm::dvec3 centerOfMass;

    Shape() = default;

    explicit Shape(glm::dvec3 const& centerOfMass);
};

/**
 * @brief Generic class for representing shapes that could be described as a simple parametric or quadric surface
 */
class SimpleShape : public Shape
{
public:
    enum class Type : uint8_t
    {
        RAY,
        PLANE,
        TRIANGLE,
        SPHERE,
        CONE,
        CYLINDER,
        CAPSULE,
        BOX,
        NONE
    };

    Type type = Type::NONE;

    SimpleShape() = default;

    explicit SimpleShape(Type type)
        : type(type)
    {
    }

    SimpleShape(glm::dvec3 const& centerOfMass, Type type);
};

/** Ray data storage class */
class Ray : public SimpleShape
{
public:
    glm::dvec3 direction;

    Ray();

    Ray(glm::dvec3 const& centerOfMass, glm::dvec3 const& normal);
};

/** Plane data storage class */
class Plane : public SimpleShape
{
public:
    glm::dvec3 normal;

    Plane();

    Plane(glm::dvec3 const& centerOfMass, glm::dvec3 const& normal);
};

/** Triangle data storage class */
class Triangle : public SimpleShape
{
public:
    glm::dvec3 aVertex;
    glm::dvec3 bVertex;
    glm::dvec3 cVertex;
    glm::dvec3 normal;

    Triangle();

    Triangle(glm::dvec3 const& centerOfMass, glm::dvec3 const& a, glm::dvec3 const& b, glm::dvec3 const& c);

    /** Calculates normal from member vertices and writes it to the normal member field */
    void CalculateNormal();
};

/** Sphere data storage class */
class Sphere : public SimpleShape
{
public:
    double radius;

    Sphere();

    Sphere(glm::dvec3 const& centerOfMass, double r);
};

/** Cone data storage class */
class Cone : public SimpleShape
{
public:
    glm::dvec3 apex;
    double radius;

    Cone();

    Cone(glm::dvec3 const& centerOfMass, glm::dvec3 const& a, double r);
};

/** Capsule data storage class */
class Capsule : public SimpleShape
{
public:
    glm::dvec3 halfHeight;
    double radius;

    Capsule();

    Capsule(glm::dvec3 const& centerOfMass, glm::dvec3 const& halfHeight, double r);
};

/** Cylinder data storage class */
class Cylinder : public SimpleShape
{
public:
    glm::dvec3 halfHeight;
    double radius;

    Cylinder();

    Cylinder(glm::dvec3 const& centerOfMass, glm::dvec3 const& halfHeight, double r);
};

/** Box data storage class */
class Box : public SimpleShape
{
public:
    glm::dvec3 iAxis;
    glm::dvec3 jAxis;
    glm::dvec3 kAxis;

    Box();

    Box(glm::dvec3 const& centerOfMass, glm::dvec3 const& i, glm::dvec3 const& j, glm::dvec3 const& k);
};

namespace intersection
{
/** Base cache data structure for shapes intersection queries */
struct CacheBase
{
};

/** Specialized cache data structure for shapes intersection queries */
template <typename ShapeA, typename ShapeB>
struct Cache : CacheBase
{
};

template <>
struct Cache<Ray, Ray> : CacheBase
{
    double denominator;
    glm::dvec3 aClosestApproach;
    glm::dvec3 bClosestApproach;
};

template <>
struct Cache<Ray, Plane> : CacheBase
{
    glm::dvec3 contact;
};

template <>
struct Cache<Ray, Sphere> : CacheBase
{
    glm::dvec3 sphereContactNormal;
    bool intersection;
    glm::dvec3 inPoint;
    glm::dvec3 outPoint;
};

template <>
struct Cache<Ray, Box> : CacheBase
{
    glm::dvec3 rayDirectionBoxSpace;
    glm::dvec3 rayOriginBoxSpace;
    glm::dmat3 boxModelMatrix;
    glm::dvec3 aabbMaxPoint;
    glm::dvec3 aabbMinPoint;
    glm::dvec3 inPoint;
    glm::dvec3 outPoint;
    glm::dvec3 boxContactNormal;
};

template <>
struct Cache<Plane, Ray> : CacheBase
{
    Cache<Ray, Plane> rpCache;
};

template <>
struct Cache<Plane, Plane> : CacheBase
{
};

template <>
struct Cache<Plane, Sphere> : CacheBase
{
    double penetration;
};

template <>
struct Cache<Plane, Box> : CacheBase
{
    std::array<glm::dvec3, 6> boxFaces;
    std::array<double, 6> boxFaceDistances;
    std::array<double, 8> boxPenetrations;
};

template <>
struct Cache<Sphere, Ray> : CacheBase
{
    Cache<Ray, Sphere> rsCache;
};

template <>
struct Cache<Sphere, Plane> : CacheBase
{
    Cache<Plane, Sphere> psCache;
};

template <>
struct Cache<Sphere, Sphere> : CacheBase
{
    glm::dvec3 baVector;
    double radiusSum;
};

template <>
struct Cache<Sphere, Box> : CacheBase
{
    std::array<glm::dvec3, 6> boxAxes;
    std::array<glm::dvec3, 6> boxNormals;
    std::array<glm::dvec3, 6> boxFaceCenterVertices;
    std::array<double, 6> boxFaceDistances;
    std::array<glm::dvec3, 8> boxVertices;
    std::array<double, 8> boxVerticesProjections;
    std::array<glm::dvec3, 4> separatingAxes;
    glm::dvec3 boxContactNormal;
    glm::dvec3 boxSphereVector;
    glm::dvec3 sphereContactNormal;
    glm::dvec3 boxContactPoint;
    glm::dvec3 sphereContactPoint;
};

template <>
struct Cache<Box, Ray> : CacheBase
{
    Cache<Ray, Box> rbCache;
};

template <>
struct Cache<Box, Plane> : CacheBase
{
    Cache<Plane, Box> pbCache;
    glm::dvec3 planeMassCenter;
    glm::dvec3 boxContactNormal;
};

template <>
struct Cache<Box, Sphere> : CacheBase
{
    Cache<Sphere, Box> sbCache;
};

template <>
struct Cache<Box, Box> : CacheBase
{
    std::array<glm::dvec3, 8> aBoxVertices, bBoxVertices;
    std::array<glm::dvec3, 6> aBoxAxes, bBoxAxes;
    std::array<glm::dvec3, 6> aBoxFaces, bBoxFaces;
    std::vector<glm::dvec3> separatingAxes;
    std::array<double, 6> aBoxFaceDistances, bBoxFaceDistances;
    std::array<double, 8> aBoxVerticesDistances, bBoxVerticesDistances;
    glm::dvec3 contactNormal;
    double penetration = 0;
};

/** Data structure to store ray factors for Ray, Sphere collisions */
struct RayIntersectionFactors
{
    double tMin;
    double tMax;
};

/**
* @brief Calculates wether a ray and a sphere are intersecting
* @param[in] raySphere vector from the ray to the sphere center
* @param[in] sphereRadius radius of the sphere
* @param[in] rayDirection normalized direction vector of the ray
* @return @c true if there is intersection, @c false otherwise
*/
bool CalculateRaySphereIntersection(
    glm::dvec3 const& raySphere, double sphereRadius, glm::dvec3 const& rayDirection
);

/**
* @brief Calculates ray factors for the sphere intersection points
*
* Must be called if and only if the ray and the sphere are intersecting
* @param[in] raySphere vector from the ray to the sphere center
* @param[in] sphereRadius radius of the sphere
* @param[in] rayDirection normalized direction vector of the ray
* @return ray intersection factors
*/
RayIntersectionFactors CalculateRaySphereIntersectionFactors(
    glm::dvec3 const& raySphere, double sphereRadius, glm::dvec3 const& rayDirection
);

/** Data structure for storing AABB using minimum and maximum points */
struct AabbExtremalVertices
{
    glm::dvec3 minVertex;
    glm::dvec3 maxVertex;
};

/**
* @brief Calculates ray intersection factors for AABB, Ray collision
* @param[in] boxMinPoint min point of AABB
* @param[in] boxMaxPoint max point of AABB
* @param[in] rayDirection normalized direction vector
* @param[in] rayOrigin ray origin
* @return in and out point factors
*/
RayIntersectionFactors CalculateRayAabbIntersectionFactors(
    glm::dvec3 const& boxMinPoint, glm::dvec3 const& boxMaxPoint, 
    glm::dvec3 const& rayDirection, glm::dvec3 const& rayOrigin
);

/**
 * @brief Calculates AABB min and max points from the given OBB basis
 * @param[in] i vector from an orthogonal basis
 * @param[in] j vector from an orthogonal basis
 * @param[in] k vector from an orthogonal basis
 * @return AABB min and max points
 */
AabbExtremalVertices MakeExtremalVerticesAabb(
    glm::dvec3 const& i, glm::dvec3 const& j, glm::dvec3 const& k
);

/**
 * @brief Calculates wether a ray and an aabb are intersecting
 * @param[in] tMin min ray intersection factor
 * @param[in] tMax max ray intersection factor
 * @return @c true if there is intersection, @c false otherwise
 */
bool CalculateRayAabbIntersection(double tMin, double tMax);

/**
 * @brief Performs intersection test using shapes and cache data and returns true if shapes are intersecting
 * @tparam ShapeA SimpleShape or SimpleShape derived object
 * @tparam ShapeB SimpleShape or SimpleShape derived object
 * @param[in] a pointer to the ShapeA type object
 * @param[in] b pointer to the ShapeB type object
 * @param[in, out] cacheBase pointer to the IntersectionCacheBase or IntersectionCacheBase derived object 
 * @return @c true if there is intersection, @c false otherwise
 */
template <typename ShapeA, typename ShapeB>
bool CalculateIntersection(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase);

/**
 * @brief Calculates surface contact normal of b shape using shapes and cache data and returns it
 * 
 * Must be called strictly after the corresponding CalculateIntersection function call, otherwise result is undefined
 * @tparam ShapeA SimpleShape or SimpleShape derived object
 * @tparam ShapeB SimpleShape or SimpleShape derived object
 * @param[in] a pointer to the ShapeA type object
 * @param[in] b pointer to the ShapeB type object
 * @param[in, out] cacheBase pointer to the IntersectionCacheBase or IntersectionCacheBase derived object 
 * @return surface contact normal of the b object
 */
template <typename ShapeA, typename ShapeB>
glm::dvec3 CalculateContactNormal(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase);

/**
 * @brief Calculates penetration depth using shapes and cache data and returns it
 * 
 * Must be called strictly after the corresponding CalculateContactNormal function call, 
 * otherwise result is undefined
 * @tparam ShapeA SimpleShape or SimpleShape derived object
 * @tparam ShapeB SimpleShape or SimpleShape derived object
 * @param[in] a pointer to the ShapeA type object
 * @param[in] b pointer to the ShapeB type object
 * @param[in, out] cacheBase pointer to the IntersectionCacheBase or IntersectionCacheBase derived object 
 * @return penetration depth
 */
template <typename ShapeA, typename ShapeB>
double CalculatePenetration(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase);

/** Ray, Ray CalculateIntersection specialization */
template <>
inline bool CalculateIntersection<Ray, Ray>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto cache = static_cast<Cache<Ray, Ray>*>(cacheBase);
    auto aRay = static_cast<Ray const*>(a);
    auto bRay = static_cast<Ray const*>(b);

    glm::dmat3 const aNumerator{
        bRay->centerOfMass - aRay->centerOfMass,
        bRay->direction,
        glm::cross(aRay->centerOfMass, bRay->centerOfMass)
    };
    glm::dmat3 const bNumerator{
        bRay->centerOfMass - aRay->centerOfMass,
        aRay->direction,
        glm::cross(aRay->centerOfMass, bRay->centerOfMass)
    };
    cache->denominator = glm::length2(aNumerator[2]);

    if (cache->denominator == 0.0)
    {
        return false;
    }

    cache->aClosestApproach =
        aRay->centerOfMass + aNumerator / cache->denominator * aRay->direction;
    cache->bClosestApproach =
        bRay->centerOfMass + bNumerator / cache->denominator * bRay->direction;

    return glm::length2(cache->aClosestApproach - cache->bClosestApproach) < 1e-10;
}

/** Ray, Ray CalculateContactNormal specialization */
template <>
inline glm::dvec3 CalculateContactNormal<Ray, Ray>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto aRay = static_cast<Ray const*>(a);
    auto bRay = static_cast<Ray const*>(b);

    return glm::normalize(glm::cross(glm::cross(bRay->direction, aRay->direction), bRay->direction));
}

/** Ray, Ray CalculatePenetration specialization */
template <>
inline double CalculatePenetration<Ray, Ray>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    return 0;
}

/** Ray, Plane CalculateIntersection specialization */
template <>
inline bool CalculateIntersection<Ray, Plane>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto cache = static_cast<Cache<Ray, Plane>*>(cacheBase);
    auto ray = static_cast<Ray const*>(a);
    auto plane = static_cast<Plane const*>(b);

    math::HyperPlane hyperPlane{ plane->normal, plane->centerOfMass };

    return hyperPlane.RayIntersection(ray->direction, ray->centerOfMass, cache->contact);
}

/** Ray, Plane CalculateContactNormal specialization */
template <>
inline glm::dvec3 CalculateContactNormal<Ray, Plane>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto ray = static_cast<Ray const*>(a);
    return ray->direction;
}

/** Ray, Plane CalculatePenetration specialization */
template <>
inline double CalculatePenetration<Ray, Plane>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    return std::numeric_limits<double>::max();
}

/** Ray, Sphere CalculateIntersection specialization */
template <>
inline bool CalculateIntersection<Ray, Sphere>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto cache = static_cast<Cache<Ray, Sphere>*>(cacheBase);
    auto ray = static_cast<Ray const*>(a);
    auto sphere = static_cast<Sphere const*>(b);

    cache->intersection = CalculateRaySphereIntersection(
        sphere->centerOfMass - ray->centerOfMass, sphere->radius, ray->direction
    );

    return cache->intersection;
}

/** Ray, Sphere CalculateContactNormal specialization */
template <>
inline glm::dvec3 CalculateContactNormal<Ray, Sphere>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto cache = static_cast<Cache<Ray, Sphere>*>(cacheBase);
    auto ray = static_cast<Ray const*>(a);
    auto sphere = static_cast<Sphere const*>(b);

    if (cache->intersection)
    {
        RayIntersectionFactors intersectionFactors = CalculateRaySphereIntersectionFactors(
            sphere->centerOfMass - ray->centerOfMass, sphere->radius, ray->direction
        );

        cache->inPoint = ray->centerOfMass + ray->direction * intersectionFactors.tMin;
        cache->outPoint = ray->centerOfMass + ray->direction * intersectionFactors.tMax;
    }

    return glm::normalize(cache->inPoint - sphere->centerOfMass);
}

/** Ray, Sphere CalculatePenetration specialization */
template <>
inline double CalculatePenetration<Ray, Sphere>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto cache = static_cast<Cache<Ray, Sphere>*>(cacheBase);
    return glm::length(cache->inPoint - cache->outPoint);
}

/** Ray, Box CalculateIntersection specialization */
template <>
inline bool CalculateIntersection<Ray, Box>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto cache = static_cast<Cache<Ray, Box>*>(cacheBase);
    auto ray = static_cast<Ray const*>(a);
    auto box = static_cast<Box const*>(b);

    //Transforming OBB into AABB, and moving ray into AABB space
    cache->boxModelMatrix = glm::dmat3{
        glm::normalize(box->iAxis), glm::normalize(box->jAxis), glm::normalize(box->kAxis)
    };
    glm::dmat3 const boxModelMatrixInverse = glm::inverse(cache->boxModelMatrix);
    cache->rayDirectionBoxSpace = boxModelMatrixInverse * ray->direction;
    cache->rayOriginBoxSpace = boxModelMatrixInverse * (ray->centerOfMass - box->centerOfMass);

    AabbExtremalVertices aabb = MakeExtremalVerticesAabb(box->iAxis, box->jAxis, box->kAxis);
    cache->aabbMinPoint = aabb.minVertex;
    cache->aabbMaxPoint = aabb.maxVertex;

    RayIntersectionFactors rayFactors = CalculateRayAabbIntersectionFactors(
        cache->aabbMinPoint, cache->aabbMaxPoint, cache->rayDirectionBoxSpace, cache->rayOriginBoxSpace
    );

    //Calculating intersection points in the obb model space
    cache->inPoint = cache->rayOriginBoxSpace + cache->rayDirectionBoxSpace * rayFactors.tMin;
    cache->outPoint = cache->rayOriginBoxSpace + cache->rayDirectionBoxSpace * rayFactors.tMax;

    return CalculateRayAabbIntersection(rayFactors.tMin, rayFactors.tMax);
}

/** Ray, Box CalculateContactNormal specialization */
template <>
inline glm::dvec3 CalculateContactNormal<Ray, Box>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto cache = static_cast<Cache<Ray, Box>*>(cacheBase);
    auto box = static_cast<Box const*>(b);

    std::array<double, 6> const faces = {
        cache->aabbMaxPoint[0], cache->aabbMaxPoint[1], cache->aabbMaxPoint[2],
        cache->aabbMinPoint[0], cache->aabbMinPoint[1], cache->aabbMinPoint[2]
    };

    std::array<double, 6> const deltas = {
        faces[0] - cache->inPoint[0], faces[1] - cache->inPoint[1], faces[2] - cache->inPoint[2],
        faces[3] - cache->inPoint[0], faces[4] - cache->inPoint[1], faces[5] - cache->inPoint[2],
    };

    size_t const contactFaceIndex = std::distance(deltas.begin(),
        std::min_element(deltas.begin(), deltas.end(),
            [](double a, double b) -> bool
        {
            return glm::abs(a) < glm::abs(b);
        }
    ));

    //Transforming intersection points in the world space
    cache->inPoint = cache->boxModelMatrix * cache->inPoint + box->centerOfMass;
    cache->outPoint = cache->boxModelMatrix * cache->outPoint + box->centerOfMass;

    //Calculating box contact normal
    cache->boxContactNormal = {};
    cache->boxContactNormal[contactFaceIndex % 3] = faces[contactFaceIndex];
    cache->boxContactNormal = cache->boxModelMatrix * cache->boxContactNormal;

    return cache->boxContactNormal;
}

/** Ray, Box CalculatePenetration specialization */
template <>
inline double CalculatePenetration<Ray, Box>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto cache = static_cast<Cache<Ray, Box>*>(cacheBase);
    return glm::length(cache->outPoint - cache->inPoint);
}

/** Plane, Ray CalculateIntersection specialization */
template <>
inline bool CalculateIntersection<Plane, Ray>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto cache = static_cast<Cache<Plane, Ray>*>(cacheBase);
    return CalculateIntersection<Ray, Plane>(b, a, &cache->rpCache);
}

/** Plane, Ray CalculateContactNormal specialization */
template <>
inline glm::dvec3 CalculateContactNormal<Plane, Ray>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto ray = static_cast<Ray const*>(b);
    return -ray->direction;
}

/** Plane, Ray CalculatePenetration specialization */
template <>
inline double CalculatePenetration<Plane, Ray>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto cache = static_cast<Cache<Plane, Ray>*>(cacheBase);
    return CalculatePenetration<Ray, Plane>(b, a, &cache->rpCache);
}

/** Plane, Plane CalculateIntersection specialization */
template <>
inline bool CalculateIntersection<Plane, Plane>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto aPlane = static_cast<Plane const*>(a);
    auto bPlane = static_cast<Plane const*>(b);
    
    return glm::length2(glm::cross(aPlane->normal, bPlane->normal)) != 0;
}

/** Plane, Plane CalculateContactNormal specialization */
template <>
inline glm::dvec3 CalculateContactNormal<Plane, Plane>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto bPlane = static_cast<Plane const*>(b);
    return bPlane->normal;
}

/** Plane, Plane CalculatePenetration specialization */
template <>
inline double CalculatePenetration<Plane, Plane>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    return std::numeric_limits<double>::max();
}

/** Plane, Sphere CalculateIntersection specialization */
template <>
inline bool CalculateIntersection<Plane, Sphere>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto plane = static_cast<Plane const*>(a);
    auto sphere = static_cast<Sphere const*>(b);
    auto cache = static_cast<Cache<Plane, Sphere>*>(cacheBase);

    cache->penetration = sphere->radius -
        (glm::dot(sphere->centerOfMass, plane->normal) - glm::dot(plane->centerOfMass, plane->normal));;

    return cache->penetration >= 0.0;
}

/** Plane, Sphere CalculateContactNormal specialization */
template <>
inline glm::dvec3 CalculateContactNormal<Plane, Sphere>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto plane = static_cast<Plane const*>(a);
    return -plane->normal;
}

/** Plane, Sphere CalculatePenetration specialization */
template <>
inline double CalculatePenetration<Plane, Sphere>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto cache = static_cast<Cache<Plane, Sphere>*>(cacheBase);
    return cache->penetration;
}

/** Plane, Box CalculateIntersection specialization */
template <>
inline bool CalculateIntersection<Plane, Box>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto plane = static_cast<Plane const*>(a);
    auto box = static_cast<Box const*>(b);
    auto cache = static_cast<Cache<Plane, Box>*>(cacheBase);

    cache->boxFaces = { box->iAxis, box->jAxis, box->kAxis, -box->iAxis, -box->jAxis, -box->kAxis };

    std::array<glm::dvec3, 8> boxVertices;
    math::CalculateBoxVertices(box->iAxis, box->jAxis, box->kAxis, boxVertices.begin());
    for (glm::dvec3& veretex : boxVertices) 
    {
        veretex += box->centerOfMass;
    }

    double const planeDistance = glm::dot(plane->centerOfMass, plane->normal);
    std::transform(boxVertices.begin(), boxVertices.end(), cache->boxPenetrations.begin(),
        [planeDistance, plane](glm::dvec3 const& p) -> double
    {
        return planeDistance - glm::dot(p, plane->normal);
    });
    std::sort(cache->boxPenetrations.begin(), cache->boxPenetrations.end());

    return cache->boxPenetrations.back() >= 0.0;
}

/** Plane, Box CalculateContactNormal specialization */
template <>
inline glm::dvec3 CalculateContactNormal<Plane, Box>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto cache = static_cast<Cache<Plane, Box>*>(cacheBase);
    auto plane = static_cast<Plane const*>(a);

    std::transform(cache->boxFaces.begin(), cache->boxFaces.end(), cache->boxFaceDistances.begin(),
        [plane](glm::dvec3 const& v) -> double
    {
        return glm::dot(v, plane->normal);
    });
    size_t const minIndex = std::distance(cache->boxFaceDistances.begin(),
        std::min_element(cache->boxFaceDistances.begin(), cache->boxFaceDistances.end())
    );

    return glm::normalize(cache->boxFaces[minIndex]);
}

/** Plane, Box CalculatePenetration specialization */
template <>
inline double CalculatePenetration<Plane, Box>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto cache = static_cast<Cache<Plane, Box>*>(cacheBase);
    return cache->boxPenetrations.back();
}

/** Sphere, Plane CalculateIntersection specialization */
template <>
inline bool CalculateIntersection<Sphere, Plane>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto cache = static_cast<Cache<Sphere, Plane>*>(cacheBase);
    return CalculateIntersection<Plane, Sphere>(b, a, &cache->psCache);
}

/** Sphere, Plane CalculateContactNormal specialization */
template <>
inline glm::dvec3 CalculateContactNormal<Sphere, Plane>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto cache = static_cast<Cache<Sphere, Plane>*>(cacheBase);
    return -CalculateContactNormal<Plane, Sphere>(b, a, &cache->psCache);
}

/** Sphere, Plane CalculatePenetration specialization */
template <>
inline double CalculatePenetration<Sphere, Plane>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto cache = static_cast<Cache<Sphere, Plane>*>(cacheBase);
    return CalculatePenetration<Plane, Sphere>(b, a, &cache->psCache);
}

/** Sphere, Ray CalculateIntersection specialization */
template <>
inline bool CalculateIntersection<Sphere, Ray>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto cache = static_cast<Cache<Sphere, Ray>*>(cacheBase);
    return CalculateIntersection<Ray, Sphere>(b, a, &cache->rsCache);
}

/** Sphere, Ray CalculateContactNormal specialization */
template <>
inline glm::dvec3 CalculateContactNormal<Sphere, Ray>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto ray = static_cast<Ray const*>(b);
    return -ray->direction;
}

/** Sphere, Ray CalculatePenetration specialization */
template <>
inline double CalculatePenetration<Sphere, Ray>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto cache = static_cast<Cache<Sphere, Ray>*>(cacheBase);
    return CalculatePenetration<Ray, Sphere>(b, a, &cache->rsCache);
}

/** Sphere, Sphere CalculateIntersection specialization */
template <>
inline bool CalculateIntersection<Sphere, Sphere>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto cache = static_cast<Cache<Sphere, Sphere>*>(cacheBase);
    auto aSphere = static_cast<Sphere const*>(a);
    auto bSphere = static_cast<Sphere const*>(b);

    cache->baVector = aSphere->centerOfMass - bSphere->centerOfMass;
    cache->radiusSum = aSphere->radius + bSphere->radius;

    return glm::pow2(cache->radiusSum) > glm::length2(cache->baVector);
}

/** Sphere, Sphere CalculateContactNormal specialization */
template <>
inline glm::dvec3 CalculateContactNormal<Sphere, Sphere>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto cache = static_cast<Cache<Sphere, Sphere>*>(cacheBase);
    return glm::normalize(cache->baVector);
}

/** Sphere, Sphere CalculatePenetration specialization */
template <>
inline double CalculatePenetration<Sphere, Sphere>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto cache = static_cast<Cache<Sphere, Sphere>*>(cacheBase);
    return cache->radiusSum - glm::length(cache->baVector);
}

/** Sphere, Box CalculateIntersection specialization */
template <>
inline bool CalculateIntersection<Sphere, Box>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto cache = static_cast<Cache<Sphere, Box>*>(cacheBase);
    auto sphere = static_cast<Sphere const*>(a);
    auto box = static_cast<Box const*>(b);

    math::CalculateBoxVertices(box->iAxis, box->jAxis, box->kAxis, cache->boxVertices.begin());
    for (glm::dvec3& vertex : cache->boxVertices)
    {
        vertex += box->centerOfMass;
    }

    cache->boxAxes = { box->iAxis, box->jAxis, box->kAxis, -box->iAxis, -box->jAxis, -box->kAxis };
    cache->boxSphereVector = sphere->centerOfMass - box->centerOfMass;

    for (uint8_t i = 0; i < cache->boxNormals.size(); ++i)
    {
        cache->boxFaceCenterVertices[i] = cache->boxAxes[i] + box->centerOfMass;
        cache->boxNormals[i] = glm::normalize(cache->boxAxes[i]);
    }

    if (glm::length2(cache->boxSphereVector))
    {
        cache->boxContactPoint = box->centerOfMass;

        for (uint8_t i = 0; i < 3; ++i)
        {
            double d = glm::dot(cache->boxSphereVector, cache->boxNormals[i]);
            double const axisNorm = glm::length(cache->boxAxes[i]);

            if (d > axisNorm)
{
                d = axisNorm;
            }
            else if (d < -axisNorm)
            {
                d = -axisNorm;
            }

            cache->boxContactPoint += cache->boxNormals[i] * d;
        }
    }
    else
    {
        cache->boxContactPoint = cache->boxFaceCenterVertices.front();
        cache->boxSphereVector = cache->boxAxes.front();
    }

    cache->sphereContactNormal = glm::normalize(cache->boxContactPoint - sphere->centerOfMass);
    cache->sphereContactPoint = cache->sphereContactNormal * sphere->radius + sphere->centerOfMass;

    return glm::length2(sphere->centerOfMass - cache->boxContactPoint) <= glm::pow2(sphere->radius);
}

/** Sphere, Box CalculateContactNormal specialization */
template <>
inline glm::dvec3 CalculateContactNormal<Sphere, Box>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto cache = static_cast<Cache<Sphere, Box>*>(cacheBase);
    auto sphere = static_cast<Sphere const*>(a);
    auto box = static_cast<Box const*>(b);

    for (uint8_t i = 0; i < cache->boxFaceDistances.size(); ++i)
    {
        cache->boxFaceDistances[i] = glm::length(cache->boxFaceCenterVertices[i] - sphere->centerOfMass);
    }

    size_t const minIndex = std::distance(cache->boxFaceDistances.begin(), 
        std::min_element(cache->boxFaceDistances.begin(), cache->boxFaceDistances.end())
    );
    cache->boxContactNormal = cache->boxNormals[minIndex];

    if (cache->boxContactPoint == sphere->centerOfMass)
    {
        cache->boxContactPoint = box->centerOfMass + glm::normalize(cache->boxSphereVector)
            * glm::dot(cache->boxAxes[minIndex], glm::normalize(cache->boxSphereVector));
        cache->sphereContactNormal = glm::normalize(cache->boxContactPoint - sphere->centerOfMass);
        cache->sphereContactPoint = cache->sphereContactNormal * sphere->radius + sphere->centerOfMass;
    }

    return cache->boxContactNormal;
}

/** Sphere, Box CalculatePenetration specialization */
template <>
inline double CalculatePenetration<Sphere, Box>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto cache = static_cast<Cache<Sphere, Box>*>(cacheBase);
    auto box = static_cast<Box const*>(b);

    if (cache->boxContactPoint == box->centerOfMass)
    {
        return glm::length(cache->boxAxes.front());
    }

    return glm::length(cache->sphereContactPoint - cache->boxContactPoint);
}

/** Box, Ray CalculateIntersection specialization */
template <>
inline bool CalculateIntersection<Box, Ray>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto cache = static_cast<Cache<Box, Ray>*>(cacheBase);
    return CalculateIntersection<Ray, Box>(b, a, &cache->rbCache);
}

/** Box, Ray CalculateContactNormal specialization */
template <>
inline glm::dvec3 CalculateContactNormal<Box, Ray>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto cache = static_cast<Cache<Box, Ray>*>(cacheBase);
    auto ray = static_cast<Ray const*>(b);
    CalculateContactNormal<Ray, Box>(b, a, &cache->rbCache);
    return ray->direction;
}

/** Box, Ray CalculatePenetration specialization */
template <>
inline double CalculatePenetration<Box, Ray>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto cache = static_cast<Cache<Box, Ray>*>(cacheBase);
    return CalculatePenetration<Ray, Box>(b, a, &cache->rbCache);
}

/** Box, Plane CalculateIntersection specialization */
template <>
inline bool CalculateIntersection<Box, Plane>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto cache = static_cast<Cache<Box, Plane>*>(cacheBase);
    return CalculateIntersection<Plane, Box>(b, a, &cache->pbCache);
}

/** Box, Plane CalculateContactNormal specialization */
template <>
inline glm::dvec3 CalculateContactNormal<Box, Plane>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto plane = static_cast<Plane const*>(b);
    return plane->normal;
}

/** Box, Plane CalculatePenetration specialization */
template <>
inline double CalculatePenetration<Box, Plane>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto cache = static_cast<Cache<Box, Plane>*>(cacheBase);
    return CalculatePenetration<Plane, Box>(b, a, &cache->pbCache);
}

/** Box, Sphere CalculateIntersection specialization */
template <>
inline bool CalculateIntersection<Box, Sphere>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto cache = static_cast<Cache<Box, Sphere>*>(cacheBase);
    return CalculateIntersection<Sphere, Box>(b, a, &cache->sbCache);
}

/** Box, Sphere CalculateContactNormal specialization */
template <>
inline glm::dvec3 CalculateContactNormal<Box, Sphere>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto cache = static_cast<Cache<Box, Sphere>*>(cacheBase);
    CalculateContactNormal<Sphere, Box>(b, a, &cache->sbCache);

    return cache->sbCache.sphereContactNormal;
}

/** Box, Sphere CalculatePenetration specialization */
template <>
inline double CalculatePenetration<Box, Sphere>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto cache = static_cast<Cache<Box, Sphere>*>(cacheBase);
    return CalculatePenetration<Sphere, Box>(b, a, &cache->sbCache);
}

/** Box, Box CalculateIntersection specialization */
template <>
inline bool CalculateIntersection<Box, Box>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto aBox = static_cast<Box const*>(a);
    auto bBox = static_cast<Box const*>(b);
    auto cache = static_cast<Cache<Box, Box>*>(cacheBase);

    cache->aBoxAxes = { aBox->iAxis, aBox->jAxis, aBox->kAxis, -aBox->iAxis, -aBox->jAxis, -aBox->kAxis };
    cache->bBoxAxes = { bBox->iAxis, bBox->jAxis, bBox->kAxis, -bBox->iAxis, -bBox->jAxis, -bBox->kAxis };

    math::CalculateBoxVertices(cache->aBoxAxes[0], cache->aBoxAxes[1], cache->aBoxAxes[2], cache->aBoxVertices.begin());
    math::CalculateBoxVertices(cache->bBoxAxes[0], cache->bBoxAxes[1], cache->bBoxAxes[2], cache->bBoxVertices.begin());

    for (glm::dvec3& vertex : cache->aBoxVertices)
    {
        vertex += aBox->centerOfMass;
    }
    for (glm::dvec3& vertex : cache->bBoxVertices)
    {
        vertex += bBox->centerOfMass;
    }

    std::transform(cache->aBoxAxes.begin(), cache->aBoxAxes.end(), cache->aBoxFaces.begin(),
        [aBox](glm::dvec3 const& v)
    {
        return v + aBox->centerOfMass;
    });
    std::transform(cache->bBoxAxes.begin(), cache->bBoxAxes.end(), cache->bBoxFaces.begin(),
        [bBox](glm::dvec3 const& v)
    {
        return v + bBox->centerOfMass;
    });

    cache->separatingAxes = {
        glm::normalize(cache->aBoxAxes[0]), glm::normalize(cache->aBoxAxes[1]), glm::normalize(cache->aBoxAxes[2]),
        glm::normalize(cache->bBoxAxes[0]), glm::normalize(cache->bBoxAxes[1]), glm::normalize(cache->bBoxAxes[2])
    };
    math::CalculateCrossProductForeach(cache->aBoxAxes.begin(), cache->aBoxAxes.begin() + 3,
        cache->bBoxAxes.begin(), cache->bBoxAxes.begin() + 3, std::back_inserter(cache->separatingAxes));

    for (glm::dvec3 const& axis : cache->separatingAxes)
    {
        math::CalculateDotProductForeach(
            axis, cache->aBoxVertices.begin(), cache->aBoxVertices.end(), cache->aBoxVerticesDistances.begin()
        );
        math::CalculateDotProductForeach(
            axis, cache->bBoxVertices.begin(), cache->bBoxVertices.end(), cache->bBoxVerticesDistances.begin()
        );
        std::sort(cache->aBoxVerticesDistances.begin(), cache->aBoxVerticesDistances.end());
        std::sort(cache->bBoxVerticesDistances.begin(), cache->bBoxVerticesDistances.end());

        if (cache->aBoxVerticesDistances.back() < cache->bBoxVerticesDistances.back())
        {
            if (cache->aBoxVerticesDistances.back() < cache->bBoxVerticesDistances.front())
            {
                return false;
            }
        }
        else if (cache->bBoxVerticesDistances.back() < cache->aBoxVerticesDistances.front())
        {
            return false;
        }
    }

    return true;
}

/** Box, Box CalculateContactNormal specialization */
template <>
inline glm::dvec3 CalculateContactNormal<Box, Box>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto cache = static_cast<Cache<Box, Box>*>(cacheBase);
    auto aBox = static_cast<Box const*>(a);

    std::array<double, 6> distances;
    for (uint32_t i = 0; i < distances.size(); ++i)
    {
        distances[i] = glm::length(aBox->centerOfMass - cache->bBoxFaces[i]);
    }

    size_t const minIndex = std::distance(
        distances.begin(), std::min_element(distances.begin(), distances.end())
    );
    cache->contactNormal = glm::normalize(cache->bBoxAxes[minIndex]);

    return cache->contactNormal;
}

/** Box, Box CalculatePenetration specialization */
template <>
inline double CalculatePenetration<Box, Box>(SimpleShape const* a, SimpleShape const* b, CacheBase* cacheBase)
{
    auto cache = static_cast<Cache<Box, Box>*>(cacheBase);

    math::CalculateDotProductForeach(cache->contactNormal, cache->aBoxVertices.begin(),
        cache->aBoxVertices.end(), cache->aBoxVerticesDistances.begin());
    math::CalculateDotProductForeach(cache->contactNormal, cache->bBoxVertices.begin(),
        cache->bBoxVertices.end(), cache->bBoxVerticesDistances.begin());

    double const bMaxVertexDistance = *std::max_element(cache->bBoxVerticesDistances.begin(), cache->bBoxVerticesDistances.end());
    double const aMinVertexDistance = *std::min_element(cache->aBoxVerticesDistances.begin(), cache->aBoxVerticesDistances.end());
    cache->penetration = bMaxVertexDistance - aMinVertexDistance;

    return cache->penetration;
}
} // namespace intersection

/**
 * @brief Provides generic interface for the runtime simple shape intersection detections
 */
class SimpleShapeIntersectionDetector
{
public:
    SimpleShapeIntersectionDetector();

    SimpleShapeIntersectionDetector(SimpleShapeIntersectionDetector const&) = delete;

    SimpleShapeIntersectionDetector& operator=(SimpleShapeIntersectionDetector const&) = delete;

    /**
     * @brief Performs intersection test of two shapes and returns true if shapes are intersecting
     * @param[in] a input shape
     * @param[in] b input shape
     * @return @c true if there is intersection, @c false otherwise
     */
    bool CalculateIntersection(SimpleShape const* a, SimpleShape const* b);

    /**
     * @brief Calculates surface contact normal of b shape
     *
     * Must be called strictly after CalculateIntersection function call, otherwise result is undefined
     * @param[in] a input shape
     * @param[in] b input shape
     * @return contact normal
     */
    glm::dvec3 CalculateContactNormal(SimpleShape const* a, SimpleShape const* b);

    /**
     * @brief Calculates penetration depth of two shapes
     * 
     * Must be called strictly after CalculateContactNormal function call, 
     * otherwise result is undefined
     * @param[in] a input shape
     * @param[in] b input shape
     * @return penetration depth
     */
    double CalculatePenetration(SimpleShape const* a, SimpleShape const* b);

private:
    using ShapeTypePair = std::pair<SimpleShape::Type, SimpleShape::Type>;

    /** Hasher for ShapeTypePair objects */
    struct ShapeTypePairHasher
    {
        size_t operator()(ShapeTypePair const& p) const;
    };

    static constexpr uint32_t s_unorderedMapInitialPrimeSize = 11;

    std::unordered_map<ShapeTypePair,
                       std::unique_ptr<intersection::CacheBase>,
                       ShapeTypePairHasher>
    m_intersectionCaches;

    std::unordered_map<ShapeTypePair,
                       bool(*)(SimpleShape const*, SimpleShape const*, intersection::CacheBase*),
                       ShapeTypePairHasher>
    m_calculateIntersectionFunctors;

    std::unordered_map<ShapeTypePair,
                       glm::dvec3(*)(SimpleShape const*, SimpleShape const*, intersection::CacheBase*),
                       ShapeTypePairHasher>
    m_calculateContactNormalFunctors;

    std::unordered_map<ShapeTypePair,
                       double(*)(SimpleShape const*, SimpleShape const*, intersection::CacheBase*),
                       ShapeTypePairHasher>
    m_calculatePenetrationFunctors;
};

} // namespace geometry
} // namespace pegasus

#endif // PEGASUS_GEOMETRY_HPP
