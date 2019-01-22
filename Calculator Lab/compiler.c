#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


const char *callerRegs[] = {"%r10", "%r11"};         //For keeping track of the registers being used
const char *argRegs[] = {"%rdi", "%rsi", "%rdx"};

int checkVariable(char *token);


int main(int argc, char *argv[])
{
    printf("   .globl compute\n\n");
    printf("compute: \n");

    int stackCount = 0;   //Count of the current stack
    int opCount = 0;      //Count of operations (shouldn't be more than one at any given time)
    int argCount = 0;     //Number of arguments that will be operated on (either 1 or 2 at any given time)
    int variableCount = 0; //For x, y and z registers (rdi, rsi, rdx)

    char* currentOp;      //the current op that will be performed

    int i;
    for(i = 1; argv[i] != 0; i++){
        //If the current value is an operator
        if((strcmp(argv[i], "+")==0) ^ (strcmp(argv[i], "-")==0) ^ (strcmp(argv[i], "t")==0) ^ (strcmp(argv[i],"n")==0)){
            if((strcmp(argv[i],"+")==0) ^ (strcmp(argv[i],"-")==0) ^ (strcmp(argv[i],"t")==0) && (stackCount < 2)){
                fprintf(stderr, "Error: The %s operator requires two arguments\n", argv[i]);
                return 1;
            }
            if((strcmp(argv[i], "n")==0) && (stackCount == 0)){
                fprintf(stderr, "Error: The %s operator requires an argument\n", argv[i]);
                return 1;
            }

            stackCount = stackCount - 2;
            currentOp = argv[i];            //current Operator is this

            if(strcmp(currentOp, "n")==0){
                stackCount = stackCount + 2;
            }
            opCount++;
        }

        //If current value is a variable
        else if(strcmp(argv[i], "x")==0 || strcmp(argv[i], "y")==0 || strcmp(argv[i], "z")==0){
            //printf("   pushq   %s\n", argRegs[variableCount]);


            if(strcmp(argv[i], "x")==0){
                printf("   pushq   %s\n", argRegs[0]);
            }
	        else if(strcmp(argv[i], "y")==0){
                printf("   pushq   %s\n", argRegs[1]);
            }
            else if(strcmp(argv[i], "z")==0){
                printf("   pushq   %s\n", argRegs[2]);
            }

            stackCount++;        //increase stack
            variableCount++;
            argCount++;

        }
        //If current value is a constant
        else{
            int valid = checkVariable(argv[i]);
            if(valid == 1){  //Checks whether the variable is valid
                fprintf(stderr, "The variable \"%s\" is not valid\n", argv[i]);
                return 1;
            }

            long long num = atoll(argv[i]);

            if(num >= 2147483647 || num <= -2147483647){
                printf("   movq    $%lld, %%r9\n", num);
                printf("   popq    %%r9\n");
                stackCount++;
                argCount++;
            }
            else{
                printf("   pushq   $%s\n", argv[i]);
                stackCount++;
                argCount++;
            }
        }

        //If the operation is +, - or t
        if((argCount>=2) && (opCount==1) && !!(strcmp(currentOp, "n"))){
            printf("   popq    %s\n", callerRegs[opCount-1]);      //Pop the top two values, store in r10 and r11
            printf("   popq    %s\n", callerRegs[opCount]);

            if(strcmp(currentOp, "+") == 0){
                printf("   addq    %s, %s\n", callerRegs[opCount-1], callerRegs[opCount]);
            }
            if(strcmp(currentOp, "-") == 0){
                printf("   subq    %s, %s\n", callerRegs[opCount-1], callerRegs[opCount]);
            }
            if(strcmp(currentOp, "t") == 0){
                printf("   imulq   %s, %s\n", callerRegs[opCount-1], callerRegs[opCount]);
            }

            printf("   pushq   %s\n",  callerRegs[opCount]);     //push back onto stack

            opCount = 0;     //reset operator
            stackCount++;    //Add the new variable back onto the stack
            argCount--;    //this new value is now of size 1
        }
        //If operation is unary negation
        else if(argCount >= 1 && strcmp(currentOp, "n") == 0 && opCount > 0){
            printf("   popq    %s\n", callerRegs[0]);
            printf("   movq    $-1, %s\n", callerRegs[1]);
            printf("   imulq   %s, %s\n", callerRegs[0], callerRegs[1]);
            printf("   pushq   %s\n", callerRegs[1]);

            opCount = 0;
        }

        //If 2 or more arguments and last value is negation
        if(argCount >= 2 && opCount == 1  && strcmp(currentOp, "n") == 0 && argv[i+1] == 0){
            fprintf(stderr, "Error: There will be %d values left on stack", stackCount);
            return 1;
        }
    }

    if(stackCount > 1){
        fprintf(stderr, "Error: There are %d values left on the stack\n", stackCount);
        return 1;
    }

    printf("   popq    %%rax\n");
    printf("   retq\n");

    return 0;
}

//To check whether variable is valid
int checkVariable(char *token){
    char *invalids = "abcdefghijklmnopqrstuvw";
    int i;
    for(i=0;i<23;i++){
        if(token[0]==invalids[i]){
            return 1;
        }
    }
    return 0;
}