#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

uint8_t tmpMemory[256];
uint8_t cycleMemory[3];
int instructionSize[] = {1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 2, 2, 2, 2, 2 }; 
int error=0;

int validateInstruction(char *instruction){
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
    if(!strcmp(instruction,"NOP")) return NOP;
    if(!strcmp(instruction,"HALT")) return HALT;
    if(!strcmp(instruction,"LDI")) return LDI;
    if(!strcmp(instruction,"MOV")) return MOV;
    if(!strcmp(instruction,"LD")) return LD;
    if(!strcmp(instruction,"ST")) return ST;
    if(!strcmp(instruction,"ADD")) return ADD;
    if(!strcmp(instruction,"SUB")) return SUB;
    if(!strcmp(instruction,"CMP")) return CMP;
    if(!strcmp(instruction,"AND")) return AND;
    if(!strcmp(instruction,"OR")) return OR;
    if(!strcmp(instruction,"XOR")) return XOR;
    if(!strcmp(instruction,"NOT")) return NOT;
    if(!strcmp(instruction,"SHL")) return SHL;
    if(!strcmp(instruction,"SHR")) return SHR;
    if(!strcmp(instruction,"JMP")) return JMP;
    if(!strcmp(instruction,"JZ")) return JZ;
    if(!strcmp(instruction,"JNZ")) return JNZ;
    if(!strcmp(instruction,"JC")) return JC;
    if(!strcmp(instruction,"JN")) return JN;
    else return -1;
}

int validateData(int opCode, int oper1, int oper2){
    int reqOperands = instructionSize[opCode]-1;
    if(reqOperands==1){
        switch(opCode){
            case 0x0C:
                if(oper1>=0 && oper1<8) return oper1;
                else return -1;
            case 0x0F:
            case 0x10:
            case 0x11:
            case 0x12:
            case 0x13:
                if(oper1>=0 && oper1<256) return oper1;
                else return -1;
            default:
                return -1;
        }
    }
    else if(reqOperands==2){
        switch(opCode){
            case 0x03:
            case 0x06:
            case 0x07:
            case 0x08:
            case 0x09:
            case 0x0A:
            case 0x0B:
                if((oper1>=0 && oper1<8) && (oper2>=0 && oper2<8)) return 1;
                else return -1;
            case 0x02:
            case 0x04:
            case 0x05:
            case 0x0D:
            case 0x0E:
                if((oper1>=0 && oper1<8) && (oper2>=0 && oper2<256)) return 1;
                else return -1;
            default:
                return -1;
            
        }
    }
}





int writeProgramBin(uint8_t tmpMemory[])
{
    FILE *file = fopen("program.bin", "wb");

    if (file == NULL) {
        printf("Could not create program.bin\n");
        return 0;
    }
    size_t bytesWritten = fwrite(tmpMemory, sizeof(uint8_t), 256, file);
    fclose(file);
    if (bytesWritten != 256) return 0;
    printf("Successfully created program.bin\n");
    return 1;
}





int main(void)
{
    FILE *file = fopen("program.txt", "r");

    if (file == NULL) {
        printf("Could not open program.txt\n");
        return 1;
    }
    int instructionOpcode=-1;
    char line[100];
    int lineCount=1;
    int programByteCount=0;
    while (fgets(line, sizeof(line), file) != NULL) {
        char *readInstruction = strtok(line," \t\n");
        if(readInstruction==NULL) continue;
        else instructionOpcode = validateInstruction(readInstruction);
        if(instructionOpcode==-1){
            error+=1;
            printf("invalid instruction at line %d\n",lineCount);
        }

        else{
            if((programByteCount+instructionSize[instructionOpcode])>256){
                printf("Excedees Memory");
                error+=1;
                break;
            }
            if(instructionSize[instructionOpcode]==1) cycleMemory[0]=instructionOpcode;
            if(instructionSize[instructionOpcode]==2){
                char *operand1=strtok(NULL," \t\n");
                if(operand1==NULL){
                    error+=1;
                    printf("need a operand at line %d\n",lineCount);
                }
                else{
                    cycleMemory[0]=instructionOpcode;
                    int opera=validateData(instructionOpcode, atoi(operand1),0);
                    if(opera==-1){
                        error+=1;
                        printf("enter a valid operand at %d\n",lineCount);
                    }
                    else{
                        cycleMemory[1]=opera;
                        cycleMemory[2]=0;
                    }
                }

            }
            else if(instructionSize[instructionOpcode]==3){
                char *operand1=strtok(NULL," \t\n");
                char *operand2=strtok(NULL," \t\n");
                if(operand1==NULL || operand2==NULL){
                    error+=1;
                    printf("need 2 operands at line %d\n",lineCount);
                }
                else{
                    cycleMemory[0]=instructionOpcode;
                    int opera=validateData(instructionOpcode, atoi(operand1),atoi(operand2));
                    if(opera==-1){
                        error+=1;
                        printf("enter a valid operand at %d\n",lineCount);
                    }
                    else{
                        cycleMemory[1]=atoi(operand1);
                        cycleMemory[2]=atoi(operand2);
                    }

                }
            }

        }
        if(error==0){
            for(int i=0;i<instructionSize[instructionOpcode];i++){
                tmpMemory[programByteCount+i]=cycleMemory[i];
            }
        }
        programByteCount+=(instructionOpcode>-1)?(instructionSize[instructionOpcode]):0; 
        lineCount++;
    }

    fclose(file);
    if(error==0){
        if(programByteCount<256){
            tmpMemory[programByteCount]=0x01;
            programByteCount+=1;
        }
        for(int i=programByteCount;i<256;i++){
            if(i>255) break;
            tmpMemory[i]=0;
        }
        char loadConform;
        printf("Sucessfully assembled\n");
        printf("Total %d bytes\n",programByteCount);
        printf("Generate Binary FIle[y/n]: ");
        scanf("%c",&loadConform);
        if(loadConform=='y' || loadConform=='Y'){
            if (writeProgramBin(tmpMemory)) {
                printf("program.bin successfully generated.\n");
            }
            else{
                printf("Something went wrong while generating program.bin File...\n");
            }
        }
        else{
            printf("\nprogtam.bin not generated\n");
        }
    }
    else{
        printf("Total errors found %d\n",error);
    }
    return 0;
}