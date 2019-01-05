# nes-solitaire
Solitaire for NES

![Start Screen](img/readme-screenshot-1.png?raw=true "Press Start")

Have a relaxing game of solitaire.

![Main Screen](img/readme-screenshot-2.png?raw=true "Main Screen")

Requires cc65 to compile and link. Requires a NES emulator to run.

To build use the command: 
`cl65 -O -t nes -C nes.cfg main.c screen.c sound.c cards.c data.s util.s -o "nes-solitaire.nes"`

If you get build errors, make sure you have the latest version of cc65 from [GitHub](https://github.com/cc65/cc65).



