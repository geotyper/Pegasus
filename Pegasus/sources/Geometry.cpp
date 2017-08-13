/*
* Copyright (C) 2017 by Godlike
* This code is licensed under the MIT license (MIT)
* (http://opensource.org/licenses/MIT)
*/
#include "Pegasus/include/Geometry.hpp"

using namespace pegasus;
using namespace geometry;

Shape::Shape(glm::dvec3 const& centerOfMass)
    : centerOfMass(centerOfMass)
{
}

SimpleShape::SimpleShape(glm::dvec3 const& centerOfMass, SimpleShape::Type type)
    : Shape(centerOfMass)
    , type(type)
{
}

Ray::Ray()
    : SimpleShape(SimpleShape::Type::RAY)
{
}

Ray::Ray(glm::dvec3 const& centerOfMass, glm::dvec3 const& normal)
    : SimpleShape(centerOfMass, SimpleShape::Type::RAY)
    , direction(normal)
{
}

Plane::Plane()
    : SimpleShape(SimpleShape::Type::PLANE)
{
}

Plane::Plane(glm::dvec3 const& centerOfMass, glm::dvec3 const& normal)
    : SimpleShape(centerOfMass, SimpleShape::Type::PLANE)
    , normal(normal)
{
}

Triangle::Triangle()
    : SimpleShape(SimpleShape::Type::TRIANGLE)
{
}

Triangle::Triangle(
    glm::dvec3 const& centerOfMass, glm::dvec3 const& a, glm::dvec3 const& b, glm::dvec3 const& c
)
    : SimpleShape(centerOfMass, SimpleShape::Type::TRIANGLE)
    , aVertex(a)
    , bVertex(b)
    , cVertex(c)
{
    CalculateNormal();
}

void Triangle::CalculateNormal()
{
    normal = glm::cross(bVertex - aVertex, cVertex - aVertex);
}

Sphere::Sphere()
    : SimpleShape(SimpleShape::Type::SPHERE)
    , radius()
{
}

Sphere::Sphere(glm::dvec3 const& centerOfMass, double r)
    : SimpleShape(centerOfMass, SimpleShape::Type::SPHERE)
    , radius(r)
{
}

Cone::Cone()
    : SimpleShape(SimpleShape::Type::CONE)
    , radius()
{
}

Cone::Cone(glm::dvec3 const& centerOfMass, glm::dvec3 const& a, double r)
    : SimpleShape(centerOfMass, SimpleShape::Type::CONE)
    , appex(a)
    , radius(r)
{
}

Capsule::Capsule()
    : SimpleShape(SimpleShape::Type::CAPSULE)
    , radius()
{
}

Capsule::Capsule(
    glm::dvec3 const& centerOfMass, glm::dvec3 const& halfHeight, double r
)
    : SimpleShape(centerOfMass, SimpleShape::Type::CAPSULE)
    , halfHeight(halfHeight)
    , radius(r)
{
}

Cylinder::Cylinder()
    : SimpleShape(SimpleShape::Type::CYLINDER)
    , radius()
{
}

Cylinder::Cylinder(
    glm::dvec3 const& centerOfMass, glm::dvec3 const& halfHeight, double r
)
    : SimpleShape(centerOfMass, SimpleShape::Type::CYLINDER)
    , halfHeight(halfHeight)
    , radius(r)
{
}

Box::Box()
    : SimpleShape(SimpleShape::Type::BOX)
{
}

Box::Box(
    glm::dvec3 const& centerOfMass, glm::dvec3 const& a, glm::dvec3 const& b, glm::dvec3 const& c
)
    : SimpleShape(centerOfMass, SimpleShape::Type::BOX)
    , iAxis(a)
    , jAxis(b)
    , kAxis(c)
{
}

bool intersection::CalculateRaySphereIntersection(
    glm::dvec3 raySphere, double sphereRadius, glm::dvec3 rayDirection
)
{
    double const tCenter = glm::dot(raySphere, rayDirection);
    double const distanceSquare = glm::dot(raySphere, raySphere) - tCenter * tCenter;

    return sphereRadius * sphereRadius - distanceSquare >= 0.0;
}

intersection::RaySphereIntersectionFactors 
intersection::CalculateRaySphereIntersectionFactors(
    glm::dvec3 raySphere, double sphereRadius, glm::dvec3 rayDirection
)
{
    double const tCenter = glm::dot(raySphere, rayDirection);
    double const distanceSquare = glm::dot(raySphere, raySphere) - tCenter * tCenter;
    double const tDelta = glm::sqrt(sphereRadius * sphereRadius - distanceSquare);

    return {tCenter - tDelta, tCenter + tDelta};
}

intersection::RayBoxIntersectionFactors 
intersection::CalculateRayAabbIntersectionFactors(
    glm::dvec3 boxMinPoint, glm::dvec3 boxMaxPoint, glm::dvec3 rayDirection, glm::dvec3 rayOrigin
)
{
    double const t1 = (boxMinPoint.x - rayOrigin.x) / rayDirection.x;
    double const t2 = (boxMaxPoint.x - rayOrigin.x) / rayDirection.x;
    double const t3 = (boxMinPoint.y - rayOrigin.y) / rayDirection.y;
    double const t4 = (boxMaxPoint.y - rayOrigin.y) / rayDirection.y;
    double const t5 = (boxMinPoint.z - rayOrigin.z) / rayDirection.z;
    double const t6 = (boxMaxPoint.z - rayOrigin.z) / rayDirection.z;

    double const tmin = glm::max(glm::max(glm::min(t1, t2), glm::min(t3, t4)), glm::min(t5, t6));
    double const tmax = glm::min(glm::min(glm::max(t1, t2), glm::max(t3, t4)), glm::max(t5, t6));

    return {tmin, tmax};
}

bool intersection::CalculateRayAabbIntersection(double tMin, double tMax)
{
    // tMax < 0, AABB is behind ray; tMin > tMax, no intesection
    return tMax > 0 && tMin < tMax;
}

SimpleShapeIntersectionDetector::SimpleShapeIntersectionDetector()
    : m_intersectionCaches(s_unorderedMapInitialPrimeSize, ShapeTypePairHasher())
    , m_initializeFunctors(s_unorderedMapInitialPrimeSize, ShapeTypePairHasher())
    , m_calculateIntersectionFunctors(s_unorderedMapInitialPrimeSize, ShapeTypePairHasher())
    , m_calculateContactNormalFunctors(s_unorderedMapInitialPrimeSize, ShapeTypePairHasher())
    , m_calculatePenetrationFunctors(s_unorderedMapInitialPrimeSize, ShapeTypePairHasher())
{
    m_intersectionCaches[std::make_pair(SimpleShape::Type::PLANE, SimpleShape::Type::PLANE)]
        = std::make_unique<intersection::Cache<Plane, Plane>>();
    m_intersectionCaches[std::make_pair(SimpleShape::Type::PLANE, SimpleShape::Type::SPHERE)]
        = std::make_unique<intersection::Cache<Plane, Sphere>>();
    m_intersectionCaches[std::make_pair(SimpleShape::Type::PLANE, SimpleShape::Type::BOX)]
        = std::make_unique<intersection::Cache<Plane, Box>>();
    m_intersectionCaches[std::make_pair(SimpleShape::Type::SPHERE, SimpleShape::Type::PLANE)]
        = std::make_unique<intersection::Cache<Sphere, Plane>>();
    m_intersectionCaches[std::make_pair(SimpleShape::Type::SPHERE, SimpleShape::Type::SPHERE)]
        = std::make_unique<intersection::Cache<Sphere, Sphere>>();
    m_intersectionCaches[std::make_pair(SimpleShape::Type::SPHERE, SimpleShape::Type::BOX)]
        = std::make_unique<intersection::Cache<Sphere, Box>>();
    m_intersectionCaches[std::make_pair(SimpleShape::Type::BOX, SimpleShape::Type::PLANE)]
        = std::make_unique<intersection::Cache<Box, Plane>>();
    m_intersectionCaches[std::make_pair(SimpleShape::Type::BOX, SimpleShape::Type::SPHERE)]
        = std::make_unique<intersection::Cache<Box, Sphere>>();
    m_intersectionCaches[std::make_pair(SimpleShape::Type::BOX, SimpleShape::Type::BOX)]
        = std::make_unique<intersection::Cache<Box, Box>>();

    m_initializeFunctors[std::make_pair(SimpleShape::Type::PLANE, SimpleShape::Type::PLANE)]
        = intersection::Initialize<Plane, Plane>;
    m_initializeFunctors[std::make_pair(SimpleShape::Type::PLANE, SimpleShape::Type::SPHERE)]
        = intersection::Initialize<Plane, Sphere>;
    m_initializeFunctors[std::make_pair(SimpleShape::Type::PLANE, SimpleShape::Type::BOX)]
        = intersection::Initialize<Plane, Box>;
    m_initializeFunctors[std::make_pair(SimpleShape::Type::SPHERE, SimpleShape::Type::PLANE)]
        = intersection::Initialize<Sphere, Plane>;
    m_initializeFunctors[std::make_pair(SimpleShape::Type::SPHERE, SimpleShape::Type::SPHERE)]
        = intersection::Initialize<Sphere, Sphere>;
    m_initializeFunctors[std::make_pair(SimpleShape::Type::SPHERE, SimpleShape::Type::BOX)]
        = intersection::Initialize<Sphere, Box>;
    m_initializeFunctors[std::make_pair(SimpleShape::Type::BOX, SimpleShape::Type::PLANE)]
        = intersection::Initialize<Box, Plane>;
    m_initializeFunctors[std::make_pair(SimpleShape::Type::BOX, SimpleShape::Type::SPHERE)]
        = intersection::Initialize<Box, Sphere>;
    m_initializeFunctors[std::make_pair(SimpleShape::Type::BOX, SimpleShape::Type::BOX)]
        = intersection::Initialize<Box, Box>;

    m_calculateIntersectionFunctors[std::make_pair(SimpleShape::Type::PLANE, SimpleShape::Type::PLANE)]
        = intersection::CalculateIntersection<Plane, Plane>;
    m_calculateIntersectionFunctors[std::make_pair(SimpleShape::Type::PLANE, SimpleShape::Type::SPHERE)]
        = intersection::CalculateIntersection<Plane, Sphere>;
    m_calculateIntersectionFunctors[std::make_pair(SimpleShape::Type::PLANE, SimpleShape::Type::BOX)]
        = intersection::CalculateIntersection<Plane, Box>;
    m_calculateIntersectionFunctors[std::make_pair(SimpleShape::Type::SPHERE, SimpleShape::Type::PLANE)]
        = intersection::CalculateIntersection<Sphere, Plane>;
    m_calculateIntersectionFunctors[std::make_pair(SimpleShape::Type::SPHERE, SimpleShape::Type::SPHERE)]
        = intersection::CalculateIntersection<Sphere, Sphere>;
    m_calculateIntersectionFunctors[std::make_pair(SimpleShape::Type::SPHERE, SimpleShape::Type::BOX)]
        = intersection::CalculateIntersection<Sphere, Box>;
    m_calculateIntersectionFunctors[std::make_pair(SimpleShape::Type::BOX, SimpleShape::Type::PLANE)]
        = intersection::CalculateIntersection<Box, Plane>;
    m_calculateIntersectionFunctors[std::make_pair(SimpleShape::Type::BOX, SimpleShape::Type::SPHERE)]
        = intersection::CalculateIntersection<Box, Sphere>;
    m_calculateIntersectionFunctors[std::make_pair(SimpleShape::Type::BOX, SimpleShape::Type::BOX)]
        = intersection::CalculateIntersection<Box, Box>;

    m_calculateContactNormalFunctors[std::make_pair(SimpleShape::Type::PLANE, SimpleShape::Type::PLANE)]
        = intersection::CalculateContactNormal<Plane, Plane>;
    m_calculateContactNormalFunctors[std::make_pair(SimpleShape::Type::PLANE, SimpleShape::Type::SPHERE)]
        = intersection::CalculateContactNormal<Plane, Sphere>;
    m_calculateContactNormalFunctors[std::make_pair(SimpleShape::Type::PLANE, SimpleShape::Type::BOX)]
        = intersection::CalculateContactNormal<Plane, Box>;
    m_calculateContactNormalFunctors[std::make_pair(SimpleShape::Type::SPHERE, SimpleShape::Type::PLANE)]
        = intersection::CalculateContactNormal<Sphere, Plane>;
    m_calculateContactNormalFunctors[std::make_pair(SimpleShape::Type::SPHERE, SimpleShape::Type::SPHERE)]
        = intersection::CalculateContactNormal<Sphere, Sphere>;
    m_calculateContactNormalFunctors[std::make_pair(SimpleShape::Type::SPHERE, SimpleShape::Type::BOX)]
        = intersection::CalculateContactNormal<Sphere, Box>;
    m_calculateContactNormalFunctors[std::make_pair(SimpleShape::Type::BOX, SimpleShape::Type::PLANE)]
        = intersection::CalculateContactNormal<Box, Plane>;
    m_calculateContactNormalFunctors[std::make_pair(SimpleShape::Type::BOX, SimpleShape::Type::SPHERE)]
        = intersection::CalculateContactNormal<Box, Sphere>;
    m_calculateContactNormalFunctors[std::make_pair(SimpleShape::Type::BOX, SimpleShape::Type::BOX)]
        = intersection::CalculateContactNormal<Box, Box>;

    m_calculatePenetrationFunctors[std::make_pair(SimpleShape::Type::PLANE, SimpleShape::Type::PLANE)]
        = intersection::CalculatePenetration<Plane, Plane>;
    m_calculatePenetrationFunctors[std::make_pair(SimpleShape::Type::PLANE, SimpleShape::Type::SPHERE)]
        = intersection::CalculatePenetration<Plane, Sphere>;
    m_calculatePenetrationFunctors[std::make_pair(SimpleShape::Type::PLANE, SimpleShape::Type::BOX)]
        = intersection::CalculatePenetration<Plane, Box>;
    m_calculatePenetrationFunctors[std::make_pair(SimpleShape::Type::SPHERE, SimpleShape::Type::PLANE)]
        = intersection::CalculatePenetration<Sphere, Plane>;
    m_calculatePenetrationFunctors[std::make_pair(SimpleShape::Type::SPHERE, SimpleShape::Type::SPHERE)]
        = intersection::CalculatePenetration<Sphere, Sphere>;
    m_calculatePenetrationFunctors[std::make_pair(SimpleShape::Type::SPHERE, SimpleShape::Type::BOX)]
        = intersection::CalculatePenetration<Sphere, Box>;
    m_calculatePenetrationFunctors[std::make_pair(SimpleShape::Type::BOX, SimpleShape::Type::PLANE)]
        = intersection::CalculatePenetration<Box, Plane>;
    m_calculatePenetrationFunctors[std::make_pair(SimpleShape::Type::BOX, SimpleShape::Type::SPHERE)]
        = intersection::CalculatePenetration<Box, Sphere>;
    m_calculatePenetrationFunctors[std::make_pair(SimpleShape::Type::BOX, SimpleShape::Type::BOX)]
        = intersection::CalculatePenetration<Box, Box>;
}

void SimpleShapeIntersectionDetector::Initialize(SimpleShape const* a, SimpleShape const* b)
{
    m_initializeFunctors[std::make_pair(a->type, b->type)](
        a, b, m_intersectionCaches[std::make_pair(a->type, b->type)].get());
}

bool SimpleShapeIntersectionDetector::CalculateIntersection(SimpleShape const* a, SimpleShape const* b)
{
    return m_calculateIntersectionFunctors[std::make_pair(a->type, b->type)](
        a, b, m_intersectionCaches[std::make_pair(a->type, b->type)].get());
}

glm::dvec3 SimpleShapeIntersectionDetector::CalculateContactNormal(SimpleShape const* a, SimpleShape const* b)
{
    return m_calculateContactNormalFunctors[std::make_pair(a->type, b->type)](
        a, b, m_intersectionCaches[std::make_pair(a->type, b->type)].get());
}

double SimpleShapeIntersectionDetector::CalculatePenetration(SimpleShape const* a, SimpleShape const* b)
{
    return m_calculatePenetrationFunctors[std::make_pair(a->type, b->type)](
        a, b, m_intersectionCaches[std::make_pair(a->type, b->type)].get());
}

size_t SimpleShapeIntersectionDetector::ShapeTypePairHasher::operator()(ShapeTypePair const& p) const
{
    return std::hash<uint32_t>{}(static_cast<uint32_t>(p.first))
        ^ std::hash<uint32_t>{}(static_cast<uint32_t>(p.second));
}

glm::dvec3 gjk::Support(Sphere const& sphere, glm::dvec3 direction)
{
    using namespace intersection;

    Cache<Ray, Sphere> cache;
    direction = glm::normalize(direction);
    Ray const ray{ sphere.centerOfMass - direction * (sphere.radius + 1), direction };

    RaySphereIntersectionFactors intersectionFactors = CalculateRaySphereIntersectionFactors(
        sphere.centerOfMass - ray.centerOfMass, sphere.radius, direction
    );
    
    return ray.centerOfMass + direction * intersectionFactors.tMax;
}

glm::dvec3 gjk::Support(Box const& box, glm::dvec3 direction)
{
    using namespace intersection;

    Cache<Ray, Box> cache;
    direction = glm::normalize(direction);
    Ray const ray{box.centerOfMass - direction, direction};

    Initialize<Ray, Box>(&ray, &box, &cache);
    RayBoxIntersectionFactors intersectionFactors = CalculateRayAabbIntersectionFactors(
        cache.boxMinPoint, cache.boxMaxPoint, cache.rayDirectionBoxSpace, cache.rayOriginBoxSpace
    );

    return ray.centerOfMass + direction * intersectionFactors.tMax;
}

glm::dvec3 gjk::Support(Box const& box1, Box const& box2, glm::dvec3 direction)
{
    return Support(box1, direction) - Support(box2, -direction);
}

bool gjk::TetrahedronPointIntersection(std::array<glm::dvec3, 4> const& vertices, glm::dvec3 const& vertex)
{
    return math::HyperPlane{ vertices[0], vertices[1], vertices[2], &vertices[3] }.SignedDistance(vertex) <= 0.0
        && math::HyperPlane{ vertices[1], vertices[2], vertices[3], &vertices[0] }.SignedDistance(vertex) <= 0.0
        && math::HyperPlane{ vertices[0], vertices[2], vertices[3], &vertices[1] }.SignedDistance(vertex) <= 0.0
        && math::HyperPlane{ vertices[0], vertices[1], vertices[3], &vertices[2] }.SignedDistance(vertex) <= 0.0;
}

gjk::NearestSimplexData NearestSimplexLineSegment(std::array<glm::dvec3, 4>& simplex)
{
    glm::dvec3 const AB = simplex[0] - simplex[1];
    glm::dvec3 const inverseA = -simplex[1];

    if (glm::dot(AB, inverseA))
    {
        return { 2, glm::cross(glm::cross(AB, inverseA), AB) };
    }

    simplex[0] = simplex[1];
    return { 1, inverseA };
}

gjk::NearestSimplexData NearestSimplexTriangle(std::array<glm::dvec3, 4>& simplex)
{
    glm::dvec3 const AB = simplex[1] - simplex[2];
    glm::dvec3 const AC = simplex[0] - simplex[2];
    glm::dvec3 const ABC = glm::cross(AB, AC);

    if (glm::dot(glm::cross(ABC, AC), -simplex[2]) > 0)
    {
        if (glm::dot(AC, -simplex[2]) > 0)
        {
            simplex = { simplex[0], simplex[2] };
            return { 2, glm::cross(glm::cross(AC, -simplex[1]), AC) };
        }

        if (glm::dot(AB, -simplex[2]) > 0)
        {
            simplex = { simplex[1], simplex[2] };
            return { 2, glm::cross(glm::cross(AB, -simplex[1]), AB) };
        }

        simplex = { simplex[2] };
        return { 1, -simplex[0] };
    }
    
    if (glm::dot(glm::cross(AB, ABC), -simplex[2]) > 0)
    {
        if (glm::dot(AB, -simplex[2]) > 0)
        {
            simplex = { simplex[1], simplex[2] };
            return { 2, glm::cross(glm::cross(AB, -simplex[1]), AB) };
        }

        simplex = { simplex[2] };
        return { 1, -simplex[0] };
    }

    if (glm::dot(ABC, -simplex[2]) > 0)
    {
        return { 3, ABC };
    }

    return { 3, ABC };
}

gjk::NearestSimplexData NearestSimplexTetrahedron(std::array<glm::dvec3, 4>& simplex)
{
    std::array<std::array<uint8_t, 3>, 3> const simplexes{
        std::array<uint8_t, 3>{ 0, 1, 3 }, 
        std::array<uint8_t, 3>{ 1, 2, 3 }, 
        std::array<uint8_t, 3>{ 0, 2, 3 } 
    };

    std::array<double, 3> const planeOriginDistances{
        math::HyperPlane{ simplex[0], simplex[1], simplex[3], &simplex[2] }.Distance(glm::dvec3{ 0, 0, 0 }),
        math::HyperPlane{ simplex[1], simplex[2], simplex[3], &simplex[0] }.Distance(glm::dvec3{ 0, 0, 0 }),
        math::HyperPlane{ simplex[0], simplex[2], simplex[3], &simplex[1] }.Distance(glm::dvec3{ 0, 0, 0 })
    };

    size_t const closestPlaneIndex = std::distance(planeOriginDistances.begin(), 
        std::min_element(planeOriginDistances.begin(), planeOriginDistances.end()));

    simplex = { 
        simplex[simplexes[closestPlaneIndex][0]], 
        simplex[simplexes[closestPlaneIndex][1]], 
        simplex[simplexes[closestPlaneIndex][2]] 
    };

    return NearestSimplexTriangle(simplex);
}

gjk::NearestSimplexData gjk::NearestSimplex(std::array<glm::dvec3, 4>& simplex, uint8_t simplexSize)
{
    if (2 == simplexSize)
    {
        return NearestSimplexLineSegment(simplex);
    }
    if (3 == simplexSize)
    {
        return NearestSimplexTriangle(simplex);
    }

    return NearestSimplexTetrahedron(simplex);
}

epa::ContactManifold epa::CalculateContactManifold(Box const& box1, Box const& box2, Polytope polytope)
{
    using ConvexHull = math::QuickhullConvexHull<std::array<glm::dvec3, 4>>;
    using Vertices = std::list<ConvexHull::Vertices::iterator>;

    ConvexHull convexHull(polytope.vertices);
    convexHull.Calculate();
    
    ConvexHull::Faces faces = convexHull.GetFaces();
    Vertices vertices = convexHull.GetVertices();

    return {};
}
