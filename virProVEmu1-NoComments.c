#include<stdio.h>
#include<stdint.h>

uint8_t reg[8];
uint8_t memory[256]; 
uint8_t pc;
uint8_t ir;
uint8_t flags; 
uint8_t halt = 0;

enum flagBit{ 
    zero,
    carry,
    negative,
    overflow
};
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
uint8_t ALU_add(uint8_t a, uint8_t b){ 
    uint8_t result = a+b;
    ((result&0xFF)== 0x00)?setFlag(zero):clearFlag(zero);
    (a+b>0xFF)?setFlag(carry):clearFlag(carry);
    ((result&0x80)==0)?clearFlag(negative):setFlag(negative); 
    (((a&0x80) == (b&0x80)) && ((a&0x80) != (result&0x80)))?setFlag(overflow):clearFlag(overflow);
    return result;
}

uint8_t ALU_sub(uint8_t a, uint8_t b){
    uint8_t subRes = ALU_add(a,~b+1);
    (((a&0x80)!=(b&0x80)) && ((subRes&0x80)!=(a&0x80)))?setFlag(overflow):clearFlag(overflow);
    return subRes;
}

uint8_t ALU(enum opcode opera, uint8_t a,uint8_t b){
    switch(opera){
        case(ADD):
            return ALU_add(a,b);
        case(SUB):
            return ALU_sub(a,b);
        case(AND): 
            (a&b)==0?setFlag(zero):clearFlag(zero);
            ((a&b)&0x80)==0x80?setFlag(negative):clearFlag(negative);
            clearFlag(carry);
            clearFlag(overflow);
            return a&b;
        case(OR): 
            (a|b)==0?setFlag(zero):clearFlag(zero);
            ((a|b)&0x80)==0x80?setFlag(negative):clearFlag(negative);
            clearFlag(carry);
            clearFlag(overflow);
            return a|b;
        case(XOR): 
            (a^b)==0?setFlag(zero):clearFlag(zero);
            ((a^b)&0x80)==0x80?setFlag(negative):clearFlag(negative);
            clearFlag(carry);
            clearFlag(overflow);
            return a^b;
        case(NOT):
            (~a&0xFF)==0?setFlag(zero):clearFlag(zero);
            ~a>>7==1?setFlag(negative):clearFlag(negative);
            clearFlag(carry);
            clearFlag(overflow);
            return ~a;
        case(SHL): 
            if(b>7) return 0;
            ((a<<b)&0xFF)==0?setFlag(zero):clearFlag(zero);
            (((a<<b)&0x80)==0x80)?setFlag(negative):clearFlag(negative);
            clearFlag(carry);
            clearFlag(overflow);
            return (a<<b);
        case(SHR):
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
void cu(){
    uint8_t currentLoc = pc;
    ir=memory[pc]; 
    pc+=instructionSize[ir];
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



int loadProgram(uint8_t memory[])
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