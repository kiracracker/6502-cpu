#include <stdio.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

using Byte = unsigned char;
using Word = unsigned short;
using u32 = unsigned int;


struct Mem
{
   static constexpr u32 MAX_MEM = 1024 * 64;
   Byte Data[MAX_MEM];
   void init(){
    for(u32 i = 0; i < MAX_MEM; i++) {
        Data[i] = 0;
    }
   }

   Byte operator[] (u32 Address) const {
    return Data[Address];
   }

   Byte &operator[] (u32 Address)  {
    return Data[Address];
   }
};

struct CPU
{


Word PC;
Word SP;

Byte A , X , Y;
Byte C : 1;
Byte Z : 1;
Byte I : 1;
Byte D : 1;
Byte B : 1;
Byte V : 1;
Byte N : 1;

void Reset(Mem &memory) {
    PC = 0xFFFC;
    SP = 0x0100;
    C = Z = I = D = B = V = N = 0;
    A = X = Y = 0;
    memory.init();


}

Byte FetchByte(u32* Cycles , Mem &memory) {
    Byte Data = memory[PC];
    PC++;
    (*Cycles)--;
    return Data;
}

Byte ReadByte (u32* Cycles ,Byte Address, Mem &memory) {
    Byte Data = memory[Address];
    (*Cycles)--;
    return Data;

}


void INS_LDA_IM(u32* Cycles, Mem& memory)
       
{
    Byte value = FetchByte(Cycles, memory);
    A = value;
    SetZeroAndNegativeFlags(A);
}


void INS_LDA_ZP(u32* Cycles ,Mem& memory) {
            Byte ZeropageAddress = FetchByte(Cycles , memory);
            A = ReadByte(Cycles,ZeropageAddress , memory);
            SetZeroAndNegativeFlags(A);
}



void INS_LDX_IM(u32* Cycles, Mem& memory)
{
        Byte value = FetchByte(Cycles, memory);
        X = value;
        SetZeroAndNegativeFlags(X);
}

void INS_LDX_ZP(u32* Cycles ,Mem& memory) {
            Byte ZeropageAddress = FetchByte(Cycles , memory);
            X = ReadByte(Cycles,ZeropageAddress , memory);
            SetZeroAndNegativeFlags(X);
}


void INS_LDY_IM(u32* Cycles, Mem& memory)
{
        Byte value = FetchByte(Cycles, memory);
        Y = value;
        SetZeroAndNegativeFlags(Y);
}

void INS_LDY_ZP(u32* Cycles ,Mem& memory) {
            Byte ZeropageAddress = FetchByte(Cycles , memory);
            Y = ReadByte(Cycles,ZeropageAddress , memory);
            SetZeroAndNegativeFlags(Y);
}


void INS_STA_ZP(u32* Cycles ,Mem& memory) {
            Byte addr = FetchByte(Cycles , memory);
            memory[addr] = A;
            (*Cycles)--;

}


void INS_STX_ZP(u32* Cycles ,Mem& memory) {
            Byte addr = FetchByte(Cycles , memory);
            memory[addr] = X;
            (*Cycles)--;

}


void INS_STY_ZP(u32* Cycles ,Mem& memory) {
            Byte addr = FetchByte(Cycles , memory);
            memory[addr] = Y;
            (*Cycles)--;

}

void INS_INX(u32* Cycles ) {
            X++;
            SetZeroAndNegativeFlags(X);
            (*Cycles)--;
}

void INS_INY(u32 * Cycles) {
            Y++;
            SetZeroAndNegativeFlags(Y);
            (*Cycles)--;
}

void INS_DEX(u32* Cycles ) {
            X--;
            SetZeroAndNegativeFlags(X);
            (*Cycles)--;
}

void INS_DEY(u32 * Cycles) {
            Y--;
            SetZeroAndNegativeFlags(Y);
            (*Cycles)--;
}

void INS_TAX(u32 * Cycles) {
            X = A;
            SetZeroAndNegativeFlags(X);
            (*Cycles)--;
}


void INS_TAY(u32 * Cycles) {

            Y = A;
            SetZeroAndNegativeFlags(Y);
            (*Cycles)--;
}


void INS_TXA(u32 * Cycles) {

            A = X;
            SetZeroAndNegativeFlags(A);
            (*Cycles)--;
}


void INS_TYA(u32 * Cycles) {

            A = Y;
            SetZeroAndNegativeFlags(A);
            (*Cycles)--;
}

void INS_NOP(u32 * Cycles) {

            (*Cycles)--;
}

void INS_ADC_IM(u32 * Cycles  , Mem &memory) {
        Byte Value = FetchByte(Cycles, memory);
            
            Word Sum = A + Value + C;

            if (Sum > 0xFF) C = 1;
            if (CheckOverflow(A , Value , (Byte)Sum) == true) V = 1;
            else V = 0;
            A = (Byte)Sum;

            SetZeroAndNegativeFlags(A);
            (*Cycles)--;
}





 // opcode
static constexpr Byte 
    OP_BRK = 0x00, 
    OP_LDA_IM = 0xA9,
    OP_LDA_ZP = 0xA5,
    OP_LDX_IM = 0xA2,
    OP_LDX_ZP = 0xA6,
    OP_LDY_IM = 0xA0,
    OP_LDY_ZP = 0xA4,
    OP_STA_ZP = 0x85,
    OP_STX_ZP = 0x86,
    OP_STY_ZP = 0x84,
    OP_INX = 0xE8,
    OP_INY = 0xC8,
    OP_DEX = 0xCA, 
    OP_DEY = 0x88,
    OP_TAX = 0xAA,
    OP_TAY = 0xA8,
    OP_TXA = 0x8A,
    OP_TYA = 0x98,
    OP_NOP = 0xEA,
    OP_ADC_IM = 0x69;


bool CheckOverflow(Byte A, Byte Value, Byte Result) 
{
 bool A_Negative = (A & 0x80) != 0;
 bool VNeg = (Value & 0x80 )!= 0;
 bool RNeg = (Result & 0x80 )!= 0;

 if (!A_Negative && !VNeg && RNeg)
    return true;

if (A_Negative && VNeg && !RNeg)
    return true;

return false;
}

    void SetZeroAndNegativeFlags(Byte value) {
        Z = (value == 0);
        N = (value & 0b10000000) > 0 ;
    }

void Execute(u32 *Cycles , Mem &memory) {

    while( *(Cycles) > 0 ) {

        Byte Ins = FetchByte(Cycles , memory);
        switch (Ins) {
        case OP_LDA_IM:
        {
           CPU::INS_LDA_IM(Cycles , memory);

        } break;


        case OP_LDA_ZP: {
           CPU::INS_LDA_ZP(Cycles , memory);
        } break;

        case OP_LDX_IM: {
            CPU::INS_LDX_IM(Cycles, memory);

        } break;


        case OP_LDX_ZP: {
           CPU::INS_LDX_ZP(Cycles , memory);

        }break;

         case OP_LDY_IM:
        {
           CPU::INS_LDY_IM(Cycles , memory);

        } break;

         case OP_LDY_ZP:
        {
           CPU::INS_LDY_ZP(Cycles , memory);

        } break;

           case OP_STA_ZP:
        {
           CPU::INS_STA_ZP(Cycles , memory);

        } break;
           case OP_STX_ZP:
        {
           CPU::INS_STX_ZP(Cycles , memory);

        } break;
           case OP_STY_ZP:
        {
           CPU::INS_STY_ZP(Cycles , memory);

        } break;

        case OP_INX: {
            CPU::INS_INX(Cycles);
        } break;

        case OP_INY: {
            CPU::INS_INY(Cycles);
        } break;

        case OP_DEX: {
            CPU::INS_DEX(Cycles);
        } break;

        case OP_DEY : {
            CPU::INS_DEY(Cycles);
        } break;

        case OP_TAX: {
            CPU::INS_TAX(Cycles);
        } break;

        case OP_TAY: {
            CPU::INS_TAY(Cycles);
        } break;

        case OP_TXA: {
            CPU::INS_TXA(Cycles);
        } break;

        case OP_TYA: {
            CPU::INS_TYA(Cycles);
        } break;
        case OP_NOP: {
            CPU::INS_NOP(Cycles);
        }

        case OP_ADC_IM: {
            CPU::INS_ADC_IM(Cycles , memory);
        }

         case 0x00: {
            return;
        }

        default: {
            printf("no instrction %d" , Ins);

        } break;
        }
      

    }
}

};

int main() {
    
   Mem mem;
   CPU cpu;
   cpu.Reset(mem);

mem[0xFFFC] = CPU::OP_LDA_IM;
mem[0xFFFD] = 255;

mem[0xFFFE] = CPU::OP_ADC_IM;
mem[0xFFFF] = 1;

mem[0x0000] = CPU::OP_BRK;
u32 Cycles = 100;

cpu.Execute(&Cycles, mem);

printf("A = %u\n", cpu.A);
printf("C = %u\n", cpu.C);
printf("V = %u\n", cpu.V);
printf("Z = %u\n", cpu.Z);
printf("N = %u\n", cpu.N);

std::cout << cpu.CheckOverflow(-10 , -10 , -20);

    return 0;
}

