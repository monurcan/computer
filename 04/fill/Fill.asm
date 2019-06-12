// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Fill.asm

// Runs an infinite loop that listens to the keyboard input.
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel;
// the screen should remain fully black as long as the key is pressed.
// When no key is pressed, the program clears the screen, i.e. writes
// "white" in every pixel;
// the screen should remain fully clear as long as no key is pressed.

(LISTEN)


@8192
D=A;
@SCREEN
D=D+A;
@i
M=D;

@KBD
D=M;
@NOT_PRESSED
D;JEQ

(LOOP1)
@i
D=M;
@SCREEN
D=D-M;
@LISTEN
D;JEQ
@i
D=M;
M=M-1;
A=D;
M=-1;
@LOOP1
0;JMP

(NOT_PRESSED)

(LOOP2)
@i
D=M;
@SCREEN
D=D-M;
@LISTEN
D;JEQ
@i
D=M;
M=M-1;
A=D;
M=0;
@LOOP2
0;JMP

@LISTEN
0;JMP
