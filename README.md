# Pong (UCR Winter 2018 EE/CS120B)

# Demo Video (Youtube) 

Link: 

# Downlaod Repository (src code)

Run command in terminal:


Move files from src folder to Atmel Studio 6 or later.

Add io.c and io.h to folder.Then inlcude in project directory.

Build/Compile the Pong.c code and program your ATmega1284 Microcontroller

# Hardware

Harware used for game:

ATmega1284 Microcontroller (1)

8x8 LED matrix (1) 

2-axis Joystick (2)

Shift Registers (2)

Buttons (1)

LCD screen (1)

# How to Play

•	Game starts with ball on (right) paddle 1
•	Ball moves back and forth vertically between both paddles (opponents) 
•	Paddles can be moved left/right by user 
•	If ball move past the paddle of either player opponent scores one point 
•	The game resets after each point at paddle 1 till either player scores 7 points 
•	Winner (informational messages) is shown on LCD Screen and Game on LED Matrix 
•	Player can reset game with button connected to PA6 on the AVR at any time.
