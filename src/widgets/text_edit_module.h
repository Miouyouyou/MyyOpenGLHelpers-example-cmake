#ifndef MYY_WIDGETS_TEXT_EDIT_MODULE_H
#define MYY_WIDGETS_TEXT_EDIT_MODULE_H 1

#include <src/widgets/common_types.h>
#include <src/widgets/text_area.h>

/* Binary flags or enumeration... hmm... ? */
enum myy_text_edit_module_flags {
	myy_text_edit_module_flag_inactive,
	myy_text_edit_module_flag_editing,
	myy_text_edit_module_flag_finishing,
};

struct myy_text_edit_module {
	uint32_t flags;
	uint32_t reserved;
	myy_vector_utf8    * __restrict edited_buffer;
	struct myy_text_area * __restrict area;
	off_t edited_buffer_insertion_before;
	off_t edited_buffer_insertion_after;
	myy_vector_utf8 inserted_data_buffer;
};

bool myy_text_edit_module_init(
	struct myy_text_edit_module * __restrict const text_edit_module);

void myy_text_edit_module_attach(
	struct myy_text_edit_module * __restrict const text_edit_module,
	struct myy_text_area * __restrict const text_area,
	off_t insertion_point);

void myy_text_edit_module_detach(
	struct myy_text_edit_module * __restrict const text_edit_module);

void myy_text_edit_module_provoke_redraw(
	struct myy_text_edit_module * __restrict const text_edit_module);

bool myy_text_edit_module_add_text(
	struct myy_text_edit_module * __restrict const text_edit_module,
	uint8_t const * __restrict const text,
	size_t text_size);

#endif
