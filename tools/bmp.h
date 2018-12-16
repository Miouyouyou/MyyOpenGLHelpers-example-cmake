#ifndef MYY_BITMAP_V5_H
#define MYY_BITMAP_V5_H

struct color_space_xyz {
	int32_t x, y, z;
};

struct bitmap_common_header {
	uint8_t signature[2];                 /* 0x00 */
	uint32_t complete_size;               /* 0x02 */
	uint16_t reserved1;                   /* 0x06 */
	uint16_t reserved2;                   /* 0x08 */
	uint32_t data_offset;                 /* 0x0A */
} __attribute__ ((__packed__));

struct bitmap_info_header_v5 {
	uint32_t info_header_size;            /* 0x0E */
	int32_t width;                        /* 0x12 */
	int32_t height;                       /* 0x16 */
	uint16_t planes;                      /* 0x1A */
	uint16_t bpp;                         /* 0x1c */
	uint32_t compression;                 /* 0x1E */
	uint32_t data_size;                   /* 0x22 */
	uint32_t x_ppi;                       /* 0x26 */
	uint32_t y_ppi;                       /* 0x2A */
	uint32_t colors_in_palette;           /* 0x2E */
	uint32_t important_colors;            /* 0x32 */
	uint32_t red_mask;                    /* 0x36 */
	uint32_t green_mask;                  /* 0x3A */
	uint32_t blue_mask;                   /* 0x3E */
	uint32_t alpha_mask;                  /* 0x42 */
	uint32_t color_space_type;            /* 0x46 */
	struct color_space_xyz red_coords;    /* 0x4A */
	struct color_space_xyz green_coords;  /* 0x56 */
	struct color_space_xyz blue_coords;   /* 0x62 */
	uint32_t red_gamma;                   /* 0x6E */
	uint32_t green_gamma;                 /* 0x72 */
	uint32_t blue_gamma;                  /* 0x76 */
	uint32_t intent;                      /* 0x7A */
	uint32_t profile_data;                /* 0x7E */
	uint32_t profile_size;                /* 0x82 */
	uint32_t reserved;                    /* 0x86 */
	/* Data start at 0x8A */
};

#define BITMAP_V5_METADATA_SIZE (\
	sizeof(struct bitmap_info_header_v5) + sizeof(struct bitmap_common_header)\
)

#endif
