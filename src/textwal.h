#ifndef TEXTWAL_H

#define TEXTWAL_H

typedef enum {
  TA_Centre,
  TA_Left,
  TA_Right,
  TA_Top,
  TA_Bottom,
  TA_TopRight,
  TA_TopLeft,
  TA_BottomRight,
  TA_BottomLeft
} TextAlign;

typedef enum {
  CA_Left,
  CA_Right
} CharAlign;

typedef unsigned char Color[3];

typedef struct {
  const char *font_path;
  const char *out_path;
  int font_size;
  int image_w, image_h;
  Color bg_color;
  Color txt_color;
  TextAlign text_align;
  CharAlign char_align;
  const char *bg_img;
  float text_opacity; // 0 -> 1
} Options;

int render(const char *text, Options options);

#endif 
