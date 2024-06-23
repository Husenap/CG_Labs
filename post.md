# Tetris

## Creator: Hussein Taher

## Game objectives
- Clear lines
- Enjoy tetris

## Features:
### Arcade machine that the player can play tetris on
The arcade machine is split into 3 parts:
- The first part is the body of the arcade machine, this is static and textured.
- The second part is the joystick, it's textured and exported as a separate model, so it can react to user input.
- The third part is the screen, this is a quad that has a shader for the bevel and crt effects on the screen and blocks.
The game board is stored in a 32x32 texture which is updated in runtime using a pixel buffer object.

