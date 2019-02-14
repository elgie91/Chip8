#include "emulator.h"
#include <iostream>
#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
#include <ctype.h>      /* toupper */

using namespace std;

unsigned char chip8::chip8_fontset[80] =
{
            0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
            0x20, 0x60, 0x20, 0x20, 0x70, // 1
            0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
            0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
            0x90, 0x90, 0xF0, 0x10, 0x10, // 4
            0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
            0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
            0xF0, 0x10, 0x20, 0x40, 0x40, // 7
            0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
            0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
            0xF0, 0x90, 0xF0, 0x90, 0x90, // A
            0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
            0xF0, 0x80, 0x80, 0x80, 0xF0, // C
            0xE0, 0x90, 0x90, 0x90, 0xE0, // D
            0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
            0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void chip8::initialize()
{
	//initialize
	pc = 0x200;
	sp = 0;
	I = 0;
	opcode = 0;

	/*In modern CHIP-8 implementations, it is common to store font data in those lower 512 bytes (0x000-0x200)*/
	for (int i = 0; i <= 79; i++)
        memory[i] = chip8_fontset[i];

    for (int i = 0; i <= 15; i++)
        stack1[i] = 0;

    delayTimer = 0;
    soundTimer = 0;
    drawFlag = false;
}

void chip8::disp_clear()
{
    for (int i = 0; i < 32*64; i++)
            screen[i] = 0;

    return;
}
void chip8::emulateCycle()
{
	//fetch opcode
	/*opcode is 2 bytes but memory is one byte, so need to combine them together --> done by shifting first part
	of memory by 8 bits which creates 8 0's at the tail. Concatenate (or can do 'or' operation) with the second part of the address to get the second part
	--> big endian store in memory*/
	opcode = (memory[pc]<<8) + (memory[pc+1]);
    //printf("memory[%d]:%hx, memory[%d + 1]:%hx\n", pc, memory[pc], pc, memory[pc+1]);
	//printf("Saw opcode: %hx\n",opcode);

	//decode opcode
	switch (opcode & 0xF000) //look at the first part of the op code (0x means hexadecimal so each letter is 4 bits and have short so 16 bits)
	{
	    case (0x0000): //Instruction begins with 0
            switch(opcode & 0xFFFF)
            {
                case(0x00E0): //Clears the screen.
                    disp_clear();
                    drawFlag = true;
                    pc +=2;
                    break;

                case (0x00EE): //return from sub routine
                    sp--;
                    pc = stack1[sp];
                    pc += 2;
                    break;

                default: //0NNNN Call
                    printf("Called RCA program\n");
                    return;
            }
            break;

	    case (0x1000): //goto 0x1NNNN
	        //printf("Pc Before: %hx\n", pc);
            pc = opcode & (0x0FFF); //get the last 12 bits for the addr
            //printf("Pc After: %hx\n", pc);
            break;

        case (0x2000): //Calls subroutine at NNN

            //calling subroutine so need to save pc
            stack1[sp] = pc;
            sp++; //move the stack pointer up (will move up one byte)

            pc = opcode & (0x0FFF); //get the last 12 bits for the addr & call subroutine
            break;

        case (0x3000): //Skips the next instruction if VX equals NN. (Usually the next instruction is a jump to skip a code block)
            if ( V[( opcode & 0x0F00 ) >> 8 ] == (opcode & 0x00FF) ) //need to bit shift 8 otherwise will have a large number as thre indice for V
                pc += 4; //instruction are two bytes so +2, but skipping next instruction so the one after would be +4
            else
                pc += 2;
            break;

        case (0x4000): //Skips the next instruction if VX doesn't equal NN. (Usually the next instruction is a jump to skip a code block)
            if ( V[( opcode & 0x0F00 ) >> 8 ] != (opcode & 0x00FF) )
                pc += 4; //instruction needs to be on even addresses so the next instruction would be +2, so the one after would be +4
            else
                pc += 2;
            break;

        case (0x5000): //Skips the next instruction if VX equals VY. (Usually the next instruction is a jump to skip a code block)
            if ( V[( opcode & 0x0F00 ) >> 8 ] ==  V[( opcode & 0x00F0  ) >> 4 ] )
                pc += 4; //instruction are short length so next instruction would be +2, so the one after would be +4
            else
                pc += 2;
            //printf("Reg 1: %x Reg 2: %x",V[( opcode & 0x0F00 ) >> 8 ], V[( opcode & 0x00F0  )>> 4 ] );
            break;

        case (0x6000): //Sets VX to NN.
            V[( opcode & 0x0F00 ) >> 8 ] = ( opcode & 0x00FF) ;
            pc += 2;
            break;

        case (0x7000): //Adds NN to VX. (Carry flag is not changed)
            V[( opcode & 0x0F00 ) >> 8 ] += ( opcode & 0x00FF);
            pc += 2;
            break;

        case (0x8000):
            switch (opcode & 0xF00F)
            {
                case (0x8000): //Sets VX to the value of VY.
                    V[( opcode & 0x0F00 ) >> 8 ] = V[( opcode & 0x00F0 ) >> 4 ];
                    pc += 2;
                    break;

                case (0x8001): //Sets VX to VX or VY. (Bitwise OR operation)
                    V[( opcode & 0x0F00 )  >> 8 ] = V[( opcode & 0x0F00 ) >> 8 ] | V[( opcode & 0x00F0  ) >> 4 ];
                    pc += 2;
                    break;

                case (0x8002): //Sets VX to VX and VY. (Bitwise AND operation)
                    V[( opcode & 0x0F00 ) >> 8 ] = V[( opcode & 0x0F00 ) >> 8 ] & V[( opcode & 0x00F0 ) >> 4 ];
                    pc += 2;
                    break;

                case (0x8003): //Sets VX to VX xor VY.
                    V[( opcode & 0x0F00 ) >> 8 ] = V[( opcode & 0x0F00 ) >> 8 ] ^ V[( opcode & 0x00F0 ) >> 4 ];
                    pc += 2;
                    break;

                case (0x8004): //Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
                    //check for overflow if (a > INT_MAX - x) & registers hold 8 bits so max number is 0xFF
                    if ( V[( opcode & 0x0F00 ) >> 8 ] > (0xFF - ( V[( opcode & 0x00F0 ) >> 4 ] ) ) )
                            V[0xF] = 1;
                    else
                            V[0xF] = 0;

                    V[( opcode & 0x0F00 ) >> 8 ] += V[( opcode & 0x00F0 ) >> 4 ]; //any overflow causes number to roll back around (0 - (2^8 - 1) )
                    pc += 2;
                    break;

                case (0x8005): //VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
                    //check for underflow if (a < INT_MIN + x) & registers hold 8 bits so min number is 0x00
                    if ( V[( opcode & 0x0F00 ) >> 8 ] < ( 0x00 + V[( opcode & 0x00F0 ) >> 4 ] ) )
                            V[0xF] = 0;
                    else
                            V[0xF] = 1;

                    V[( opcode & 0x0F00 ) >> 8 ] -= V[( opcode & 0x00F0 ) >> 4 ]; //any underrflow causes number to roll back around (0 - (2^8 - 1) )
                    pc += 2;
                    break;

                case (0x8006): //Stores the least significant bit of VX in VF and then shifts VX to the right by 1

                    V[0xF] = V[( opcode & 0x0F00 ) >> 8 ] & 0x01; //registers are 8 bits so and with only two hex letters!
                    V[( opcode & 0x0F00 ) >> 8 ] = V[( opcode & 0x0F00 ) >> 8 ] >> 1; //shift Vx by 1 to the right
                    pc += 2;
                    break;

                case (0x8007): //Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
                    //check for underflow if (a < INT_MIN + x) & registers hold 8 bits so min number is 0x00
                    if ( V[( opcode & 0x00F0 ) >> 4 ] < ( 0x00 + V[( opcode & 0x0F00 ) >> 8 ]  ) )
                            V[0xF] = 0;
                    else
                            V[0xF] = 1;

                    V[( opcode & 0x0F00 ) >> 8 ] = V[( opcode & 0x00F0 ) >> 4 ] - V[( opcode & 0x0F00 ) >> 8 ]; //any underrflow causes number to roll back around (0 - (2^8 - 1) )
                    pc += 2;
                    break;

                case (0x800E): //Stores the most significant bit of VX in VF and then shifts VX to the left by 1

                    V[0xF] = V[( opcode & 0x0F00 ) >> 8 ] >> 7; //registers are 8 bits so and with only two hex letters!
                    V[( opcode & 0x0F00 ) >> 8 ]  = V[( opcode & 0x0F00 ) >> 8 ] << 1; //shift Vx by 1 to the left
                    pc += 2;
                    break;

                default:
                    printf("Unknown 0x8000 instruction\n");
                    return;
            }
            break;

        case (0x9000): //Skips the next instruction if VX doesn't equal VY. (Usually the next instruction is a jump to skip a code block)
            if ( V[( opcode & 0x0F00 ) >> 8 ] !=  V[( opcode & 0x00F0 )>> 4 ] )
                pc += 4; //instruction are short length so next instruction would be +2, so the one after would be +4
            else
                pc += 2;
            break;

        case (0xA000): //Sets I to the address NNN
            I = ( opcode & 0x0FFF);
            pc += 2;
            break;


        case (0xB000): //Jumps to the address NNN plus V0.
            pc = V[0] + ( opcode & 0x0FFF);
            break;

        case (0xC000): //Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.
            V[( opcode & 0x0F00 ) >> 8 ] = (rand() % 256) & ( opcode & 0x00FF);
            pc += 2;
            break;

        /*Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.
        Each row of 8 pixels is read as bit-coded starting from memory location I;
        I value doesn't change after the execution of this instruction.
        As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn,
        and to 0 if that doesn't happen*/
        case (0xD000):
        {

            unsigned char xCoord = V [ (opcode & 0x0F00) >> 8]; //get x coord
            unsigned char yCoord = V [ (opcode & 0x00F0) >> 4]; //get y coord
            unsigned char N = opcode & 0x000F;
            unsigned char spritePixels;

            int iterations = N; //convert N to # of iterations --> legal because char is unsigned 1 byte while int = 4 bytes (will not overflow)
            V[0xF] = 0; //reset VF

            for (int y = 0; y < iterations; y++) //loop that draws 8 pixels for every row defined by N
            {
                spritePixels = memory[I + y]; //each spritePixel row is defined by memory[I + some offset (i)] 8 bits

                for (int x = 0; x <= 7; x++) //draw the 8 pixels one by one
                {
                    if ( (spritePixels & (0x80 >> x))  != 0  )// if the pixel to draw is 1 then need to check if screen is already 1
                    {
                        if (screen[ (xCoord + x) + ( (yCoord + y) * 64) ] == 1) //if screen pixel is already set (=1) then 'xor'ing will make it 0
                            V[0xF] = 1;

                        screen[ (xCoord + x) + ( (yCoord + y) * 64) ] ^=1;
                    }
                }
            }
            //printf("memory[%d]:%hx, memory[%d + 1]:%hx\n", pc, memory[pc], pc, memory[pc+1]);
            drawFlag = true;
            pc += 2;
        }
            break;

        case 0xE000:
            switch (opcode & 0x00FF)
            {
            case (0x009E): //Skips the next instruction if the key stored in VX is pressed. (Usually the next instruction is a jump to skip a code block)
                if (V [ (opcode & 0x0F00) >> 8 ] == keypad)
                    pc += 4;
                else
                    pc += 2;
                break;

            case 0x00A1: //Skips the next instruction if the key stored in VX isn't pressed. (Usually the next instruction is a jump to skip a code block)
                if (V [ (opcode & 0x0F00) >> 8 ] != keypad)
                    pc += 4;
                else
                    pc += 2;
                break;

            default:
                printf("Unknown 0xE000 Instruction\n");
                return;
            }
            break;

        case (0xF000):
            switch (opcode & 0x00FF)
            {
                case (0x0007): //Sets VX to the value of the delay timer.
                     V [ (opcode & 0x0F00) >> 8] = delayTimer;
                     pc += 2;
                     break;

                case (0x000A): //A key press is awaited, and then stored in VX. (Blocking Operation. All instruction halted until next key event)
                     /*cin>>keypad; //blocking operation

                     while (keypad > 15 || keypad < 0) //map buttons to first fifteen numbers & if not one of them, keep reading again
                        cin>>keypad;

                     V [ (opcode & 0x0F00) >> 8] = keypad;*/
                     V [ (opcode & 0x0F00) >> 8] = 0xF; //temporary
                     break;

                case (0x0015): //Sets the delay timer to VX.
                     delayTimer = V [ (opcode & 0x0F00) >> 8];
                     pc += 2;
                     break;

                case (0x0018): //Sets the sound timer to VX.
                     soundTimer = V [ (opcode & 0x0F00) >> 8];
                     pc += 2;
                     break;

                case (0x001E): //Adds VX to I.
                     I += V [ (opcode & 0x0F00) >> 8];
                     pc += 2;
                     break;

                case (0x0029): //Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
                     I = V [ (opcode & 0x0F00) >> 8] * 5; //is a single array so need to multiply by 5 to get to the next character beginning
                     pc += 2;
                     break;

                case (0x0033): //https://whatis.techtarget.com/definition/binary-coded-decimal
                {
                    /*Stores the binary-coded decimal representation of VX, with the most significant of three digits at the address in I
                      the middle digit at I plus 1, and the least significant digit at I plus 2.
                     (In other words, take the decimal representation of VX, place the hundreds digit in memory at location in I, the tens digit at
                      location I+1, and the ones digit at location I+2.)*/

                     unsigned int number = V [(opcode & 0x0F00) >> 8];
                     unsigned char ones, tens, hundreds;
                     ones=tens=hundreds=0;

                     for (int i = 0; i <=2; i++)
                     {
                         if (i == 0)
                         {
                             hundreds = (number / 100) & 0xFF; //is a char so only take last byte
                             number = number % 100; //grab the remainder
                         }

                         if (i == 1)
                         {
                             tens = (number / 10) & 0xFF;
                             number = number % 10;
                         }

                         if (i == 2)
                             ones = number & 0xFF; //modulus from above step will already give you the amount of ones

                     }

                     memory[I] = hundreds;
                     memory[I+1] = tens;
                     memory[I+2] = ones;

                     pc += 2;
                }
                     break;

                     //Vx is three digits b/c 8 bits means 0 - 255 possible numbers so we can have 3 digits

                case (0x0055):
                {


                    /*Stores V0 to VX (including VX) in memory starting at address I. The offset from I is increased by 1 for each value written,
                     but I itself is left unmodified.*/

                     int x = (opcode & 0x0F00) >> 8;
                     for (int i = 0; i <= x; i++ )
                     {
                         memory[I + i] = V[i];
                     }

                     pc += 2;
                }
                     break;

                case (0x0065):
                {

                    /* Fills V0 to VX (including VX) with values from memory starting at address I. The offset from I is increased by 1 for each
                        value written, but I itself is left unmodified.*/

                    int x = (opcode & 0x0F00) >> 8;
                    for (int i = 0; i <= x; i++ )
                    {
                        V[i] = memory[I + i];
                    }

                     pc += 2;
                }
                     break;

                default:
                    printf("Unknown 0xF000 Instruction\n");
                    return;

            }
            break;
	    default:
	        printf("Unknown Instruction\n");
	        return;
	}

    if (delayTimer > 0)
        delayTimer-= 1;
    if (soundTimer > 0)
        soundTimer-= 1;

}

bool chip8::loadApplication(const char * filename)
{
	initialize();
	printf("Loading: %s\n", filename);

	// Open file
	FILE * pFile = fopen(filename, "rb");
	if (pFile == NULL)
	{
		fputs ("File error", stderr);
		return false;
	}

	// Check file size
	fseek(pFile , 0 , SEEK_END);
	long lSize = ftell(pFile);
	rewind(pFile);
	printf("Filesize: %d\n", (int)lSize);

	// Allocate memory to contain the whole file
	char * buffer = (char*)malloc(sizeof(char) * lSize);
	if (buffer == NULL)
	{
		fputs ("Memory error", stderr);
		return false;
	}

	// Copy the file into the buffer
	size_t result = fread (buffer, 1, lSize, pFile);
	if (result != lSize)
	{
		fputs("Reading error",stderr);
		return false;
	}

	// Copy buffer to Chip8 memory
	if((4096-512) > lSize)
	{
		for(int i = 0; i < lSize; ++i)
			memory[i + 512] = buffer[i];
	}
	else
		printf("Error: ROM too big for memory");

	// Close file, free buffer
	fclose(pFile);
	free(buffer);

	return true;
}
