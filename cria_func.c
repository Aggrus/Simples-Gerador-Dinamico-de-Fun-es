/* Gustavo Zalcman - 1921124 */
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

void printCodigo (unsigned char *codigo, int pos);

void cria_func (void* f, DescParam params[], int n, unsigned char codigo[])
{
    int pos = 0;
    initFunc(codigo, &pos);
    setParamsToRegisters(codigo, &pos); //guarda valor de parametros em r8,9, e 10 respectivamente
    unsigned char paramNew = 0;
    for (int paramOrig = 0; paramOrig < n; paramOrig++)
    {
        DescParam param = params[paramOrig];
        if (param.orig_val == PARAM)
        {
            paramMatch(codigo, paramNew, paramOrig, &pos);
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
        }
        else if (param.orig_val == IND)
        {
            if (param.tipo_val == PTR_PAR)
            {
                void *val;
                val = param.valor.v_ptr;
                setParamOrigToFix(codigo, param.tipo_val, val, paramOrig, &pos);
            }
            else if (param.tipo_val == INT_PAR)
            {
                unsigned char movAddr[10]; 
                movQ(movAddr, param.valor.v_ptr, 10, 5, 1);
                addBytes(movAddr, 10, codigo, &pos);
                unsigned char bytes[] = {0x8b, 0x00};
                if (paramOrig == 0)
                {
                    bytes[1] = 0x38;
                }
                else if (paramOrig == 1)
                {
                    bytes[1] = 0x30;
                }
                else if (paramOrig == 2)
                {
                    bytes[1] = 0x10;
                }
                
                addBytes(bytes, 2, codigo, &pos);
            }
        }
    }
    callInstruction (f, codigo, &pos);
    addByteAtPos(codigo, 0xc9, &pos);    //leave
    addByteAtPos(codigo, 0xc3, &pos);    //ret
    printCodigo(codigo, pos);
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
            unsigned char temp[] = { 0x4c, 0x89, 0xc2 }; //mov %r9, %rdx
            moveParam = temp;
        }
        else if (paramNew == 1 && paramOrig == 2)
        {
            unsigned char temp[] = { 0x4c, 0x89, 0xca }; //mov %r10, %rdx
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
        case 5:
        {
            string[len] = (unsigned char)(isAbs?0xb8:0xc0); //rax
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