

class chip8
{
    public:
        bool drawFlag;
        void initialize();
        void disp_clear();
        void emulateCycle();
        bool loadApplication(const char * filename);

        unsigned char memory [4096];  //4K memory with addresses of 8 bits
        unsigned short opcode; //short = 2 bytes & all opcodes are 2 bytes
        unsigned char V [16]; //16 registers with naming convention V

        unsigned short I; //I register
        unsigned short pc; //program counter
        unsigned short sp; //stack pointer --> number of addresses = 0x000 - 0xFFF & 8 bits = 1 byte so have 12 bits or 2 bytes - 4 bits
        unsigned short stack1 [16]; //stack has 16 levels of support --> can't use stack name b/c keyword

        unsigned char screen [32*64]; //size of screen
/*--> couldn't we use a double array? & if it's only 1 or 0 can't we use something smaller than char?--> no such thing so have to do this
layout of screen is:
 -------------------------
| (0,0)           (0,63)  |
|                         |
|                         |
| (31,0)          (31,63) |
 -------------------------
 We cannot do a double array b/c char(byte) is the smallest unit and each array indice would be either 0 or 1 to denote pixel on or off
 Thus, we use a char array which is 8 so when we actually draw each pixel, the pixel is composed of 8 bits (even though it's supposed to be 1)
 and we take account of this when we draw

    Layout of screen:
 indices | coordinates
 0-63 --> [(0,0) --> (63,0)]
64 - 127 --> [(1,1) --> (63,1)]
       ...
*/

        unsigned char delayTimer;   //why char --> stored in the V registers
        unsigned char soundTimer;  //why char --> stored in V registers
        unsigned char keypad; //[16]; //16 buttons

        static unsigned char chip8_fontset[80]; /*=
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
        };*/

};


