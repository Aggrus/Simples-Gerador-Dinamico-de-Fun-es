#include <stdio.h>
#include "cria_func.h"

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


typedef int (*func_ptr) (void* candidata/* , size_t n */, void *suplemento);

char fixa[] = "%s string é um prefixo dessa";

unsigned char codigo[120];

int main (void) {
  execpage(codigo, (size_t) 120);
  DescParam params[2];
  func_ptr print;
  char s[] = "quero saber se a outra";
  int tam;

  params[0].tipo_val = PTR_PAR; /* o primeiro parâmetro de memcmp é um ponteiro para char */
  params[0].orig_val = PARAM;     /* a nova função passa para memcmp o endereço da string "fixa" */
  //params[0].valor.v_ptr = fixa;

  params[1].tipo_val = PTR_PAR; /* o segundo parâmetro de memcmp é também um ponteiro para char */
  params[1].orig_val = PARAM;   /* a nova função recebe esse ponteiro e repassa para memcmp */

//  params[2].tipo_val = INT_PAR; /* o terceiro parâmetro de memcmp é um inteiro */
//  params[2].orig_val = PARAM;   /* a nova função recebe esse inteiro e repassa para memcmp */

  cria_func (printf, params, 2, codigo);
  print = (func_ptr) codigo;
  print(fixa, s);

  /* tam = 12;
  printf ("'%s' tem mesmo prefixo-%d de '%s'? %s\n", s, tam, fixa, mesmo_prefixo (s, tam)?"NAO":"SIM");
  tam = strlen(s);
  printf ("'%s' tem mesmo prefixo-%d de '%s'? %s\n", s, tam, fixa, mesmo_prefixo (s, tam)?"NAO":"SIM");
 */
  return 0;
}


#undef PAGE_START
#undef PAGE_END