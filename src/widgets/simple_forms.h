#ifndef MYY_WIDGETS_SIMPLE_FORMS
#define MYY_WIDGETS_SIMPLE_FORMS 1

#include <stdint.h>
#include <myy/helpers/position.h>
#include <myy/helpers/vector.h>

struct rgba8 {
	uint8_t r, g, b, a;
};

struct simple_rgb_point {
	position_S pos;
	int16_t z, w;
	struct rgba8 color;
};
myy_vector_template(rgb_points, struct simple_rgb_point)

__attribute__((unused))
static inline struct rgba8 rgba8_color(
	uint8_t const r, uint8_t const g, uint8_t const b, uint8_t const a)
{
	struct rgba8 color = { .r = r, .g = g, .b = b, .a = a };
	return color;
}

__attribute__((unused))
static inline struct simple_rgb_point simple_rgb_point_struct(
	position_S const pos, int16_t depth,
	struct rgba8 const color)
{
	struct simple_rgb_point const point = {
		.pos   = pos,
		.color = color,
		.z     = depth,
		.w     = 1
	};

	return point;
}

__attribute__((unused))
static inline void simple_rgb_triangle(
	myy_vector_rgb_points * __restrict const points,
	int16_t depth,
	position_S const a, position_S const b, position_S const c,
	struct rgba8 const color)
{
	struct simple_rgb_point triangle_vertices[3] = {
		simple_rgb_point_struct(a, depth, color),
		simple_rgb_point_struct(b, depth, color),
		simple_rgb_point_struct(c, depth, color)
	};
	myy_vector_rgb_points_add(points, 3, triangle_vertices);
}

__attribute__((unused))
static inline void simple_rgb_quad(
	myy_vector_rgb_points * __restrict const points,
	int16_t const depth,
	position_S const down_left, position_S const up_right,
	struct rgba8 const color)
{
	position_S const up_left = {
		.x = down_left.x,
		.y = up_right.y
	};
	position_S const down_right = {
		.x = up_right.x,
		.y = down_left.y
	};
	struct simple_rgb_point two_triangles_quad_vertices[6] = {
		simple_rgb_point_struct(up_left, depth, color),
		simple_rgb_point_struct(down_left, depth, color),
		simple_rgb_point_struct(up_right, depth, color),

		simple_rgb_point_struct(down_right, depth, color),
		simple_rgb_point_struct(up_right, depth, color),
		simple_rgb_point_struct(down_left, depth, color)
	};

	myy_vector_rgb_points_add(points, 6, two_triangles_quad_vertices);
}


#endif
