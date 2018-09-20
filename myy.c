#include <myy.h>
#include <myy/current/opengl.h>

void myy_draw() {
	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
	glClearColor(0.1f, 0.3f, 0.5f, 1.0f);
}

void myy_key(unsigned int keycode) {
	if (keycode == 1) { myy_user_quit(); }
}
