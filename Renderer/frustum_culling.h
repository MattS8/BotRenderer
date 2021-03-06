#pragma once

#include <array>
#include "math_types.h"
#include "view.h"

// Note: You are free to make adjustments/additions to the declarations provided here.

namespace end
{
	struct sphere_t { float3 center; float radius; }; //Alterative: using sphere_t = float4;

	struct plane_t 
	{ 
		float3 normal;
		float offset;

	};  //Alterative: using plane_t = float4;

	struct frustum_points
	{
		union
		{
			struct
			{
				float3 NBL;
				float3 NBR;
				float3 NTL;
				float3 NTR;
				float3 FBL;
				float3 FBR;
				float3 FTL;
				float3 FTR;
			};
			float3 points[8];
		};
	};
	using frustum_t = std::array<plane_t, 6>;

	struct camera_properties
	{
		float CAMERA_ASPECT_RATIO = 9.0f / 16.0f;
		float cameraWidth = 5.25f;
		float cameraLength = 5;
		float cameraHeight = cameraWidth * CAMERA_ASPECT_RATIO;
		float cameraNearWidth = cameraWidth / 3;
		float cameraNearHeight = cameraHeight / 3;
		float nearViewCutoff = 1.25f;
	};

	frustum_points calculate_frustum_points(camera_properties& cam_props, const view_t& view);

	// Calculates the plane of a triangle from three points.
	plane_t calculate_plane(float3 a, float3 b, float3 c);

	// Calculates a frustum (6 planes) from the input view parameter.
	//
	// Calculate the eight corner points of the frustum. 
	// Use your debug renderer to draw the edges.
	// 
	// Calculate the frustum planes.
	// Use your debug renderer to draw the plane normals as line segments.
	void calculate_frustum(camera_properties& cam_props, frustum_t& frustum, 
		const view_t& view);

	// Calculates which side of a plane the sphere is on.
	//
	// Returns -1 if the sphere is completely behind the plane.
	// Returns 1 if the sphere is completely in front of the plane.
	// Otherwise returns 0 (Sphere overlaps the plane)
	int classify_sphere_to_plane(const sphere_t& sphere, const plane_t& plane);

	// Calculates which side of a plane the aabb is on.
	//
	// Returns -1 if the aabb is completely behind the plane.
	// Returns 1 if the aabb is completely in front of the plane.
	// Otherwise returns 0 (aabb overlaps the plane)
	// MUST BE IMPLEMENTED UsING THE PROJECTED RADIUS TEST
	int classify_aabb_to_plane(const aabb_t& aabb, const plane_t& plane);

	// Determines if the aabb is inside the frustum.
	//
	// Returns false if the aabb is completely behind any plane.
	// Otherwise returns true.
	bool aabb_to_frustum(const aabb_t& aabb, const frustum_t& frustum);
}