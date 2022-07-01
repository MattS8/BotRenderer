#include "debug_renderer.h"
#include <array>

// Anonymous namespace
namespace
{
	// Declarations in an anonymous namespace are global BUT only have internal linkage.
	// In other words, these variables are global but are only visible in this source file.

	// Maximum number of debug lines at one time (i.e: Capacity)
	constexpr size_t MAX_LINE_VERTS = 4096; 

	// CPU-side buffer of debug-line verts
	// Copied to the GPU and reset every frame.
	size_t line_vert_count = 0;
	std::array< end::colored_vertex, MAX_LINE_VERTS> line_verts;
}

namespace end
{
	namespace debug_renderer
	{
		void add_grid(unsigned int horizontal_lines, unsigned int vertical_lines, 
			float4 color_horz_start, 
			float4 color_horz_end,
			float4 color_vert_start,
			float4 color_vert_end)
		{
			float horizontal_mid = (float) horizontal_lines / 2;
			float vertical_mid = (float) vertical_lines / 2;
			float increment = 1.0f;
			float vertical_offset = 2.0f;

			float horizontal_start = -vertical_mid * increment;
			float horizontal_end = vertical_mid * increment;
			float vertical_start = -horizontal_mid * increment + vertical_offset;
			float vertical_end = horizontal_mid * increment;

			for (int i = 0; i < horizontal_lines+1; i++)
			{
				float progPoint = (i - horizontal_mid) * increment;
				float3 pos_start(horizontal_start, 0, progPoint);
				float3 pos_end(horizontal_end, 0, progPoint);
				add_line(pos_start, pos_end, color_horz_start, color_horz_end);
			}

			for (int i = 0; i < vertical_lines+1; i++)
			{
				float progPoint = (i - vertical_mid) * increment;
				float3 pos_start(progPoint, 0, vertical_start);
				float3 pos_end(progPoint, 0, vertical_end);
				add_line(pos_start, pos_end, color_vert_start, color_vert_end);
			}
		}

		void add_line(float3 point_a, float3 point_b, float4 color_a, float4 color_b)
		{
			line_verts[line_vert_count].pos = point_a;
			line_verts[line_vert_count++].color = color_a;

			line_verts[line_vert_count].pos = point_b;
			line_verts[line_vert_count++].color = color_b;
		}

		void clear_lines()
		{
			line_vert_count = 0;
		}

		const colored_vertex* get_line_verts()
		{ 
			return line_verts.data();
		}

		size_t get_line_vert_count() 
		{ 
			return line_vert_count;
		}

		size_t get_line_vert_capacity()
		{
			return MAX_LINE_VERTS;
		}
	}
}