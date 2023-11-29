#include <stdio.h>
#include <stdlib.h>
#include "cria_func.h"

void cria_func (void* f, DescParam params[], int n, unsigned char codigo[]);
void initFunc (unsigned char *c, int *pos);
void addByteAtPos (unsigned char *codigo, char c, int *pos);
void paramMatch (unsigned char *codigo, int paramNew, int paramOrig, int *pos);
void addBytes(unsigned char *bytes, int size, unsigned char *codigo, int *pos);
void setParamOrigToFix(unsigned char *codigo, TipoValor tpVal, void *valor, unsigned int paramOrig, int *pos);
void callInstruction (void *f, unsigned char *codigo, int *pos);
void addByteToStringAtLen (unsigned char *string, int paramOrig, int len, int isAbs);
void movQ (unsigned char *bytes, void *valor, int size, int paramOrig, int isAbs);
void setParamsToRegisters(unsigned char *codigo, int *pos);

int getPos ();
void printCodigo (unsigned char *codigo, int pos);

unsigned int absPos = 0;


//Passos:
    //  - inicializar função (OK!)
    //  - for n iterar params (1 a 3) 
    //      - Se for PARAM:
    //          - Botar o próximo parâmetro (ou o 1o se não tem nenhum ainda) como o próximo parâmetro de call (OK!)
    //      - Se for FIX:
    //          - Botar o valor fixo dado em params como o próximo parâmetro de call (Parcial)
    //      - Se for IND:
    //          - Checa se o tamanho é de int ou ponteiro, se for int retorna um erro
    //          - Pega o valor dado como um endereço e usa o valor neste endereço como o próximo parâmetro de call
    //  - call usando endereço dinâmico (Lab13)
    //  - ret; leave;
    // Zerar variável global de elemento de "código"

    //Auxiliar:
    //  - Função que adiciona instrução de código de máquina para "codigo" (usar variável para saber último elemento de código) (OK!)
    //  - Função que pega deslocamento de call
//
void cria_func (void* f, DescParam params[], int n, unsigned char codigo[])
{
    int pos = 0;
    absPos = pos;
    initFunc(codigo, &pos);
    printf("Init: \n");
    printCodigo(codigo, pos);
    setParamsToRegisters(codigo, &pos); //guarda valor de parametros em r8,9, e 10 respectivamente
    printf("paramsToRegisters: \n");
    printCodigo(codigo, pos);
    unsigned char paramNew = 0;
    for (int paramOrig = 0; paramOrig < n; paramOrig++)
    {
        DescParam param = params[paramOrig];
        if (param.orig_val == PARAM)
        {
            paramMatch(codigo, paramNew, paramOrig, &pos);
            printf("PARAM: \n");
            printCodigo(codigo, pos);
            paramNew++;
        }
        else if (param.orig_val == FIX)
        {
            void *val;
            if (param.tipo_val == PTR_PAR)
            {
                val = param.valor.v_ptr;
            }
            else
            {
                val = &(param.valor.v_int);
            }
            setParamOrigToFix(codigo, param.tipo_val, val, paramOrig, &pos);
            printf("FIX: \n");
            printCodigo(codigo, pos);
        }
        else if (param.orig_val == IND)
        {
            //TODO: IND processing
        }
    }
    callInstruction (f, codigo, &pos);
    printf("call: \n");
    printCodigo(codigo, pos);
    addByteAtPos(codigo, 0xc9, &pos);    //leave
    printf("leave: \n");
    printCodigo(codigo, pos);
    addByteAtPos(codigo, 0xc3, &pos);    //ret
    printf("ret: \n");
    printCodigo(codigo, pos);
    absPos = pos;
}

void addByteAtPos (unsigned char *codigo, char c, int *pos)
{
    *(codigo + *pos) = c;
    (*pos)++;
}

void initFunc (unsigned char *c, int *pos)
{
    unsigned char start[] = {0x55, 
    0x48, 0x89, 0xe5};
    addBytes(start, 4, c, pos);
    
}

void paramMatch (unsigned char *codigo, int paramNew, int paramOrig, int *pos)
{
    if (paramNew < paramOrig)
    {
        unsigned char *moveParam;
        if (paramNew == 0 && paramOrig == 1)
        {
            unsigned char temp[] = { 0x4c, 0x89, 0xc6 }; //mov %r8, %rsi
            moveParam = temp;
        }
        else if (paramNew == 0 && paramOrig == 2) 
        {
            unsigned char temp[] = { 0x4c, 0x89, 0xc2 }; //mov %r8, %rdx
            moveParam = temp;
        }
        else if (paramNew == 1 && paramOrig == 2)
        {
            unsigned char temp[] = { 0x4c, 0x89, 0xca }; //mov %r9, %rdx
            moveParam = temp;
        }
        addBytes(moveParam, 3, codigo, pos);
    }
}

void addBytes(unsigned char *bytes, int size, unsigned char *codigo, int *pos)
{
    for (int i = 0; i < size; i++)
    {
        addByteAtPos(codigo, bytes[i], pos);
    }
}

void setParamOrigToFix(unsigned char *codigo, TipoValor tpVal, void *valor, unsigned int paramOrig, int *pos)
{
    int size = 6;
    unsigned char *setFixBytes = (unsigned char *)malloc(size);
    if (tpVal == PTR_PAR)
    {
        size = 10;
        setFixBytes = realloc(setFixBytes, size);
        movQ (setFixBytes, valor, size, paramOrig, 1);
        //
        printf("\nvalor: %p (%s)\n", valor, (unsigned char *)valor);
        //
    }
    else if (tpVal == INT_PAR)
    {
        //movl $valor, %edi/%esi/%edx
        size = 5;
        setFixBytes = realloc(setFixBytes, size);
        int *val = (int *)(setFixBytes + 1);
        *val = *((int *) valor);
        addByteToStringAtLen (setFixBytes, paramOrig, 0, 1);
    }
    else
    {
        printf("Illegal TipoValor %d error: non-existent TipoValor!!", tpVal);
        free(setFixBytes);
        exit(1);
    }
    addBytes(setFixBytes, size, codigo, pos);
    free(setFixBytes);
}

void callInstruction (void *f, unsigned char *codigo, int *pos)
{
    int size = 10;
    movQ (codigo + *pos, f, size, 3, 1); 
    *pos += size;
    unsigned char callRcx[] = {0xff, 0xd1};
    addBytes(callRcx, 2, codigo, pos);
    /* addByteAtPos(codigo, 0xe8, pos);
    int *drift = (int *)(codigo + *pos);
    *drift = f - ((void *)(codigo + *pos + 4)); */
    //printf("foo: %p, codigo: %p, leave: %p\n", f, codigo, (codigo + *pos));
    //*pos += 4;
}

//Adds machine code for mov for rdi/rsi/rdx
void addByteToStringAtLen (unsigned char *string, int paramOrig, int len, int isAbs)
{
    switch (paramOrig)
    {
        case 0:
        {
            string[len] = (unsigned char)(isAbs?0xbf:0xc7); //rdi
            break;
        }
        case 1:
        {
            string[len] = (unsigned char)(isAbs?0xbe:0xc6); //rsi
            break;
        }
        case 2:
        {
            string[len] = (unsigned char)(isAbs?0xba:0xc2); //rdx
            break;
        }
        case 3:
        {
            string[len] = (unsigned char)(isAbs?0xb9:0xc1); //rcx
            break;
        }
        default:
        {
            printf("PTR_PARAM %d parameter error: non-existent parameter!!", paramOrig);
            free(string);
            exit(1);
        }
    }
}


int getPos ()
{
    return absPos;
}

void printCodigo (unsigned char *codigo, int pos)
{
    for (int i = 0; i < pos; i++)
    {
        printf("%02X ", codigo[i]);
    }
    printf("\n");
}


void movQ (unsigned char *bytes, void *valor, int size, int paramOrig, int isAbs)
{
    bytes[0] = 0x48;

    *((void **) (bytes + 2)) = valor;
    addByteToStringAtLen (bytes, paramOrig, 1, isAbs);
}

void setParamsToRegisters(unsigned char *codigo, int *pos)
{
    unsigned char paramsToRegisters[] = {
        0x49, 0x89, 0xf8, //movq %rdi, %r8
        0x49, 0x89, 0xf1, //movq %rdi, %r9
        0x49, 0x89, 0xd2 //movq %rdi, %r10
    };
    addBytes(paramsToRegisters, 9, codigo, pos);
}