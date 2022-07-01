#pragma once
#include <cstdint>
#include <chrono>
#include <bitset>
#include "math_types.h"
#include "MatrixMath.h"

#define D_VK_W 0x57
#define D_VK_A 0x41
#define D_VK_S 0x53
#define D_VK_D 0x44

namespace end
{
	// Simple app class for development and testing purposes
	class dev_app_t
	{
	private:
		bool particles_initialized = false;
		bool emitters_initialized = false;
		bool grid_initialized = false;
		bool matrices_initialized = false;

		matrixMath::Matrix4x4 character_matrix;
		matrixMath::Matrix4x4 watcher1_matrix;
		matrixMath::Matrix4x4 watcher2_matrix;

		void draw_character(float3 position, matrixMath::Matrix4x4& worldMatrix);
	public:
		float movement_speed = 4;
		float rotation_speed = 100;
		std::bitset<256> keyStates;

		void update();

		dev_app_t();

		double get_delta_time()const;

		// Initialization functions
		void initialize_grid();

		void initialize_particles();

		void initialize_emitters();

		void initialize_world_matrices();

		// Update functions
		void update_sorted_pool_emitters();

		void update_free_pool_emitters();

		void update_user_character_movement();

		// Draw functions
		void draw_characters();
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