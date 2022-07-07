#include "frustum_culling.h"

using namespace end;

plane_t end::calculate_plane(float3 a, float3 b, float3 c)
{
	float3 norm = a.cross(b - a, c - b);
	norm = norm.normalize(norm);
	float offset = norm.dot(norm, a);

	return {norm, offset};
}

void end::calculate_frustum(camera_properties& cam_props, frustum_t& frustum, const view_t& view)
{
	// Calculate points
	float3 camera_pos = float3(
		view.view_mat[3][0],
		view.view_mat[3][1],
		view.view_mat[3][2]);
	frustum_points points = {
		/*NBL*/ 
		float3(camera_pos.x - (cam_props.cameraWidth / 2), 
			camera_pos.y - (cam_props.cameraHeight) / 2, 
			camera_pos.z),
		/*NBR*/ 
		float3(camera_pos.x + (cam_props.cameraWidth / 2),
			camera_pos.y - (cam_props.cameraHeight) / 2,
			camera_pos.z),
		/*NTL*/ 
		float3(camera_pos.x - (cam_props.cameraWidth / 2),
			camera_pos.y + (cam_props.cameraHeight) / 2,
			camera_pos.z),
		/*NTR*/ 
		float3(camera_pos.x + (cam_props.cameraWidth / 2),
			camera_pos.y + (cam_props.cameraHeight) / 2,
			camera_pos.z),
		/*FBL*/
		float3(camera_pos.x - (cam_props.cameraWidth / 2),
			camera_pos.y - (cam_props.cameraHeight) / 2,
			camera_pos.z + cam_props.cameraLength),
		/*FBR*/
		float3(camera_pos.x + (cam_props.cameraWidth / 2),
			camera_pos.y - (cam_props.cameraHeight) / 2,
			camera_pos.z + cam_props.cameraLength),
		/*FTL*/
		float3(camera_pos.x - (cam_props.cameraWidth / 2),
			camera_pos.y + (cam_props.cameraHeight) / 2,
			camera_pos.z + cam_props.cameraLength),
		/*FTR*/
		float3(camera_pos.x + (cam_props.cameraWidth / 2),
			camera_pos.y + (cam_props.cameraHeight) / 2,
			camera_pos.z + cam_props.cameraLength)
	};

	// Left Plane
	frustum[0] = calculate_plane(points.NBL, points.FBL, points.FTL);
	// Right Plane
	frustum[1] = calculate_plane(points.NBR, points.FBR, points.FTR);
	// Far Plane
	frustum[2] = calculate_plane(points.FBL, points.FBR, points.FTR);
	// Near Plane
	frustum[3] = calculate_plane(points.NBL, points.NBR, points.NTR);
	// Top Plane
	frustum[4] = calculate_plane(points.NTL, points.FTL, points.FTR);
	// Bottom Plane
	frustum[5] = calculate_plane(points.NBL, points.FBL, points.FBR);
}

int end::classify_sphere_to_plane(const sphere_t& sphere, const plane_t& plane)
{
	float3 center = sphere.center;
	float3 normal = plane.normal;
	float dist = center.dot(center, normal) - plane.offset;

	return dist > sphere.radius 
		? 1 
		: dist < -sphere.radius
			? -1
			: 0;
}

int end::classify_aabb_to_plane(const aabb_t& aabb, const plane_t& plane)
{
	return 0;
}



