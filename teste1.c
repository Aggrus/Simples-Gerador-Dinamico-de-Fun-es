#include <stdio.h>
#include "cria_func.h"
#include "aux.h"

#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>

#define PAGE_START(P) ((intptr_t)(P)&~(pagesize-1))
#define PAGE_END(P) (((intptr_t)(P)+pagesize-1)&~(pagesize-1))

/*
 * The execpage() function shall change the specified memory pages
 * permissions into executable.
 *
 * void *ptr  = pointer to start of memory buff
 * size_t len = memory buff size in bytes
 *
 * The function returns 0 if successful and -1 if any error is encountered.
 * errono may be used to diagnose the error.
 */
int execpage(void *ptr, size_t len) {
	int ret;

	const long pagesize = sysconf(_SC_PAGE_SIZE);
	if (pagesize == -1)
		return -1;

	ret = mprotect((void *)PAGE_START(ptr),
		 PAGE_END((intptr_t)ptr + len) - PAGE_START(ptr),
		 PROT_READ | PROT_WRITE | PROT_EXEC);
	if (ret == -1)
		return -1;

	return 0;
}

typedef int (*func_ptr) (int x);

int mult(int x, int y) {
  return x * y;
}

unsigned char codigo[500];

int main (void) {
    execpage(codigo, (size_t) 500);
    DescParam params[2];
    func_ptr f_mult;
    int i;
    
    params[0].tipo_val = INT_PAR; /* o primeiro parãmetro de mult é int */
    params[0].orig_val = PARAM;   /* a nova função repassa seu parämetro */

    params[1].tipo_val = INT_PAR; /* o segundo parâmetro de mult é int */
    params[1].orig_val = FIX;     /* a nova função passa para mult a constante 10 */
    params[1].valor.v_int = 10;

    cria_func (mult, params, 2, codigo);
    f_mult = (func_ptr) codigo;   

    for (i = 1; i <=10; i++) {
    printf("%d\n", f_mult(i)); /* a nova função só recebe um argumento */
    }
    int sizeOfCodigo = getPos();
    printf("Pos:%d\n", sizeOfCodigo);
    for (int j = 0; j < sizeOfCodigo; j++)
    {
        if (j%10 == 0)
        {
            printf("\n");
        }
        printf("%02X ", codigo[j]);
    }
    printf("\n");
    return 0;
}

#undef PAGE_START
#undef PAGE_END