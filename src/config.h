#ifndef CONFIG_H

#define CONFIG_H

typedef struct {
  int render_width;
  int render_height;
  char *render_font;
  int render_font_size;
  char *render_text_color;
  char *render_background_color;
  char *render_text_align;
  char *render_char_align;
  char *render_bg_img;
  float render_opacity;

  char *text_command;

  char *wallpaper_path;
  char *wallpaper_set_command;
} Config;

Config *read_config(void);
void config_destroy(Config **config);
void config_print(Config *config);

#endif
