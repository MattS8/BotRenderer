#ifndef _EMITTER_H
#define _EMITTER_H
#include "math_types.h"
#include <vector>
#include "pools.h"

namespace end
{
	struct particle
	{
		float3 pos;
		float3 particle_size;
		float3 velocity;
		float4 color;
		double current_lifetime;

		float3 get_endpoint()
		{
			return (velocity.normalize(velocity) * particle_size) + pos;
		}
	};

	struct velocity_values
	{
		float maxX;
		float minX;
		float maxY;
		float minY;
		float maxZ;
		float minZ;
	};

	struct emitter
	{
		float3 spawn_pos;
		float3 particle_size;
		std::vector<float4>& particle_colors;
		// indices into the shared_pool 
		std::vector<unsigned int> indices;
		velocity_values vel_vals;
	};
}
#endif // ifndef _EMITTER_H
