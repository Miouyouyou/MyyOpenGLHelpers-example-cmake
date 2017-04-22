#include <myy.h>
#include <myy/current/opengl.h>

struct myy_platform_handlers platform = {
	.stop = NULL,
	.stop_data = NULL
};

struct myy_platform_handlers * myy_get_platform_handlers() {
	return &platform;
}
void myy_init() {}

void myy_display_initialised(unsigned int width, unsigned int height) {
}

void myy_generate_new_state() {}

void myy_init_drawing() {}

void myy_draw() {

	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
	glClearColor(0.1f, 0.3f, 0.5f, 1.0f);

}
void myy_after_draw() {}

void myy_rel_mouse_move(int x, int y) {
}

void myy_mouse_action(enum mouse_action_type type, int value) {
}

void myy_save_state(struct myy_game_state * const state) {}

void myy_resume_state(struct myy_game_state * const state) {}

void myy_cleanup_drawing() {}

void myy_stop() {}

void myy_user_quit() {
	platform.stop(platform.stop_data);
}

void myy_click(int x, int y, unsigned int button) {}
void myy_doubleclick(int x, int y, unsigned int button) {}
void myy_move(int x, int y, int start_x, int start_y) {
}
void myy_hover(int x, int y) {
}

void myy_key(unsigned int keycode) {
	if (keycode == 1) { myy_user_quit(); }
}
void myy_key_release(unsigned int keycode) {}
