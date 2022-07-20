#include "dev_app.h"
#include "math_types.h"
#include "debug_renderer.h"
#include <iostream>
#include "pools.h"
#include "emitter.h"
#include <vector>


namespace
{
	// Time stuff
	double delta_time = 0.0;
	double particle_lifetime = 1.0;

	// Colors
	end::grid_colors debug_grid_colors = {};
	std::vector<end::float4> particle_colorsRed;
	std::vector<end::float4> particle_colorsGreen;
	std::vector<end::float4> particle_colorsBlue;
	std::vector<end::float4> particle_colorsRG;
	std::vector<end::float4> particle_colorsBG;
	std::vector<end::float4> particle_colorsBR;
	std::vector<end::float4> particle_colorsRGB;
	end::float4 camera_frustum_color = end::float4(0.8f, 0.8f, 0.4f, 1.0f);

	// Pools
	end::sorted_pool_t<end::particle, 300> sorted_pool;
	end::pool_t<end::particle, 1000> free_pool;

	// Emitter
	std::vector<end::emitter> free_pool_emitters;
	std::vector<end::emitter> sorted_pool_emitters;

	// Constants
	const end::float3 Gravity = end::float3(0, -9.8f, 0);
	const end::float3 ParticleSize = end::float3(1.0f, 1.0f, 1.0f);
}

namespace end
{
	double dev_app_t::get_delta_time()const
	{
		return delta_time;
	}

#pragma region Calculation Functions
	matrixMath::Matrix4x4 look_at(float3 position, 
		float3 target, 
		float3 world_up)
	{
		matrixMath::Matrix4x4 look_at_matrix;
		float3 z_axis = target - position;
		z_axis = z_axis.normalize(z_axis);
		float3 x_axis = z_axis.cross(world_up, z_axis);
		x_axis = x_axis.normalize(x_axis);
		float3 y_axis = x_axis.cross(z_axis, x_axis);
		y_axis = y_axis.normalize(y_axis);

		look_at_matrix[0][0] = x_axis[0];
		look_at_matrix[0][1] = x_axis[1];
		look_at_matrix[0][2] = x_axis[2];
		look_at_matrix[0][3] = 0;
		look_at_matrix[1][0] = y_axis[0];
		look_at_matrix[1][1] = y_axis[1];
		look_at_matrix[1][2] = y_axis[2];
		look_at_matrix[1][3] = 0;
		look_at_matrix[2][0] = z_axis[0];
		look_at_matrix[2][1] = z_axis[1];
		look_at_matrix[2][2] = z_axis[2];
		look_at_matrix[2][3] = 0;
		look_at_matrix[3][0] = position[0];
		look_at_matrix[3][1] = position[1];
		look_at_matrix[3][2] = position[2];
		look_at_matrix[3][3] = 1;

		return look_at_matrix;
	}

	float turn_to(float3 position, float3 target, float3 plane)
	{
		float3 turn_position = position - target;
		return turn_position.dot(turn_position.normalize(turn_position), plane);
	}

	double calc_delta_time()
	{
		static std::chrono::time_point<std::chrono::high_resolution_clock> last_time = std::chrono::high_resolution_clock::now();

		std::chrono::time_point<std::chrono::high_resolution_clock> new_time = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed_seconds = new_time - last_time;
		last_time = new_time;

		return std::min(1.0 / 15.0, elapsed_seconds.count());
	}

	float3 get_random_velocity(velocity_values& vel)
	{
		return float3(
			(rand() / (float)RAND_MAX) * (vel.maxX - vel.minX) + vel.minX,
			(rand() / (float)RAND_MAX) * (vel.maxY - vel.minY) + vel.minY,
			(rand() / (float)RAND_MAX) * (vel.maxZ - vel.minZ) + vel.minZ);
	}
#pragma endregion

#pragma region Initialization Functions
	void dev_app_t::initialize_grid()
	{
		debug_grid_colors.increments[0] = { 0.5f, 0.6f, 0.4f, 1.0f };
		debug_grid_colors.increments[1] = { 0.5f, 0.5f, 0.5f, 1.0f };
		debug_grid_colors.increments[2] = { 0.1f, 0.5f, 0.1f, 1.0f };
		debug_grid_colors.increments[3] = { 1.0f, 1.0f, 1.0f, 1.0f };
		debug_grid_colors.horizontal_start = { 1.0f, 0.0f, 0.0f, 1.0f };
		debug_grid_colors.horizontal_end = { 1.0f, 1.0f, 0.0f, 1.0f };
		debug_grid_colors.vertical_start = { 0.0f, 1.0f, 0.0f, 1.0f };
		debug_grid_colors.vertical_end = { 0.0f, 1.0f, 1.0f, 1.0f };

		initializers[Initializers::GRID] = true;
	}

	void dev_app_t::initialize_particles()
	{
		const int variations = 10;
		particle_colorsRed.reserve(variations + 1);
		particle_colorsGreen.reserve(variations + 1);
		particle_colorsBlue.reserve(variations + 1);

		for (int i = 0; i <= variations; i++)
		{
			float varColor = 1.0f - ((float)i / variations);
			particle_colorsRed.push_back(float4(varColor, 0.0f, 0.0f, 1.0f));
			particle_colorsGreen.push_back(float4(0.0f, varColor, 0.0f, 1.0f));
			particle_colorsBlue.push_back(float4(0.0f, 0.0f, varColor, 1.0f));
			particle_colorsBR.push_back(float4(varColor, varColor, 0.0f, 1.0f));
			particle_colorsRG.push_back(float4(varColor, 0.0f, varColor, 1.0f));
			particle_colorsBG.push_back(float4(0.0f, varColor, varColor, 1.0f));
			particle_colorsRGB.push_back(float4(1 - varColor, varColor, 1 - varColor, 1.0f));
		}

		initializers[Initializers::PARTICLES] = true;
	}

	void dev_app_t::initialize_emitters()
	{
		if (!initializers[Initializers::PARTICLES])
			initialize_particles();

		emitter sorted_emitter =
		{
			float3(0, 0, 0), float3(0.5f, 0.5f, 0.5f), particle_colorsRed, std::vector<unsigned int>()
		};
		sorted_emitter.vel_vals = { -2,2, 2,3, -2,2 };
		sorted_pool_emitters.push_back(sorted_emitter);

		emitter free_emitter =
		{
			float3(5, 0, 5), float3(0.5f, 0.5f, 0.5f), particle_colorsGreen, std::vector<unsigned int>()
		};
		free_emitter.vel_vals = { -2,2, 2,3, -2,2 };
		free_pool_emitters.push_back(free_emitter);

		free_emitter.spawn_pos = float3(-5, 0, 5);
		free_emitter.particle_colors = particle_colorsBlue;
		free_pool_emitters.push_back(free_emitter);

		free_emitter.spawn_pos = float3(-6, 4, 3);
		free_emitter.vel_vals = { -4,2, 2,5, -3,5 };
		free_emitter.particle_colors = particle_colorsRGB;
		free_pool_emitters.push_back(free_emitter);

		free_emitter.spawn_pos = float3(-0.3, 0, 7);
		free_emitter.vel_vals = { -2,0, 2,5, -4,4 };
		free_emitter.particle_colors = particle_colorsBG;
		free_pool_emitters.push_back(free_emitter);

		free_emitter.spawn_pos = float3(0.3, 0, 7);
		free_emitter.vel_vals = { 0,2, 2,5, -4,4 };
		free_emitter.particle_colors = particle_colorsBG;
		free_pool_emitters.push_back(free_emitter);

		initializers[Initializers::EMITTERS] = true;
	}

	void dev_app_t::initialize_world_matrices()
	{
		character_matrix = matrixMath::IdentityMatrix4x4;
		character_matrix.Translate(0, 1, 0);
		watcher1_matrix = matrixMath::IdentityMatrix4x4;
		watcher1_matrix.Translate(-6, 4, -5);
		watcher2_matrix = matrixMath::IdentityMatrix4x4;
		watcher2_matrix.Translate(6, 3, 3);

		initializers[Initializers::MATRICES] = true;
	}

	void dev_app_t::initialize_camera_view(view_t* view)
	{
		this->view = view;
		camera_view_matrix.From(view->view_mat);

		initializers[Initializers::CAMERA] = true;
	}

	void dev_app_t::initialize_character_camera()
	{
		if (!initializers[Initializers::MATRICES])
			return;

		character_view.view_mat = character_matrix.ToFloat4x4_a();

		initializers[Initializers::CHAR_CAMERA] = true;
	}

	void dev_app_t::initialize_aabbs()
	{
		if (!initializers[Initializers::MATRICES] || !initializers[Initializers::CHAR_CAMERA])
			return;

		aabbs.push_back({ float3(0, 1, -3), float3(0.25f, 1, 0.35f) });
		aabbs.push_back({ float3(-5, 2, -6), float3(1.1f, 2, 0.7f) });
		aabbs.push_back({ float3(-2, 1.1f, 7), float3(1.2f, 1.1f, 2) });
		aabbs.push_back({ float3(4, 0.5f, -2), float3(1.2f, 0.5f, 1.3f) });

		initializers[Initializers::TEST_AABBS] = true;
	}

	void dev_app_t::initialize_terrain_aabbs()
	{
		initializers[Initializers::CHARACTER_AABB] = true;

		character_matrix[3][1] = 0;
	}
#pragma endregion

#pragma region Update Functions
	void dev_app_t::update_mouseX(long deltaX)
	{
		if (mouseStates[D_VK_RMB] || mouseStates[D_VK_LMB])
		{
			float3 pos;
			pos.x = camera_view_matrix[3][0];
			pos.y = camera_view_matrix[3][1];
			pos.z = camera_view_matrix[3][2];
			float rotation_amount = deltaX * delta_time * camera_rotation_speed;
			camera_view_matrix.InverseOrthoAffine();
			camera_view_matrix.RotateY(-rotation_amount);
			camera_view_matrix.InverseOrthoAffine();
			camera_view_matrix[3][0]= pos.x;
			camera_view_matrix[3][1]= pos.y;
			camera_view_matrix[3][2]= pos.z;
		}
	}

	void dev_app_t::update_mouseY(long deltaY)
	{
		if (mouseStates[D_VK_RMB] || mouseStates[D_VK_LMB])
		{
			float3 pos;
			pos.x = camera_view_matrix[3][0];
			pos.y = camera_view_matrix[3][1];
			pos.z = camera_view_matrix[3][2];
			float rotation_amount = deltaY * delta_time * camera_rotation_speed;
			camera_view_matrix.InverseOrthoAffine();
			camera_view_matrix.RotateX(-rotation_amount);
			camera_view_matrix.InverseOrthoAffine();
			camera_view_matrix[3][0] = pos.x;
			camera_view_matrix[3][1] = pos.y;
			camera_view_matrix[3][2] = pos.z;
		}
	}

	void dev_app_t::update_camera_view()
	{
		float translation_x = (keyStates[D_VK_D] * camera_movement_speed * delta_time)
			- (keyStates[D_VK_A] * camera_movement_speed * delta_time);
		float translation_z = (keyStates[D_VK_W] * camera_movement_speed * delta_time)
			- (keyStates[D_VK_S] * camera_movement_speed * delta_time);

		camera_view_matrix.Translate(translation_x, 0, translation_z);
		view->view_mat = camera_view_matrix.ToFloat4x4_a();
	}

	void dev_app_t::update_user_character_movement()
	{
		static float debug_report_time = 0;
		debug_report_time += delta_time;
		// Player Character Movement
		float rotation = (keyStates[VK_LEFT] * rotation_speed * delta_time)
			- (keyStates[VK_RIGHT] * rotation_speed * delta_time);
		character_matrix.RotateY(rotation);

		float translation_z = (keyStates[VK_UP] * movement_speed * delta_time)
			- (keyStates[VK_DOWN] * movement_speed * delta_time);
		character_matrix.Translate(0, 0, translation_z);

		float3 character_position = float3(
			character_matrix[3][0],
			character_matrix[3][1],
			character_matrix[3][2]
		);

		float3 world_up(0, 1, 0);

		// Watcher 1 (Look At)
		float3 watcher1_position = float3(
			watcher1_matrix[3][0],
			watcher1_matrix[3][1],
			watcher1_matrix[3][2]
		);
		watcher1_matrix = look_at(watcher1_position, character_position, world_up);

		// Watcher 2 (Turn To)
		float3 watcher2_position = float3(
			watcher2_matrix[3][0],
			watcher2_matrix[3][1],
			watcher2_matrix[3][2]
		);

		float3 xComponent = float3(watcher2_matrix[0][0], watcher2_matrix[0][1], watcher2_matrix[0][2]);
		float turn_amount = turn_to(watcher2_position, character_position, xComponent);
		watcher2_matrix.RotateY(turn_amount * delta_time * rotation_speed);

		float3 yComponent = float3(watcher2_matrix[1][0], watcher2_matrix[1][1], watcher2_matrix[1][2]);
		turn_amount = turn_to(watcher2_position, character_position, yComponent);
		
		float3 temp = float3(watcher2_matrix[0][1], watcher2_matrix[0][2], watcher2_matrix[1][3]);
		watcher2_matrix.RotateX(turn_amount * delta_time * rotation_speed);
		watcher2_matrix[1][0] = temp[0];
		watcher2_matrix[1][1] = temp[1];
		watcher2_matrix[1][2] = temp[2];
		watcher2_matrix[3][0] = watcher2_position[0];
		watcher2_matrix[3][1] = watcher2_position[1];
		watcher2_matrix[3][2] = watcher2_position[2];

		if (debug_report_time >= .15f)
		{
			//std::cout << "Right Plane: " << rightPlanef3[0] << ", " << rightPlanef3[1] << ", " << rightPlanef3[2] << "\n";
			//std::cout << "Turn Amount: " << turn_amount << "\n";
			//debug_report_time = 0;
		}

	}

	void dev_app_t::update_sorted_pool_emitters()
	{
		// Update live particles first
		for (int i = 0; i < sorted_pool.size();)
		{
			// Destroy any particles that have reached lifetime
			particle& p = sorted_pool[i];
			p.current_lifetime += delta_time;

			if (p.current_lifetime < particle_lifetime)
			{
				// Update particle trajectory
				p.velocity += Gravity * delta_time;
				p.pos += p.velocity * delta_time;

				// Increment to next pool position
				++i;
			}
			else
				sorted_pool.free(i);
		}

		int ranProc = (rand() % 100);
		int max_per_frame = (10 * delta_time) + ranProc > 99 ? 1 : 0;

		// Spawn new particles for each emitter using sorted_pool
		for (int i = 0; i < sorted_pool_emitters.size() && i < max_per_frame; i++)
		{
			emitter& em = sorted_pool_emitters[i];
			int16_t newIndex = sorted_pool.alloc();
			if (newIndex != -1)
			{
				int picked_color = rand() % (em.particle_colors.size() - 1);
				sorted_pool[newIndex].color = em.particle_colors[picked_color];
				sorted_pool[newIndex].current_lifetime = 0;
				sorted_pool[newIndex].pos = em.spawn_pos;
				sorted_pool[newIndex].velocity = get_random_velocity(em.vel_vals);
				sorted_pool[newIndex].particle_size = em.particle_size;
			}
			else
			{
				std::cerr << "ERROR: emitter -> update_sorted_pool could not allocate "
					<< i << " particle!\n";
			}
		}

	}

	void dev_app_t::update_free_pool_emitters()
	{
		// Spawn new particles for each emitter using free_pool
		for (int i = 0; i < free_pool_emitters.size(); i++)
		{
			emitter& em = free_pool_emitters[i];

			// Update live particles first
			for (int j = 0; j < em.indices.size();)
			{
				particle& p = free_pool[em.indices[j]];
				p.current_lifetime += delta_time;

				if (p.current_lifetime < particle_lifetime)
				{
					// Update particle trajectory
					p.velocity += Gravity * delta_time;
					p.pos += p.velocity * delta_time;
					j++;
				}
				else
				{
					free_pool.free(em.indices[j]);
					em.indices.erase(em.indices.begin() + j);
				}
			}

			// Spawn in new particles
			int ranProc = (rand() % 100);
			int max_per_frame = (10 * delta_time) + ranProc > 99 ? 1 : 0;

			for (int j = 0; j < max_per_frame; j++)
			{
				int16_t newIndex = free_pool.alloc();
				if (newIndex != -1)
				{
					em.indices.push_back(newIndex);
					int picked_color = rand() % (em.particle_colors.size() - 1);
					free_pool[newIndex].color = em.particle_colors[picked_color];
					free_pool[newIndex].current_lifetime = 0;
					free_pool[newIndex].pos = em.spawn_pos;
					free_pool[newIndex].velocity = get_random_velocity(em.vel_vals);
					free_pool[newIndex].particle_size = em.particle_size;
				}
				else
				{
					std::cerr << "ERROR: emitter -> update_free_pool could not allocate "
						<< i << " particle!\n";
				}
			}
		}
	}

	void dev_app_t::update_character_camera()
	{
		character_view.view_mat = character_matrix.ToFloat4x4_a();

		calculate_frustum(character_cam_props, character_frustum, character_view);
	}

	void dev_app_t::update_aabbs()
	{
		for (auto aabb = aabbs.begin(); aabb != aabbs.end(); aabb++)
		{
			draw_aabb(*aabb, aabb_to_frustum(*aabb, character_frustum));
		}
	}

	void dev_app_t::update_character_aabb()
	{
		static const float3 aabb_CharacterExtents = { 0.5f, 1.5f, 0.5f };
		static const float4 character_aabb_color = { 0.2f, 0.2f, 1.0f, 1.0f };

		float3 character_position = float3(
			character_matrix[3][0],
			character_matrix[3][1],
			character_matrix[3][2]
		);

		character_aabb.center = character_position;
		character_aabb.extents = aabb_CharacterExtents;

		draw_aabb(character_aabb, character_aabb_color);
	}

	void grid_colors::update()
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				float newVal = colors[i][j] + (increments[i][j] * delta_time);
				if (newVal > 1.0f)
				{
					newVal = 1.0f;
					increments[i][j] *= -1;
				}
				else if (newVal < 0.0f)
				{
					newVal = 0.0f;
					increments[i][j] *= -1;
				}

				colors[i][j] = newVal;
			}
		}
	}

#pragma region Draw Functions
	void dev_app_t::draw_character(float3 position, matrixMath::Matrix4x4& worldMatrix)
	{
		static const float line_length = 1;
		static const float4 red(1, 0, 0, 1);
		static const float4 green(0, 1, 0, 1);
		static const float4 blue(0, 0, 1, 1);

		matrixMath::Vertex posX_vector(1, 0, 0, 1);
		matrixMath::Vertex posY_vector(0, 1, 0, 1);
		matrixMath::Vertex posZ_vector(0, 0, 1, 1);

		matrixMath::Vertex line_x = posX_vector * worldMatrix;
		matrixMath::Vertex line_y = posY_vector * worldMatrix;
		matrixMath::Vertex line_z = posZ_vector * worldMatrix;

		end::debug_renderer::add_line(
			position,
			float3(line_x[0] * line_length, line_x[1] * line_length, line_x[2] * line_length),
			red
		);
		end::debug_renderer::add_line(
			position,
			float3(line_y[0] * line_length, line_y[1] * line_length, line_y[2] * line_length),
			green
		);
		end::debug_renderer::add_line(
			position,
			float3(line_z[0] * line_length, line_z[1] * line_length, line_z[2] * line_length),
			blue
		);
	}

	void dev_app_t::draw_characters()
	{
		// User character
		float3 character_position = float3(
			character_matrix[3][0],
			character_matrix[3][1],
			character_matrix[3][2]
		);
		draw_character(character_position, character_matrix);

		// Watcher 1
		float3 watcher1_position = float3(
			watcher1_matrix[3][0],
			watcher1_matrix[3][1],
			watcher1_matrix[3][2]
		);
		draw_character(watcher1_position, watcher1_matrix);

		// Watcher 2
		float3 watcher2_position = float3(
			watcher2_matrix[3][0],
			watcher2_matrix[3][1],
			watcher2_matrix[3][2]
		);
		draw_character(watcher2_position, watcher2_matrix);
	}

	void dev_app_t::draw_normal(float3 start, float3 end, float3 norm, float3 offset, float normDir)
	{
		float3 dir = end - start;
		float dirLen = sqrtf(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z) * .5;
		float3 dirNorm = dir.normalize(dir);

		float3 startNorm = end - start;
		startNorm = startNorm.normalize(startNorm);
		float3 e = start + offset + (dirNorm * dirLen);

		end::debug_renderer::add_line(
			e,
			e + norm * normDir * .55f,
			float4(.1, .1, 1, 1), float4(1, 1, 1, 1));
	}

	void dev_app_t::draw_character_camera()
	{

		frustum_points p = calculate_frustum_points(character_cam_props, character_view);

		// Draw Frustum Bounds

		end::debug_renderer::add_line(p.NBL, p.NBR, camera_frustum_color);
		end::debug_renderer::add_line(p.NBL, p.NTL, camera_frustum_color);
		end::debug_renderer::add_line(p.NTL, p.NTR, camera_frustum_color);
		end::debug_renderer::add_line(p.NTR, p.NBR, camera_frustum_color);

		end::debug_renderer::add_line(p.FBL, p.FBR, camera_frustum_color);
		end::debug_renderer::add_line(p.FBL, p.FTL, camera_frustum_color);
		end::debug_renderer::add_line(p.FTL, p.FTR, camera_frustum_color);
		end::debug_renderer::add_line(p.FTR, p.FBR, camera_frustum_color);

		end::debug_renderer::add_line(p.FBL, p.NBL, camera_frustum_color);
		end::debug_renderer::add_line(p.FBR, p.NBR, camera_frustum_color);
		end::debug_renderer::add_line(p.FTR, p.NTR, camera_frustum_color);
		end::debug_renderer::add_line(p.FTL, p.NTL, camera_frustum_color);

		// Draw Frustum Normals
		float3 offset = float3(0, (p.NTL[1] - p.NBL[1]) / -2, 0);
		//Left
		draw_normal(p.NTL, p.FTL, character_frustum[0].normal, offset, 1);
		//Right
		draw_normal(p.NTR, p.FTR, character_frustum[1].normal, offset, 1);
		offset = float3(0, (p.FBL[1] - p.FTR[1]) / 2, 0);
		// Back
		draw_normal(p.FTL, p.FTR, character_frustum[2].normal, offset, 1);
		offset = float3(0, (p.NTL[1] - p.NBL[1]) / -2, 0);
		// Front
		draw_normal(p.NTL, p.NTR, character_frustum[3].normal, offset, 1);
		offset = float3((p.NBL[0] - p.NTR[0]) / 2, 0, 0);
		// Top
		draw_normal(p.NTL, p.FTR, character_frustum[4].normal, offset, 1);
		// Bottom
		draw_normal(p.NBL, p.FBR, character_frustum[5].normal, offset, 1);

	}

	void dev_app_t::draw_aabb(const aabb_t& aabb, float4 color)
	{
		float3 nbl = float3(
			aabb.center.x - aabb.extents.x,
			aabb.center.y - aabb.extents.y,
			aabb.center.z - aabb.extents.z);
		float3 nbr = float3(
			aabb.center.x + aabb.extents.x,
			aabb.center.y - aabb.extents.y,
			aabb.center.z - aabb.extents.z);
		float3 ntl = float3(
			aabb.center.x - aabb.extents.x,
			aabb.center.y + aabb.extents.y,
			aabb.center.z - aabb.extents.z
		);
		float3 ntr = float3(
			aabb.center.x + aabb.extents.x,
			aabb.center.y + aabb.extents.y,
			aabb.center.z - aabb.extents.z
		);
		float3 fbl = float3(
			aabb.center.x - aabb.extents.x,
			aabb.center.y - aabb.extents.y,
			aabb.center.z + aabb.extents.z);
		float3 fbr = float3(
			aabb.center.x + aabb.extents.x,
			aabb.center.y - aabb.extents.y,
			aabb.center.z + aabb.extents.z);
		float3 ftl = float3(
			aabb.center.x - aabb.extents.x,
			aabb.center.y + aabb.extents.y,
			aabb.center.z + aabb.extents.z
		);
		float3 ftr = float3(
			aabb.center.x + aabb.extents.x,
			aabb.center.y + aabb.extents.y,
			aabb.center.z + aabb.extents.z
		);

		end::debug_renderer::add_line(nbl, ntl, color);
		end::debug_renderer::add_line(ntl, ntr, color);
		end::debug_renderer::add_line(ntr, nbr, color);
		end::debug_renderer::add_line(nbr, nbl, color);

		end::debug_renderer::add_line(fbl, ftl, color);
		end::debug_renderer::add_line(ftl, ftr, color);
		end::debug_renderer::add_line(ftr, fbr, color);
		end::debug_renderer::add_line(fbr, fbl, color);

		end::debug_renderer::add_line(ntl, ftl, color);
		end::debug_renderer::add_line(nbl, fbl, color);
		end::debug_renderer::add_line(nbr, fbr, color);
		end::debug_renderer::add_line(ntr, ftr, color);
	}

	void dev_app_t::draw_aabb(const aabb_t& aabb, bool in_frustum)
	{
		draw_aabb(aabb, !in_frustum
			? float4(1, 1, 1, 1)
			: float4(.3, 1, .3, 1));
	}
#pragma endregion

#pragma endregion

	dev_app_t::dev_app_t()
	{
		std::cout << "Log whatever you need here.\n";

		// Initialize Debug Grid Colors
		//initialize_grid();

		// Initialize particle colors
		//initialize_particles();

		// Initialize Emitters
		//initialize_emitters();

		// Initialize World Matrices
		initialize_world_matrices();

		// Initialize showing character camera/frustum
		//initialize_character_camera();
		
		// Initialize AABBs
		//initialize_aabbs();

		// Initialize Terrain AABBs 
		initialize_terrain_aabbs();
	}

	void dev_app_t::update()
	{
		delta_time = calc_delta_time();

		// Update Grid
		if (initializers[Initializers::GRID])
		{
			debug_grid_colors.update();
			end::debug_renderer::add_grid(20, 20,
				debug_grid_colors.horizontal_start, debug_grid_colors.horizontal_end,
				debug_grid_colors.vertical_start, debug_grid_colors.vertical_end);
		}

		// Update Matrix-related functions
		if (initializers[Initializers::MATRICES])
		{
			update_user_character_movement();

			draw_characters();
		}

		// Update Camera View
		if (initializers[Initializers::CAMERA])
		{
			update_camera_view();
		}

		// Update Emitters
		if (initializers[Initializers::EMITTERS])
		{
			update_sorted_pool_emitters();
			update_free_pool_emitters();

			// Draw particles
			for (int i = 0; i < sorted_pool.size(); i++)
				end::debug_renderer::add_line(sorted_pool[i].pos,
					sorted_pool[i].get_endpoint(),
					sorted_pool[i].color);

			for (int i = 0; i < free_pool_emitters.size(); i++)
			{
				emitter& em = free_pool_emitters[i];
				for (auto itter = em.indices.begin(); itter != em.indices.end(); itter++)
				{
					end::debug_renderer::add_line(free_pool[*itter].pos,
						free_pool[*itter].get_endpoint(),
						free_pool[*itter].color);
				}
			}
		}

		// Update Character Camera/Frustum
		if (initializers[Initializers::CHAR_CAMERA])
		{
			update_character_camera();
			draw_character_camera();
		}

		// Update AABBs
		if (initializers[Initializers::TEST_AABBS])
		{
			update_aabbs();
		}

		// Update Character AABBs 
		if (initializers[Initializers::CHARACTER_AABB])
		{
			update_character_aabb();
		}

		// Update Terrain AABBs 
		if (initializers[Initializers::TERRAIN_AABBS])
		{

		}
	}
}
