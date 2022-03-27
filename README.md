# Stabber - Analog Circuit Video Game

This is an ambitious project that will make a video game completely out of 
analog circuit components and using only analog circuit logic.

Output of analog circuit will be a video signal that will be passed through
a composite or component cable and then to the TV.

#### The circuit still needs development. There is not a video signal output or audio signal output to the TV display yet.

## The Game

The game is called Stabber. 

It is a 2 player game controlled with a 2 joystick controller for each player.

The objective of the game is to stab the other player with your weapon.

If a player's avatar is stabbed, then that player is eliminated.

Last player standing is the winner.

One joystick controls the player's position, the other joystick controlls the player's weapon position relative to the player.

## How to Simulate the Circuit

1. Open LTSpice

2. Open file analog-vg-circuit-stabber.asc

3. Click on Simulate->Run.

4. Change resistors of potentiometers modelled by volt dividers.

## Circuits

### analog-vg-circuit-stabber.asc

This is the main circuit that makes up the entire game input, logic, render output, and sound output.


Inputs:

- P1posx = voltage representation of player screen position x-coordinate
- P1posy = voltage representation of player screen position y-coordinate
- P2posx = voltage representation of player screen position x-coordinate
- P2posy = voltage representation of player screen position y-coordinate
- W1posx - voltage representation of horizontal position of weapon relative to player
- W1posy - voltage representation of vertical position of weapon relative to player
- W2posx - voltage representation of horizontal position of weapon relative to player
- W2posy - voltage representation of vertical position of weapon relative to player

Function:

- Runs the entire operation of input handling, logic, rendering to screen, reproducing sound on TV speakers. 
- Operates on imposed rule that 0V represents left and bottom of the display, 5V represents top and right of the display.

Output:

- Analog video signal to television set.

- Analog audio signal to television set.

### test-circuit/analog-vg-circuit-stabber-protov1-onlysum.asc

This is a test circuit used for centering the weapon to a player's position.

It works by producing a voltage representation of a collision object for the player's weapon 
based on this mathematical formula, Weapon_collider = Player_position + 0.1*Weapon_position + -0.5*weapon_position_max.

A summing amplifier is used to achieve this mathematical operation.

Weapon position was scaled to 1/10 so that weapon would not move across the entire screen of the display,
but rather 1/10 of the screen relative to the player.

Weapon position is offset to left by half of the max weapon position in order to take away the center bias
given by joystick when it sits idly at its center. This allows it to be moved to the left or right of the player.

### test-circuit/analog-vg-circuit-stabber-protov1-onlycollisiondetect.asc

This is a test circuit used for detecting collision.

It works by taking the difference between a player's position and the position of the collision object of the other player's weapon
and sending it to the zero-crossing detector circuit to detect when the difference is zero at any point.

If a zero is detected by the zero-crossing detector circuit, then the output of the detector goes to the positive rail
if the difference was negative and hit zero or the negative rail if the difference was positive and it hit zero.

For use with a switch, the negative part of the ac signal of the zero-crossing detector 
needs to be positive to produce a signal that only goes positive
every time a zero is detected in either direction so a full-wave rectification is used.
