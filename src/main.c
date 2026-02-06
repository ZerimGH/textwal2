#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include "def.h"
#include "tomlc17.c"
#include "textwal.h"
#include "stdlib.h"
#include "config.h"

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

char *run_command(const char *command) {
  if (!command) return NULL;
  char *expanded = expand_path((char *)command);
  if (!expanded) return NULL;

  size_t len = strlen(expanded) + 16;
  char *cmd = malloc(len);
  if (!cmd) {
    free(expanded);
    return NULL;
  }

  snprintf(cmd, len, "/bin/sh -c \"%s\"", expanded);
  free(expanded);
  FILE *fp = popen(cmd, "r");
  free(cmd);

  if (!fp) {
    PERROR("popen() failed\n");
    return NULL;
  }

  size_t alloced = 1024;
  size_t count = 0;
  char *out = malloc(sizeof(char) * alloced);
  if (!out) {
    pclose(fp);
    return NULL;
  }

  int c;
  while ((c = fgetc(fp)) != EOF) {
    if (count + 1 >= alloced) {
      alloced *= 2;
      char *tmp = realloc(out, sizeof(char) * alloced);
      if (!tmp) {
        free(out);
        pclose(fp);
        return NULL;
      }
      out = tmp;
    }
    out[count++] = (char)c;
  }
  out[count] = '\0';

  int status = pclose(fp);
  if (status != 0) {
    free(out);
    return NULL;
  }
  return out;
}


void run_command_noread(const char *command) {
  if (!command) return;
  system(command);
}

TextAlign parse_text_align(char *s) {
  if(!s) return -1;
  if(strcmp(s, "centre") == 0) return TA_Centre;
  if(strcmp(s, "top") == 0) return TA_Top;
  if(strcmp(s, "bottom") == 0) return TA_Bottom;
  if(strcmp(s, "left") == 0) return TA_Left;
  if(strcmp(s, "right") == 0) return TA_Right;
  if(strcmp(s, "top-left") == 0) return TA_TopLeft;
  if(strcmp(s, "top-right") == 0) return TA_TopRight;
  if(strcmp(s, "bottom-left") == 0) return TA_BottomLeft;
  if(strcmp(s, "bottom-right") == 0) return TA_BottomRight;
  return -1;
}

CharAlign parse_char_align(char *s) {
  if(!s) return -1;
  if(strcmp(s, "left") == 0) return CA_Left;
  if(strcmp(s, "right") == 0) return CA_Right;
  return -1;
}

int set(void) {
  Config *c = read_config();
  if(!c) {
    PERROR("Couldn't read config file.\n");
    return 1;
  }
  char *txt = run_command(c->text_command);
  if(!txt) {
    PERROR("Text command failed.\n");
    config_destroy(&c);
    return 1;
  }

  Color bg_c;
  Color text_c;
  sscanf(c->render_background_color, "#%2hhx%2hhx%2hhx", &bg_c[0], &bg_c[1], &bg_c[2]);
  sscanf(c->render_text_color, "#%2hhx%2hhx%2hhx", &text_c[0], &text_c[1], &text_c[2]);

  Options options = {
    c->render_font,
    c->wallpaper_path,
    c->render_font_size,
    c->render_width,
    c->render_height,
    {bg_c[0], bg_c[1], bg_c[2]},
    {text_c[0], text_c[1], text_c[2]},
    parse_text_align(c->render_text_align),
    parse_char_align(c->render_char_align),
    c->render_bg_img,
    c->render_opacity
  };

  if(render(txt, options)) {
    PERROR("Failed to render image.");
    free(txt);
    config_destroy(&c);
    return 1;
  }
  free(txt);

  run_command_noread(c->wallpaper_set_command);  

  config_destroy(&c);
  return 0;
}

int get_path(void) {
  Config *c = read_config();
  if(!c) {
    PERROR("Couldn't read config file.\n");
    return 1;
  }
  printf("%s\n", c->wallpaper_path ? c->wallpaper_path : "(null)");
  return 0;
}

void print_usage(char *exec) {
  printf("Usage: %s [set|get_path]\n", exec);
}

int main(int argc, char **argv) {
  if(argc < 2) {
    print_usage(argv[0]);
    return 0;
  }

  if(strcmp(argv[1], "set") == 0) return set();
  else if(strcmp(argv[1], "get_path") == 0) return get_path();
  else PERROR("Unknown command \"%s\"\n", argv[1]);
  return 1;
}
