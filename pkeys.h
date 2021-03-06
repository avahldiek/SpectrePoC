/* Based on Dave Hansen's pku example
 */

#ifndef _PKEYS_HELPER_H
#define _PKEYS_HELPER_H
#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>


#define NR_PKEYS 16
#define PKEY_DISABLE_ACCESS 0x1
#define PKEY_DISABLE_WRITE 0x2
/*
 * pkru intrinsics
 */

#define __rdpkru()                              \
  ({                                            \
    unsigned int eax, edx;                      \
    unsigned int ecx = 0;                       \
    unsigned int pkru;                          \
    asm volatile(".byte 0x0f,0x01,0xee\n\t"     \
                 : "=a" (eax), "=d" (edx)       \
                 : "c" (ecx));                  \
    pkru = eax;                                 \
    pkru;                                       \
  })


#define __wrpkru(PKRU_ARG)						\
  do {											\
    asm volatile ("xor %%ecx, %%ecx\n\txor %%edx, %%edx\n\tmov %0,%%eax\n\t.byte 0x0f,0x01,0xef\n\t" \
	      : : "n" (PKRU_ARG)					\
	      :"eax", "ecx", "edx");			\
  } while (0)

/*
 * Syscalls
 */
#ifndef SYS_mprotect_key
#define SYS_mprotect_key 329//__NR_pkey_mprotect
#define SYS_pkey_alloc   330//__NR_pkey_alloc
#define SYS_pkey_free    331//__NR_pkey_free
#endif     

#define pkey_mprotect(ptr, size, flags, pkey)         \
  syscall(SYS_mprotect_key, ptr, size, flags, pkey)

#define pkey_alloc(pkey, init_val)              \
  syscall(SYS_pkey_alloc, pkey, init_val)

#define pkey_free(pkey)                         \
  syscall(SYS_pkey_free, pkey)

/*
 * Function to check if machine has pku
 */

#define PAGE_SIZE 4096
#define MB	(1<<20)

#define __cpuid(eax, ebx, ecx, edx)             \
  {                                             \
    asm volatile(                               \
                 "cpuid;"                       \
                 : "=a" (*eax),                 \
                   "=b" (*ebx),                 \
                   "=c" (*ecx),                 \
                   "=d" (*edx)                  \
                 : "0" (*eax), "2" (*ecx));     \
  }

/* Intel-defined CPU features, CPUID level 0x00000007:0 (ecx) */
#define X86_FEATURE_PKU        (1<<3) /* Protection Keys for Userspace */
#define X86_FEATURE_OSPKE      (1<<4) /* OS Protection Keys Enable */

// Returns 1 if CPU supports PKU and is enabled by OS
#define cpu_has_pku                                                     \
  {                                                                     \
    unsigned int eax;                                                   \
    unsigned int ebx;                                                   \
    unsigned int ecx;                                                   \
    unsigned int edx;                                                   \
    eax = 0x7;                                                          \
    ecx = 0x0;                                                          \
    __cpuid(&eax, &ebx, &ecx, &edx);                                    \
    if ((!(ecx & X86_FEATURE_PKU)) || (!(ecx & X86_FEATURE_OSPKE))) {   \
      fprintf(stderr, "cpu does not have PKU or OSPKE\n");              \
		0;                                                                \
    } else {                                                            \
      1;                                                                \
    }                                                                   \
  }

#endif /* _PKEYS_HELPER_H */
