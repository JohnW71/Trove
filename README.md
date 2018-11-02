# Trove
This is a simple, secure password database written in C, with a console version for Windows & Linux and a mostly completed GUI version for Windows. I may add a Linux GUI in future.

The database is encrypted with AES 256 bit using https://github.com/kokke/tiny-AES-c.

Linux build
gcc trove.c shared.c encryption.c aes.c -std=c99 -o trove_cli -D_POSIX_C_SOURCE -Wall -Wextra -Werror -Wno-pointer-sign -O3

Windows build
CLI
cl trove.c shared.c encryption.c aes.c user32.lib /FeTrove_cli.exe /W3 /WX /FC /O2 /link

GUI
cl gui.c functions.c shared.c gui_encryption.c aes.c user32.lib /FeTrove.exe /W3 /WX /FC /O2 /link
