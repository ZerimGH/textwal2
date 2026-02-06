#include "config.h"
#include "tomlc17.h"
#include "def.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static char *expand_path(char *path) {
  if(!path || *path == '\0') return NULL;
  if(*path != '~') return strdup(path);
  const char *home = getenv("HOME");
  if(!home || strlen(home) == 0) {
    PERROR("Couldn't get home directory.");
    return NULL;
  }
  size_t new_len = strlen(path) + strlen(home) - 1;
  char *out = malloc(sizeof(char) * (new_len + 1));
  if(!out) {
    PERROR("malloc() failed.\n");
    return NULL;
  }
  snprintf(out, new_len + 1, "%s%s", home, path + 1);
  return out;
}

#define CONFIG_PATH "~/.config/textwal2/config.toml"

Config *read_config(void) {
  char *config_path = expand_path(CONFIG_PATH);
  if(!config_path) {
    PERROR("Couldn't expand config path.\n");
    return NULL;
  }

  toml_result_t result = toml_parse_file_ex(config_path);
  free(config_path);
  if (!result.ok) {
    PERROR("%s\n", result.errmsg);
    return NULL;
  }

#define ASSIGN_INT(RES, KEY, REQUIRED) do { \
  toml_datum_t RES##_r = toml_seek(result.toptab, KEY); \
  if (RES##_r.type == TOML_INT64) \
    RES = RES##_r.u.int64; \
  else if (REQUIRED) { \
    PERROR("Missing or invalid key: %s\n", KEY); \
    toml_free(result); \
    return NULL; \
  } \
} while (0)

#define ASSIGN_FLOAT(RES, KEY, REQUIRED) do { \
  toml_datum_t RES##_r = toml_seek(result.toptab, KEY); \
  if (RES##_r.type == TOML_INT64) \
    RES = RES##_r.u.fp64; \
  else if (REQUIRED) { \
    PERROR("Missing or invalid key: %s\n", KEY); \
    toml_free(result); \
    return NULL; \
  } \
} while (0)

#define ASSIGN_STRING(RES, KEY, REQUIRED) do { \
  toml_datum_t RES##_r = toml_seek(result.toptab, KEY); \
  if (RES##_r.type == TOML_STRING) { \
    free(RES); \
    RES = strdup(RES##_r.u.str.ptr); \
  } else if (REQUIRED) { \
    PERROR("Missing or invalid key: %s\n", KEY); \
    toml_free(result); \
    return NULL; \
  } \
} while (0)


  // Default values
  int width = 1920;
  int height = 1080;
  char *font = strdup("/usr/share/fonts/liberation/LiberationSerif-Regular.ttf");
  int font_size = 38;
  char *text_color = strdup("#ffffff");
  char *background_color = strdup("#000000");
  char *text_align = strdup("centre");
  char *char_align = strdup("left");
  char *bg_img = NULL;
  float opacity = 1.0;
  char *command = strdup("echo 'Hello, world!'");
  char *set_command = strdup("~/.config/txtwal/set-wallpaper.sh");
  char *path = strdup("~/Pictures/textwal2/output.png");

  ASSIGN_INT(width, "render.width", 0);
  ASSIGN_INT(height, "render.height", 0);
  ASSIGN_INT(font_size, "render.font_size", 0);
  ASSIGN_FLOAT(opacity, "render.opacity", 0);
  ASSIGN_STRING(font, "render.font", 0);
  ASSIGN_STRING(text_color, "render.text_color", 0);
  ASSIGN_STRING(background_color, "render.background_color", 0);
  ASSIGN_STRING(text_align, "render.text_align", 0);
  ASSIGN_STRING(char_align, "render.char_align", 0);
  ASSIGN_STRING(bg_img, "render.bg_img", 0);
  ASSIGN_STRING(command, "text.command", 0);
  ASSIGN_STRING(set_command, "wallpaper.set_command", 0);
  ASSIGN_STRING(path, "wallpaper.path", 0);

  Config *config = malloc(sizeof(Config));
  if(!config) {
    PERROR("malloc() failed.\n");
  }

  config->render_width = width;
  config->render_height = height;
  config->render_font = font;
  config->render_font_size = font_size;
  config->render_text_color = text_color;
  config->render_background_color = background_color;
  config->render_text_align = text_align;
  config->render_char_align = char_align;
  config->render_bg_img = bg_img;
  config->render_opacity = opacity;
  config->wallpaper_set_command = set_command;
  config->wallpaper_path = path;
  toml_free(result);

  // Expand all paths
  char *old = config->render_font;
  config->render_font = expand_path(old);
  free(old);

  old = config->render_bg_img;
  config->render_bg_img = expand_path(old);
  free(old);

  old = config->wallpaper_set_command;
  config->wallpaper_set_command = expand_path(old);
  free(old);

  old = config->wallpaper_path;
  config->wallpaper_path = expand_path(old);
  free(old);

  return config;
}

void config_destroy(Config **config) {
  if(!config) return;
  Config *c = *config;
  if(c->render_font) free(c->render_font);
  if(c->render_text_color) free(c->render_text_color);
  if(c->render_background_color) free(c->render_background_color);
  if(c->render_text_align) free(c->render_text_align);
  if(c->render_char_align) free(c->render_char_align);
  if(c->render_bg_img) free(c->render_bg_img);
  if(c->wallpaper_path) free(c->wallpaper_path);
  if(c->wallpaper_set_command) free(c->wallpaper_set_command);
  free(c);
  *config = NULL;
}

void config_print(Config *config) {
  if(!config) printf("(null)\n");
  printf("render.width = %d\n", config->render_width);
  printf("render.height = %d\n", config->render_height);
  printf("render.font_size = %d\n", config->render_font_size);
  printf("render.opacity = %f\n", config->render_opacity);
  if(config->render_font) printf("render.font = %s\n", config->render_font);
  if(config->render_text_color) printf("render.text_color = %s\n", config->render_text_color);
  if(config->render_background_color) printf("render.background_color = %s\n", config->render_background_color);
  if(config->render_text_align) printf("render.text_align = %s\n", config->render_text_align);
  if(config->render_char_align) printf("render.char_align = %s\n", config->render_char_align);
  if(config->render_bg_img) printf("render.bg_img = %s\n", config->render_bg_img);
  if(config->wallpaper_path) printf("wallpaper.path = %s\n", config->wallpaper_path);
  if(config->wallpaper_set_command) printf("wallpaper.set_command = %s\n", config->wallpaper_set_command);
}
