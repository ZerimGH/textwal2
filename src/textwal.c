#include "def.h"
#include <freetype/ftbitmap.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttypes.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_RESIZE2_IMPLEMENTATION
#include <stb_image_resize2.h>
#include "textwal.h"

int render(const char *text, Options options) {
  // Ensure valid options 
  if(!options.font_path) {
    PERROR("NULL font path\n");
    return 1;
  }
  if(!options.out_path) {
    PERROR("NULL output path\n");
    return 1;
  }
  if(options.font_size <= 0) {
    PERROR("Non-positive font size.\n");
    return 1;
  }
  if(options.image_w <= 0 || options.image_h <= 0) {
    PERROR("Non-positive width or height.\n");
    return 1;
  }
  if(options.text_opacity < 0) {
    PERROR("Negative opacity.\n");
    return 1;
  }
  FT_Library ft;
  FT_Face face;

  // Init FreeType
  if(FT_Init_FreeType(&ft)) {
    PERROR("Failed to initialise freetype.\n");
    return 1;
  }

  // Load font
  if(FT_New_Face(ft, options.font_path, 0, &face)) {
    PERROR("Failed to load font %s\n", options.font_path);
    FT_Done_FreeType(ft);
    return 1;
  }

  if(FT_Set_Pixel_Sizes(face, 0, options.font_size)) {
    PERROR("Failed to set font size to %d.\n", options.font_size);
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    return 1;
  }

  // Allocate image
  unsigned char *buf = (unsigned char *)malloc(options.image_w * options.image_h * 3);
  if(!buf) {
    PERROR("Failed to allocate image buffer.\n");
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    return 1;
  }

  // Set background color
  for(int i = 0; i < options.image_w * options.image_h; i++) {
    buf[i * 3 + 0] = options.bg_color[0];
    buf[i * 3 + 1] = options.bg_color[1];
    buf[i * 3 + 2] = options.bg_color[2];
  }

  // Background image
  if(options.bg_img) {
    int img_w, img_h, img_channels;

    unsigned char *img = stbi_load(options.bg_img, &img_w, &img_h, &img_channels, 3);

    if (!img) {
      PERROR("Failed to load background image %s. Defaulting to solid color\n", options.bg_img);
      goto bg_err;
    }

    stbir_resize_uint8_linear(img, img_w, img_h, 0, buf, options.image_w, options.image_h, 0, STBIR_RGB);

    stbi_image_free(img);
  }
bg_err:

  // Calculate total height and width of text rendered
  int total_height = 0;
  int max_width = 0;
  int cur_width = 0;

  total_height = (face->size->metrics.height >> 6);

  for(int i = 0; text[i]; i++) {
    if(text[i] == '\n') {
      total_height += (face->size->metrics.height >> 6);
      if(cur_width > max_width) max_width = cur_width;
      cur_width = 0;
    } else {
      if (FT_Load_Char(face, text[i], FT_LOAD_DEFAULT)) continue;
      cur_width += (face->glyph->advance.x >> 6);
    }
  }

  if(cur_width > max_width) max_width = cur_width;
  int total_width = max_width;


  int y_start = (options.image_h - total_height) / 2 + (face->size->metrics.ascender >> 6);
  int x_start = (options.image_w - total_width) / 2;

  int ra = options.char_align == CA_Right;
  int ra_off = total_width - options.font_size;
  int dir = options.char_align == CA_Right ? -1 : 1;

  // Calculate starting x and y positions
  switch(options.text_align) {
    case TA_Left:
      x_start = 0 + ra_off * ra;
      break;
    case TA_Right:
      x_start = options.image_w - total_width + ra_off * ra;
      break;
    case TA_Top:
      y_start = face->size->metrics.ascender >> 6;
      break;
    case TA_Bottom:
      y_start = options.image_h - total_height + (face->size->metrics.ascender >> 6); 
      break;
    case TA_TopLeft:
      y_start = face->size->metrics.ascender >> 6;
      x_start = 0;
      break;
    case TA_TopRight:
      y_start = face->size->metrics.ascender >> 6;
      x_start = options.image_w - total_width + ra_off * ra;
      break;
    case TA_BottomLeft:
      y_start = options.image_h - total_height + (face->size->metrics.ascender >> 6); 
      x_start = 0 + ra_off * ra;
      break;
    case TA_BottomRight:
      y_start = options.image_h - total_height + (face->size->metrics.ascender >> 6); 
      x_start = options.image_w - total_width + ra_off * ra;
      break;
    default: break;
  }

  char *text_copy = strdup(text);
  if(!text_copy) {
    PERROR("strdup() failed.\n");
    free(buf);
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    return 1;
  }

  // Render text
  int y = y_start;
  int x = x_start;
  char *line = strtok(text_copy, "\n");
  while(line) {
    int line_len = strlen(line);
    for(int i = 0; line[i] != '\0'; i++) {
      char c = line[ra ? (line_len - 1 - i) : (i)]; 
      FT_UInt glyph_index = FT_Get_Char_Index(face, c);

      if(FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER | FT_LOAD_NO_HINTING)) {
        continue;
      }

      FT_Bitmap *bitmap = &face->glyph->bitmap;
      int glyph_baseline_y = y + bitmap->rows - face->glyph->bitmap_top;

      // Write character to image
      for(int row = 0; row < bitmap->rows; row++) {
        for(int col = 0; col < bitmap->width; col++) {
          unsigned char pixel_value = (unsigned char)((float)bitmap->buffer[row * bitmap->width+ col] * options.text_opacity);
          if(pixel_value) {
            int pixel_x =  x + col;
            int pixel_y = (glyph_baseline_y - (bitmap->rows - 1 - row));
            if(pixel_x >= options.image_w || pixel_x < 0) continue;
            if(pixel_y >= options.image_h || pixel_y < 0) continue;

            int idx = (pixel_y * options.image_w + pixel_x) * 3;

            if(idx + 2 >= options.image_w * options.image_h * 3 || idx < 0) continue;
            buf[idx + 0] = (options.txt_color[0] * pixel_value + buf[idx + 0] * (255 - pixel_value)) / 255;
            buf[idx + 1] = (options.txt_color[1] * pixel_value + buf[idx + 1] * (255 - pixel_value)) / 255;
            buf[idx + 2] = (options.txt_color[2] * pixel_value + buf[idx + 2] * (255 - pixel_value)) / 255;
          }
        }
      }

      x += (face->glyph->advance.x >> 6) * dir;
    }

    // Next line
    y += options.font_size;
    x = x_start;
    line = strtok(NULL, "\n");
  }

  // Save image
  stbi_write_png(options.out_path, options.image_w, options.image_h, 3, buf, options.image_w * 3);

  // Cleanup
  free(buf);
  free(text_copy);
  FT_Done_Face(face);
  FT_Done_FreeType(ft);

  return 0;
}
