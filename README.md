Textwal 2 is a program to render text to a wallpaper

Setup:
1) Clone the repo and install
   > git clone https://github.com/ZerimGH/textwal2.git && cd textwal2 && ./install.sh
   >
2) Fill in the script to set the wallpaper at ~/.config/textwal2/set-wallpaper.sh
   An example script for X11 might look like:
   > #!/usr/bin/env bash
   >
   > \# These exports are only needed if the script will be run indirectly, by something like cronie or your wm
   >
   > export DISPLAY=:0
   >
   > export XAUTHORITY=$(ls /tmp/xauth_* | head -n 1)
   >
   > WALLPAPER_DIR=~/.config/textwal2/wallpaper/output.png
   >
   > feh --bg-fill $WALLPAPER_DIR
   > 
3) Run textwal2
   Any text given to the program will be used to render the wallpaper.
   For example to render the text "Hello, World!":
   > echo 'Hello, World!' | textwal2
   >
You can configure the appearance of the text rendered at ~/.config/textwal2/config.toml
