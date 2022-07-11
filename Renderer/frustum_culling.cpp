#include "frustum_culling.h"
#include "MatrixMath.h"
#include <cmath>        // std::abs

using namespace end;

plane_t end::calculate_plane(float3 a, float3 b, float3 c)
{
	float3 norm = a.cross(b - a, c - b);
	norm = norm.normalize(norm);
	float offset = norm.dot(norm, a);

	return {norm, offset};
}

frustum_points end::calculate_frustum_points(camera_properties& cam_props, const view_t& view)
{
	frustum_points points = {};

	matrixMath::Matrix4x4 view_mat;
	view_mat = view_mat.From(view.view_mat);

	float3 character_position = float3(
		view_mat[3][0],
		view_mat[3][1],
		view_mat[3][2]
	);

	float cnWidth = cam_props.cameraNearWidth / 2;
	float cnHeight = cam_props.cameraNearHeight / 2;
	float cWidth = cam_props.cameraWidth / 2;
	float cHeight = cam_props.cameraHeight / 2;

	float3 row = float3(view_mat[2][0], view_mat[2][1], view_mat[2][2]);
	float3 forward_amt = row * cam_props.nearViewCutoff;
	float3 center_near = character_position + forward_amt;
	float3 center_far = character_position + (forward_amt * cam_props.cameraLength);

	float3 widthAmount = float3(view_mat[0][0], view_mat[0][1], view_mat[0][2]);
	widthAmount = widthAmount.normalize(widthAmount) * cnWidth;
	float3 heightAmount = float3(view_mat[1][0], view_mat[1][1], view_mat[1][2]);
	heightAmount = heightAmount.normalize(heightAmount) * cnHeight;
	/*NBL*/
	points.NBL = center_near - widthAmount - heightAmount;
	/*NBR*/
	points.NBR = center_near + widthAmount - heightAmount;
	/*NTL*/
	points.NTL = center_near - widthAmount + heightAmount;
	/*NTR*/
	points.NTR = center_near + widthAmount + heightAmount;

	widthAmount = float3(view_mat[0][0], view_mat[0][1], view_mat[0][2]);
	widthAmount = widthAmount.normalize(widthAmount) * cWidth;
	heightAmount = float3(view_mat[1][0], view_mat[1][1], view_mat[1][2]);
	heightAmount = heightAmount.normalize(heightAmount) * cHeight;

	/*FBL*/
	points.FBL = center_far - widthAmount - heightAmount;
	/*FBR*/
	points.FBR = center_far + widthAmount - heightAmount;
	/*FTL*/
	points.FTL = center_far - widthAmount + heightAmount;
	/*FTR*/
	points.FTR = center_far + widthAmount + heightAmount;

	return points;
}

void end::calculate_frustum(camera_properties& cam_props, frustum_t& frustum, const view_t& view)
{
	// Calculate points
	frustum_points points = calculate_frustum_points(cam_props, view);

	// Left Plane
	frustum[0] = calculate_plane(points.NBL, points.FBL, points.FTL);
	// Right Plane
	frustum[1] = calculate_plane(points.NBR, points.FBR, points.FTR);
	frustum[1].normal *= -1;
	// Far Plane
	frustum[2] = calculate_plane(points.FBL, points.FBR, points.FTR);
	// Near Plane
	frustum[3] = calculate_plane(points.NBL, points.NBR, points.NTR);
	frustum[3].normal *= -1;
	// Top Plane
	frustum[4] = calculate_plane(points.NTL, points.FTL, points.FTR);
	// Bottom Plane
	frustum[5] = calculate_plane(points.NBL, points.FBL, points.FBR);
	frustum[5].normal *= -1;
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
	end::sphere_t sphere;
	float3 normalAbs;
	normalAbs.x = std::abs(plane.normal.x);
	normalAbs.y = std::abs(plane.normal.y);
	normalAbs.z = std::abs(plane.normal.z);
	sphere.radius = normalAbs.dot(aabb.extents, normalAbs);
	sphere.center = aabb.center;

	return classify_sphere_to_plane(sphere, plane);
}

bool end::aabb_to_frustum(const aabb_t& aabb, const frustum_t& frustum)
{
	for (int i = 0; i < 6; i++)
	{
		if (classify_aabb_to_plane(aabb, frustum[i]) < 0)
			return false;
	}

	return true;
}



