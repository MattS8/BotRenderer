#pragma once
#include <cstdint>
#include <chrono>
#include <bitset>
#include "math_types.h"
#include "MatrixMath.h"
#include "view.h"
#include "frustum_culling.h"

#define VK_LEFT				0x25
#define VK_UP				0x26
#define VK_RIGHT			0x27
#define VK_DOWN				0x28

#define D_VK_A				0x41
#define D_VK_D				0x44
#define D_VK_W				0x57
#define D_VK_S				0x53

#define D_VK_RMB			0x1
#define D_VK_LMB			0x0

#define INIT_PARTICLES		0
#define INIT_EMITTERS		1
#define INIT_GRID			2
#define INIT_MATRICES		3
#define INIT_CAMERA			4
#define INIT_CHAR_CAMERA	5

namespace end
{
	// Simple app class for development and testing purposes
	class dev_app_t
	{
	private:
		std::bitset<8> initializers;

		matrixMath::Matrix4x4 character_matrix;
		matrixMath::Matrix4x4 watcher1_matrix;
		matrixMath::Matrix4x4 watcher2_matrix;
		matrixMath::Matrix4x4 camera_view_matrix;

		void draw_character(float3 position, matrixMath::Matrix4x4& worldMatrix);
	public:
		view_t* view;
		view_t character_view;
		frustum_t character_frustum;
		camera_properties character_cam_props;

		float movement_speed = 4;
		float rotation_speed = 100;
		float camera_rotation_speed = 200;
		float camera_movement_speed = 4;
		std::bitset<256> keyStates;
		std::bitset<2> mouseStates;

		void update();

		dev_app_t();

		double get_delta_time()const;

		// Initialization functions
		void initialize_grid();

		void initialize_particles();

		void initialize_emitters();

		void initialize_world_matrices();

		void initialize_camera_view(view_t* view);

		void initialize_character_camera();

		// Update functions
		void update_sorted_pool_emitters();

		void update_free_pool_emitters();

		void update_user_character_movement();

		void update_camera_view();

		void update_mouseX(long deltaX);
		void update_mouseY(long deltaY);

		void update_character_camera();

		// Draw functions
		void draw_characters();

		void draw_character_camera();
	};

	struct grid_colors
	{
		float4 increments[4];
		union
		{
			struct
			{
				float4 horizontal_start;
				float4 horizontal_end;
				float4 vertical_start;
				float4 vertical_end;
			};
			float4 colors[4];
		};

		void update();
	};
}