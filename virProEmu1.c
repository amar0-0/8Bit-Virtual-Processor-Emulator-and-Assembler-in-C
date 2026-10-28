#include<stdio.h>
#include<stdint.h>
uint8_t reg[8];
uint8_t memory[256]; 

uint8_t pc;
uint8_t ir;
uint8_t flags; //binary represnetion: NA NA NA NA O N C Z 

               // Z-->Zero flag, C-->Carry flag, N-->Negative flag, O-->Overflow flag, NA-->Not used

uint8_t halt = 0;

/*
Flags are used to store status of ALU after processed some data, the status of flag is not stored just used immediate after ALU use, if we do other operation using ALU now flag will store most recent operation.
for example, our alu is 8bit if we perform addition of 200 and 100, out put will be in binary with a carry flag as 1, and then next instructuion is add 10,12 which can be easily represented in 8 bit so the carry flag will become 0.
FLAGS HAVE FIXED MEANING BUT THE EXACT RULES OF CALCULATING THEM MIGHT DIFFERENT AND DEPENDS ON THE OPERATION.


Zero flag is used/resposible to represnet 1 when ALU results 0 after processing, if z=1 means result is 0 and when z=0 means result is not equal to 0.

Carry flag, used to store/represent a extra bit when generated while performiming arthematic operation by ALU, for example, if we add 255(8bit unsigned number)+120(8bit unsigned number) output is 375, which cannot be represented in 8 bits, means need a carry or requires 9th bit, then carry flag will be 1 as output of ALU by this operation.



Overflow flag, used to detect change in sign or change in MSB bit, only when inputs have same sign/Same MSB, and the result is different MSB then overflow will be 1, for differend MSB inputs no overflow wil occur. example inputs are 01111111, 01111111 then output will be 11111110 and over flow will be 1 as MSB of inputs are 0 and MSB of output/result is 1 so change in MSB leads to overflow 1, if 10010000, 011110011 are inputs then overflow after operation will never be 1 as 2 different signed numbers operation can never result a number outside of signed range so overflow will always 0 for this kind of cases.
input MSB OVERFLOW
0 0        1/0(1 when relult MSB is 1, else 0)
0v1        0 always
1 0        0 always
1 1        1/0(1 when reslut MSB is 0, else 0)
OverFlow is used to determine weather then number produced by the operation is in the range of its signed vlaue(2's complement signed numbers) here in our case -128 to 127, if the produces greater than this, overflow sets 1.
NOTE: ALU doesn't care/know number is signed or unsigned, we use flags for out convinence, so we can know the produced result is right or wrong.
for above example like adding 127+127 produces 254 a valid 8 bit number but this doesn't comes in the range of signed interger, we only refer overflow for signed number operations only or the numbers which are in 2's complement form.

Negative flag, set when MSB if the result is 1, else clear.

*/
enum flagBit{ //flag names
    zero,
    carry,
    negative,
    overflow
};

/* this are the opcodes of instructions, using enum so can directy write in memory array.*/

enum opcode {
    NOP  = 0x00,
    HALT = 0x01,
    LDI  = 0x02,
    MOV  = 0x03,
    LD   = 0x04,
    ST   = 0x05,
    ADD  = 0x06,
    SUB  = 0x07,
    CMP  = 0x08,
    AND  = 0x09,
    OR   = 0x0A,
    XOR  = 0x0B,
    NOT  = 0x0C,
    SHL  = 0x0D,
    SHR  = 0x0E,
    JMP  = 0x0F,
    JZ   = 0x10,
    JNZ  = 0x11,
    JC   = 0x12,
    JN   = 0x13
};

/*This array represent size of each instruction according to the opcode index/enum number*/
uint8_t instructionSize[] = {1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 2, 2, 2, 2, 2 };



void setFlag(enum flagBit bit){
    flags |= 1<<bit;
}
void clearFlag(enum flagBit bit){
    flags &= ~(1<<bit);
}
uint8_t checkFlag(enum flagBit bit){
     return ((flags>>bit)&0x01);
}

uint8_t ALU_add(uint8_t a, uint8_t b){  //didn't promoted size of a+b manually because C automaticall promote every variable size which is less that int size, if C won't do, we can manually do like (uint16_t)a+b, now the result of a+b in 16bits we need this to detect only carry and c upgrades size of 8bit int in this pc so didn't do manual typecast.
    uint8_t result = a+b;
    ((result&0xFF)== 0x00)?setFlag(zero):clearFlag(zero); //updates zero flag
    (a+b>0xFF)?setFlag(carry):clearFlag(carry); //updates carry flag
    ((result&0x80)==0)?clearFlag(negative):setFlag(negative); //updates negative flag, updates if MSB is one
    (((a&0x80) == (b&0x80)) && ((a&0x80) != (result&0x80)))?setFlag(overflow):clearFlag(overflow);//updates overflow flag, overflow flag 1 when change in sign of result comapred to inputs(only when both inputs are of same sign)
    return result;
}

uint8_t ALU_sub(uint8_t a, uint8_t b){
    uint8_t subRes = ALU_add(a,~b+1);
    (((a&0x80)!=(b&0x80)) && ((subRes&0x80)!=(a&0x80)))?setFlag(overflow):clearFlag(overflow);
    return subRes;
}

uint8_t ALU(enum opcode opera, uint8_t a,uint8_t b){
    switch(opera){
        case(ADD): //Add
            return ALU_add(a,b);
        case(SUB): //Sub
            return ALU_sub(a,b);
        case(AND): //Bitwise AND
            (a&b)==0?setFlag(zero):clearFlag(zero);
            ((a&b)&0x80)==0x80?setFlag(negative):clearFlag(negative);
            clearFlag(carry);
            clearFlag(overflow);
            return a&b;
        case(OR): //Bitwise OR
            (a|b)==0?setFlag(zero):clearFlag(zero);
            ((a|b)&0x80)==0x80?setFlag(negative):clearFlag(negative);
            clearFlag(carry);
            clearFlag(overflow);
            return a|b;
        case(XOR): //Bitwise XOR
            (a^b)==0?setFlag(zero):clearFlag(zero);
            ((a^b)&0x80)==0x80?setFlag(negative):clearFlag(negative);
            clearFlag(carry);
            clearFlag(overflow);
            return a^b;
        case(NOT): //Bitwise NOT
            (~a&0xFF)==0?setFlag(zero):clearFlag(zero);
            (~a)>>7==1?setFlag(negative):clearFlag(negative);
            clearFlag(carry);
            clearFlag(overflow);
            return ~a;
        case(SHL): // bits left shift b time
            if(b>7) return 0;
            ((a<<b)&0xFF)==0?setFlag(zero):clearFlag(zero);
            (((a<<b)&0x80)==0x80)?setFlag(negative):clearFlag(negative);
            clearFlag(carry);
            clearFlag(overflow);
            return (a<<b);
        case(SHR): // bits right shift b time
            if(b>7) return 0;
            ((a>>b)&0xFF)==0?setFlag(zero):clearFlag(zero);
            (((a>>b)&0x80)==0x80)?setFlag(negative):clearFlag(negative);
            clearFlag(carry);
            clearFlag(overflow);
            return (a>>b);
        default:
            return 0;
    }
}

void fn_LDI(uint8_t r, uint8_t value){
    reg[r]=value;
}

void fn_MOV(uint8_t rd, uint8_t rs){
    reg[rd]=reg[rs];
}

void fn_LD(uint8_t r,uint8_t ma){
    reg[r]=memory[ma];
}

void fn_ST(uint8_t r, uint8_t ma){
    memory[ma]=reg[r];
}

void fn_CMP(uint8_t ra, uint8_t rb){
    ALU(SUB,reg[ra],reg[rb]);
}

void fn_JMP(uint8_t a){
    pc=a;
}

void fn_JZ(uint8_t a){
    if(checkFlag(zero)==1) pc=a;
}

void fn_JNZ(uint8_t a){
    if(checkFlag(zero)==0) pc=a;
}

void fn_JC(uint8_t a){
    if(checkFlag(carry)==1) pc=a;
}

void fn_JN(uint8_t a){
    if(checkFlag(negative)==1) pc=a;
}

void fn_ADD(uint8_t ra, uint8_t rb) {
    reg[ra] = ALU(ADD, reg[ra], reg[rb]);
}

void fn_SUB(uint8_t ra, uint8_t rb) {
    reg[ra] = ALU(SUB, reg[ra], reg[rb]);
}

void fn_AND(uint8_t ra, uint8_t rb) {
    reg[ra] = ALU(AND, reg[ra], reg[rb]);
}

void fn_OR(uint8_t ra, uint8_t rb) {
    reg[ra] = ALU(OR, reg[ra], reg[rb]);
}

void fn_XOR(uint8_t ra, uint8_t rb) {
    reg[ra] = ALU(XOR, reg[ra], reg[rb]);
}

void fn_NOT(uint8_t r) {
    reg[r] = ALU(NOT, reg[r], 0);
}

void fn_SHL(uint8_t ra, uint8_t rb){
    reg[ra]=ALU(SHL,reg[ra],reg[rb]);
}

void fn_SHR(uint8_t ra, uint8_t rb){
    reg[ra]=ALU(SHR,reg[ra],reg[rb]);
}

/*Control unit, which is used/responsible for fetch, decode, execute
Fetch--> memory address from pc is fecthed and copy instruction form memory to ir using that address

Decode--> read instruction from ir, get size of that instruction, increment pc according to the size of the instruction(generally incrementing pc done in fetch cycle where every instruction have same size, but we are having variable sizes for each instruction so, we are incrementing pc after decoding), then read operands

execute--> generate control signals to implement the program/ execute the instruction

*/
void cu(){
    uint8_t currentLoc = pc;
    ir=memory[pc]; //coping instruction into ir from the memory using pc
    pc+=instructionSize[ir]; //incrementing PC according to instruction size
    switch (ir)
    {
    case NOP:
        break;
    case HALT:
        halt = 1;
        break;
    case LDI:
        fn_LDI(memory[currentLoc+1],memory[currentLoc+2]);
        break;
    case MOV:
        fn_MOV(memory[currentLoc+1],memory[currentLoc+2]);
        break;
    case LD:
        fn_LD(memory[currentLoc+1],memory[currentLoc+2]);
        break;
    case ST:
        fn_ST(memory[currentLoc+1],memory[currentLoc+2]);
        break;
    case ADD:
        fn_ADD(memory[currentLoc+1],memory[currentLoc+2]);
        break;
    case SUB:
        fn_SUB(memory[currentLoc+1],memory[currentLoc+2]);
        break;
    case CMP:
        fn_CMP(memory[currentLoc+1],memory[currentLoc+2]);
        break;
    case AND:
        fn_AND(memory[currentLoc+1],memory[currentLoc+2]);
        break;
    case OR:
        fn_OR(memory[currentLoc+1],memory[currentLoc+2]);
        break;
    case XOR:
        fn_XOR(memory[currentLoc+1],memory[currentLoc+2]);
        break;
    case NOT:
        fn_NOT(memory[currentLoc+1]);
        break;
    case SHL:
        fn_SHL(memory[currentLoc+1],memory[currentLoc+2]);
        break;
    case SHR:
        fn_SHR(memory[currentLoc+1],memory[currentLoc+2]);
        break;
    case JMP:
        fn_JMP(memory[currentLoc+1]);
        break;
    case JZ:
        fn_JZ(memory[currentLoc+1]);
        break;
    case JNZ:
        fn_JNZ(memory[currentLoc+1]);
        break;
    case JC:
        fn_JC(memory[currentLoc+1]);
        break;
    case JN:
        fn_JN(memory[currentLoc+1]);
        break;
    
    
    default:
        break;
    }


}










int loadProgram(uint8_t memory[]) /*READ line by line of program.bin file which contains instructions with data line by line*/
{
    FILE *file = fopen("program.bin", "rb");
    if (file == NULL) return 0;
    size_t bytesRead = fread(memory, sizeof(uint8_t), 256, file);
    fclose(file);
    if (bytesRead != 256) return 0;

    return 1;
}






int main(){
    if (!loadProgram(memory))
        printf("Something went wrong while loading program from program.bin\n");

    while(halt==0 && pc<256){
        cu();
    }
    printf("R0 = %u\n", reg[0]);
    printf("R1 = %u\n", reg[1]);
    printf("R2 = %u\n", reg[2]);
    printf("R3 = %u\n", reg[3]);
    printf("R4 = %u\n", reg[4]);
    printf("R5 = %u\n", reg[5]);
    printf("R6 = %u\n", reg[6]);
    printf("R7 = %u\n", reg[7]);

    printf("\n\nFlags\n\n");
    printf("Zero: %u\n",checkFlag(zero));
    printf("Carry: %u\n",checkFlag(carry));
    printf("Negative: %u\n",checkFlag(negative));
    printf("Overflow: %u\n",checkFlag(overflow));

    printf("\n\nPC = %u\n", pc);
    return 0;
}
