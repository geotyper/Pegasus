#ifndef PEGAS_MECHANICS_HPP
#define PEGAS_MECHANICS_HPP

#include "Pegasus/include/geometry.hpp"
#include "Pegasus/include/particle.hpp"

#include <memory>
#include <vector>

namespace pegas {
class Body {
public:
    virtual ~Body() {}
};

class RigidBody : public Body {
public:
    using Ptr = std::shared_ptr<RigidBody>;

    RigidBody(Particle::Ptr const& p, gmt::Sphere::Ptr const& s)
        : p(p)
        , s(s)
    {
    }

    Particle::Ptr const p;
    gmt::Sphere::Ptr const s;
};

using RigidBodies = std::vector<RigidBody::Ptr>;
}

#endif // PEGAS_MECHANICS_HPP
