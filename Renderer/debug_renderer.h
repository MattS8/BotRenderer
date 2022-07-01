#pragma once

#include "math_types.h"

// Interface to the debug renderer
namespace end
{
	namespace debug_renderer
	{
		void add_grid(unsigned int horizontal_lines, unsigned int vertical_lines,
			float4 color_horz_start,
			float4 color_horz_end,
			float4 color_vert_start,
			float4 color_vert_end);

		inline void add_grid(unsigned int horizontal_lines, unsigned int vertical_lines, float4 color) 
		{
			add_grid(horizontal_lines, vertical_lines, color, color, color, color); 
		}

		inline void add_grid(unsigned int horizontal_lines, unsigned int vertical_lines, float4 color_horz, float4 color_vert)
		{
			add_grid(horizontal_lines, vertical_lines, color_horz, color_horz, color_vert, color_vert);
		}

		void add_line(float3 point_a, float3 point_b, float4 color_a, float4 color_b);

		inline void add_line(float3 p, float3 q, float4 color) { add_line(p, q, color, color); }

		void clear_lines();

		const colored_vertex* get_line_verts();

		size_t get_line_vert_count();

		size_t get_line_vert_capacity();
	}
}