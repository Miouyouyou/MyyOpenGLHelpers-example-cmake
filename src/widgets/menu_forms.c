#include <src/widgets/menu_forms.h>
#include <src/widgets/common_types.h>

#include <stdint.h>

#include <myy/helpers/matrices.h>
#include <myy/helpers/dimensions.h>

#include <shaders.h>

void menu_forms_init(
	struct menu_forms * __restrict const forms)
{
	forms->n_points = 0;
	forms->cpu_buffer = myy_vector_rgb_points_init(256);
	glGenBuffers(1, &forms->gpu_buffer);
	forms->offset = DEFAULT_OFFSET_4D;
}


void menu_forms_reset(
	struct menu_forms * __restrict const forms)
{
	myy_vector_rgb_points_reset(&forms->cpu_buffer);
}

void menu_forms_store_to_gpu(
	struct menu_forms * __restrict const forms)
{
	myy_vector_rgb_points * __restrict const points =
		&forms->cpu_buffer;

	glBindBuffer(GL_ARRAY_BUFFER, forms->gpu_buffer);
	glBufferData(GL_ARRAY_BUFFER,
		myy_vector_rgb_points_allocated_used(points),
		myy_vector_rgb_points_data(points),
		GL_DYNAMIC_DRAW);

	forms->n_points = myy_vector_rgb_points_length(points);
}

void menu_forms_draw(
	struct menu_forms * __restrict const forms)
{
	glUseProgram(myy_programs.menu_forms_id);
	glUniform4f(myy_programs.menu_forms_unif_global_offset,
		forms->offset.x,
		forms->offset.y,
		forms->offset.z,
		forms->offset.w);
	glBindBuffer(GL_ARRAY_BUFFER, forms->gpu_buffer);
	glVertexAttribPointer(
		menu_forms_xyz, 4, GL_SHORT, GL_FALSE,
		myy_vector_rgb_points_type_size(),
		(gpu_buffer_offset_t)
		(offsetof(struct simple_rgb_point, pos)));
	glVertexAttribPointer(
		menu_forms_in_color, 4, GL_UNSIGNED_BYTE, GL_TRUE,
		myy_vector_rgb_points_type_size(),
		(gpu_buffer_offset_t)
		(offsetof(struct simple_rgb_point, color)));

	glDrawArrays(GL_TRIANGLES, 0, forms->n_points);
	glUseProgram(0);
}


void menu_forms_add_arrow_left(
	struct menu_forms * __restrict const forms,
	position_S const pos,
	struct rgba8 const color)
{
	myy_vector_rgb_points * __restrict const forms_buffer =
		&forms->cpu_buffer;
	position_S const a = {
		.x = 0+pos.x,
		.y = pos.y-16
	};
	position_S const b = {
		.x = 32+pos.x,
		.y = pos.y-32
	};
	position_S const c = {
		.x = 32+pos.x,
		.y = 0+pos.y
	};
	int16_t const depth = 0;

	simple_rgb_triangle(forms_buffer, depth, a, b, c, color);
}

void menu_forms_add_arrow_right(
	struct menu_forms * __restrict const forms,
	position_S const pos,
	struct rgba8 const color)
{
	myy_vector_rgb_points * __restrict const forms_buffer =
		&forms->cpu_buffer;
	position_S const a = {
		.x = 0+pos.x,
		.y = 0+pos.y
	};
	position_S const b = {
		.x = 0+pos.x,
		.y = pos.y-32
	};
	position_S const c = {
		.x = 32+pos.x,
		.y = pos.y-16
	};
	int16_t const depth = 0;

	simple_rgb_triangle(forms_buffer, depth, a, b, c, color);
}

void menu_forms_add_bordered_rectangle(
	struct menu_forms * __restrict const forms,
	position_S const down_left, dimensions_S dimensions,
	struct rgba8 const color, struct rgba8 borders_color)
{
	myy_vector_rgb_points * __restrict const forms_buffer =
		&forms->cpu_buffer;

	position_S const up_right = {
		.x = down_left.x + dimensions.width,
		.y = down_left.y - dimensions.height
	};
	int16_t zone_depth = 0;

	simple_rgb_quad(forms_buffer, zone_depth, down_left, up_right, color);

	position_S const border_down_left = {
		.x = down_left.x - 1,
		.y = down_left.y + 1
	};
	position_S const border_up_right = {
		 // +1 to compensate the -1, +1 for the border
		.x = up_right.x + 1,
		.y = up_right.y - 1
	};

	int16_t const border_depth = 1; // Greater mean behind
	simple_rgb_quad(forms_buffer, border_depth,
		border_down_left, border_up_right, borders_color);
}

void menu_parts_handler_init(
	struct menu_parts_handler * __restrict const handler,
	struct gl_text_infos * __restrict const text_atlas_properties,
	uint32_t surface_width, uint32_t surface_height)
{
	text_buffer_init(&handler->static_text, text_atlas_properties);
	text_buffer_init(&handler->input_text, text_atlas_properties);
	menu_forms_init(&handler->forms);
	/* TODO Remove fixed offset. Compute based on current resolution. */
	handler->pos = position_S_4D_struct(surface_width-400,0,0,0);
	/* TODO Add a set position function */
	text_buffer_set_global_position(&handler->input_text, handler->pos);
	text_buffer_set_global_position(&handler->static_text, handler->pos);
	menu_forms_set_global_position(&handler->forms, handler->pos);
}

void menu_part_generate_label(
	union menu_part const * __restrict const label_part,
	struct menu_parts_handler * __restrict const handler)
{
	struct menu_part_label const * __restrict const label =
		&(label_part->label);
	struct myy_text_properties props = {
		.myy_text_flows = ((block_top_to_bottom << 8) | line_left_to_right),
		.z_layer = 16,
		.r = 255, .g = 255, .b = 255, .a = 255,
		.user_metadata = NULL
	};
	position_S pos = position_S_2D_from_4D(label->pos);
	text_buffer_add_string(&handler->static_text,
		label->text,
		&pos,
		&props);
}

void menu_part_generate_not_implemented(
	union menu_part const * __restrict const unknown_part,
	struct menu_parts_handler * __restrict const handler)
{
}

void menu_part_generate_input_numeric(
	union menu_part const * __restrict const input_numeric_part,
	struct menu_parts_handler * __restrict const handler)
{
	struct menu_part_input_numeric const * __restrict const input_numeric =
		&(input_numeric_part->input_numeric);

	position_S pos = position_S_2D_from_4D(input_numeric->pos);
	menu_forms_add_bordered_rectangle(
		&handler->forms,
		pos,
		dimensions_S_struct(50,32),
		rgba8_color(0,0,0,255),
		rgba8_color(120,120,120,255));
}

void menu_parts_reset(
	struct menu_parts_handler * __restrict const handler)
{
	text_buffer_reset(&handler->static_text);
	text_buffer_reset(&handler->input_text);
	menu_forms_reset(&handler->forms);
}

void menu_parts_store_to_gpu(
	struct menu_parts_handler * __restrict const handler)
{
	text_buffer_store_to_gpu(&handler->static_text);
	text_buffer_store_to_gpu(&handler->input_text);
	menu_forms_store_to_gpu(&handler->forms);
}

void menu_parts_draw(
	struct menu_parts_handler * __restrict const handler)
{
	menu_forms_draw(&handler->forms);

	text_buffer_draw(&handler->static_text);
	text_buffer_draw(&handler->input_text);
}

void (* menu_parts_generator[n_menu_part_type])(
	union menu_part const * __restrict,
	struct menu_parts_handler * __restrict) =
{
	[menu_part_type_invalid]        = menu_part_generate_not_implemented,
	[menu_part_type_label]          = menu_part_generate_label,
	[menu_part_type_toggle_button]  = menu_part_generate_not_implemented,
	[menu_part_type_input_register] = menu_part_generate_not_implemented,
	[menu_part_type_input_numeric]  = menu_part_generate_input_numeric,
	[menu_part_type_end]            = menu_part_generate_not_implemented,
};

static void menu_parts_add_static_parts(
	struct menu_parts_handler * __restrict const handler,
	uint32_t surface_width, uint32_t surface_height)
{
	simple_rgb_quad(&handler->forms.cpu_buffer, 15,
		position_S_struct(0,0), position_S_struct(400,surface_height),
		rgba8_color(0,0,0,1));
}

void menu_parts_handler_generate_menu(
	union menu_part const * __restrict parts,
	struct menu_parts_handler * __restrict const handler,
	uint32_t surface_width, uint32_t surface_height)
{
	union menu_part const * __restrict current_part =
		(void *) (parts++);

	struct menu_part_header current_header =
		*((struct menu_part_header *) current_part);

	menu_parts_reset(handler);
	while(menu_part_type_valid(current_header.type)
	      && current_header.type != menu_part_type_end)
	{
		menu_parts_generator[current_header.type](
			current_part,
			handler);
		current_part = (void *) (parts++);
		current_header = *((struct menu_part_header *) current_part);
	}

	menu_parts_add_static_parts(handler, surface_width, surface_height);

	menu_parts_store_to_gpu(handler);
}
