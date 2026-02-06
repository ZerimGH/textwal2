#!/usr/bin/env bash

set -xe

# Install program
git submodule update --init --recursive
make all
sudo install -Dm755 ./build/textwal2 /usr/bin/textwal2

# Write default files
mkdir -p ~/.config/textwal2

cat <<EOL > ~/.config/textwal2/config.toml
[render]
width = 1920
height = 1080
font = "/usr/share/fonts/liberation/LiberationMono-Regular.ttf"
font_size = 38 
text_color = "#ffffff"
background_color = "#000000"
text_align = "centre"
char_align = "left"
# bg_img = "/path/to/img.png"
opacity = 1.0 

[text]
command="echo 'Hello, world!'"

[wallpaper]
path="~/.config/textwal2/output.png"
set_command="~/.config/textwal2/set-wallpaper.sh"
EOL

cat <<EOL > ~/.config/textwal2/set-wallpaper.sh
#!/usr/bin/env bash

WALLPAPER_DIR=~/.config/textwal2/output.png
echo 'Please update ~/.config/textwal2/set-wallpaper.sh to properly set your wallpaper.'
EOL

sudo chmod +x ~/.config/textwal2/set-wallpaper.sh 
