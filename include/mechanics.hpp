#ifndef PEGAS_MECHANICS_HPP
#define PEGAS_MECHANICS_HPP

#include "Pegas/include/geometry.hpp"
#include "Pegas/include/particle.hpp"

#include <vector>
#include <memory>

namespace pegas {
	class Body {
	public:
		virtual ~Body() {}
	};

	class RigidBody : public Body
	{
	public:
		using Ptr = std::shared_ptr<RigidBody>;

		RigidBody(Particle::Ptr const & p, Sphere::Ptr const & s)
			: p(p), s(s)
		{
		}

		Particle::Ptr const p;
		Sphere::Ptr const s;
	};

	using RigidBodies = std::vector<RigidBody::Ptr>;
}

#endif // PEGAS_MECHANICS_HPP