#include <myy/myy.h>
#include <myy/current/opengl.h>
#include <myy/helpers/opengl/loaders.h>
#include <myy/helpers/opengl/shaders_pack.h>
#include <myy/helpers/fonts/packed_fonts_parser.h>
#include <myy/helpers/fonts/packed_fonts_display.h>
#include <myy/helpers/matrices.h>

#include <myy/helpers/opengl/buffers.h>

#include <myy/helpers/position.h>
#include <myy/helpers/dimensions.h>

#include <menu_parts.h>

#include <string.h>

#include "shaders.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include <src/widgets/simple_forms.h>
#include <src/widgets/menu_forms.h>
#include <src/widgets/text_buffer.h>
#include <src/widgets/stencil.h>
#include <src/widgets/text_area.h>
#include <src/widgets/common_types.h>

/**
 * List widgets :
 * - myy_text_area : Text input widget
 *   * Editing is done with an editor module.
 *     The whole idea is that the module implementation
 *     differs from platform to platform.
 *     (e.g. Android text module editor is completely
 *      different from the X11 one).
 * - 
 */

#include <myy_data.h>



struct menu_parts_handler menu_handler;
struct gl_text_infos gl_text_meta;
struct myy_text_area area;

void myy_init_drawing(
	myy_states * __restrict state,
	uintreg_t surface_width,
	uintreg_t surface_height)
{
	myy_shaders_pack_load_all_programs_from_file(
		"data/shaders.pack",
		(uint8_t * __restrict) &myy_programs);

	struct gl_text_infos * __restrict const loaded_infos =
		&gl_text_meta;
	struct myy_sampler_properties properties =
		myy_sampler_properties_default();

	glActiveTexture(GL_TEXTURE4);
	myy_packed_fonts_load(
		"data/font_pack_meta.dat", loaded_infos, NULL, &properties);

	float inv_tex_width  = 1.0f/loaded_infos->tex_width;
	float inv_tex_height = 1.0f/loaded_infos->tex_height;

	union myy_4x4_matrix matrix;
	myy_matrix_4x4_ortho_layered_window_coords(&matrix, surface_width, surface_height, 64);
	glUseProgram(myy_programs.text_id);
	glUniform1i(
		myy_programs.text_unif_fonts_texture,
		4);
	glUniformMatrix4fv(
		myy_programs.text_unif_projection,
		1,
		GL_FALSE,
		matrix.raw_data);
	glUniform2f(
		myy_programs.text_unif_texture_projection,
		inv_tex_width,
		inv_tex_height);

	glEnableVertexAttribArray(text_xy);
	glEnableVertexAttribArray(text_in_st);


	/* Load the text quads into the GPU */
	text_buffer_init(&menu_text, &gl_text_meta);

	struct myy_text_properties string_display_props = {
		.myy_text_flows = ((block_top_to_bottom << 8) | line_left_to_right),
		.z_layer = 16,
		.r = 255, .g = 255, .b = 255, .a = 255,
		.user_metadata = NULL		
	};
	uint8_t const * __restrict const string =
		(uint8_t const * __restrict)
		"MOV R0, R1, LSL #3\n"
		"Ni roxe des poneys rose, mais uniquement sur le toit.\n"
		"Potatoes will rule the world !\n"
		"何言ってんだこいつ・・・";

	struct myy_rectangle menu_move_limits = {
		.left = 0,    .right  = 0,
		.top  = -600, .bottom = 64
	};
	text_buffer_set_offset_limits(&menu_text, menu_move_limits);
	position_S text_position = position_S_struct(300,60);
	int64_t offset_between_lines = 32;

	text_buffer_add_strings_list(
		&menu_text, supported_armv8_instructions,
		&text_position, &string_display_props,
		offset_between_lines);

	text_buffer_store_to_gpu(&menu_text);
	simple_stencil_init(&menu_stencil);
	simple_stencil_set_projection(&matrix);
	simple_stencil_start_preparing(&menu_stencil);
	simple_stencil_store_rectangle(&menu_stencil,
		position_S_struct(300, 90),
		position_S_struct(600, 600));
	simple_stencil_done_preparing(&menu_stencil);

	menu_forms_init(&test_menu);
	menu_forms_set_projection(&matrix);
	menu_forms_reset(&test_menu);
	menu_forms_add_arrow_left(&test_menu,
		position_S_struct(500,300),
		rgba8_color(0, 175, 225, 255));
	menu_forms_add_arrow_right(&test_menu,
		position_S_struct(548,300),
		rgba8_color(40, 40, 40, 255));
	menu_forms_add_bordered_rectangle(&test_menu,
		position_S_struct(500,348),
		dimensions_S_struct(80, 32),
		rgba8_color(60, 60, 60, 255),
		rgba8_color(255,255,255,255));
	menu_forms_store_to_gpu(&test_menu);

	menu_parts_handler_init(
		&menu_handler, &gl_text_meta,
		surface_width, surface_height);
	menu_parts_handler_generate_menu(
		(union menu_part *) (&mov_menu),
		&menu_handler,
		surface_width, surface_height);


	myy_text_area_init(&area, &gl_text_meta, position_S_struct(32,32));
	/*myy_text_edit_module_init(&module);
	myy_text_edit_module_attach(&module, &area, 0);
	myy_text_edit_module_add_text(
		&module,
		(uint8_t const * __restrict) "そういうことか",
		sizeof("そういうことか"));*/
	
	glClearColor(GLOBAL_BACKGROUND_COLOR);
}

void myy_draw(
	myy_states * __restrict state, 
	uintreg_t i,
	uint64_t last_frame_delta_ns)
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
	simple_stencil_apply(&menu_stencil);
	text_buffer_draw(&menu_text);
	simple_stencil_put_away(&menu_stencil);
	menu_forms_draw(&test_menu);
	menu_parts_draw(&menu_handler);
	myy_text_area_draw(&area);
}

void myy_key(
	myy_states * __restrict state,
	unsigned int keycode)
{
	if (keycode == 1) { myy_user_quit(state); }
}

void myy_doubleclick(
	myy_states * __restrict state,
	int x,
	int y,
	unsigned int button)
{
	if (button == 4)
		text_buffer_move(&menu_text, position_S_struct(0, 16));
	if (button == 5)
		text_buffer_move(&menu_text, position_S_struct(0, -16));
}

void myy_click(
	myy_states * __restrict state,
	int x, int y, unsigned int button)
{
	if (button == 4)
		text_buffer_move(&menu_text, position_S_struct(0, 16));
	if (button == 5)
		text_buffer_move(&menu_text, position_S_struct(0, -16));

	LOG("Click : %d, %d\n", x, y);
	//if ((x > 950) & (x < 1000) & (y > 68) & (y < 100))
		myy_text_input_start(state);
}

void myy_move(	myy_states * __restrict state,
	int x, int y,
	int start_x, int start_y)
{
	
}

void myy_text(
	myy_states * __restrict state,
	char const * __restrict const text,
	size_t const text_size)
{
	LOG("Text %s\n", text);
	myy_text_area_append_text_utf8_characters(
		&area,
		text_size, (uint8_t const * __restrict) text);
	//myy_text_edit_module_add_text(&module, text, text_size);
}

void myy_editor_finished(
	myy_states * __restrict const states,
	uint8_t const * __restrict const string,
	size_t const string_size)
{
	myy_text_area_set_text_utf8_characters(
		&area,
		string_size, string);
}
