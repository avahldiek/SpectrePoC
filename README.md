# SpectrePoC with Intel MPK mitigation

Proof of concept code for the Spectre CPU exploit showcasing the capabilities of Intel Memory Protection Keys (MPK). This PoC produces two different binaries (`./spectre` and `./spectre-fail`). Both binaries follow the same initial setup including creating an MPK domain, allocating a page in the new domain to store the secret and then denying access to the new domain. From this point onwards the execution has no access to the secret. Both binaries differ when it comes to how they run the victim function. `./spectre` runs the victim function with privileges to access the secret by updating the PKRU register (WRPKRU). On the other hand, `./spectre-fail` does not update the PKRU register and runs the victim function without privileges to access the secret. The training fails and the subsequent speculative execution fails to leak the secret.

This shows that in an application with different MPK domains, an attacker has to alter the control flow in such a way that the victim function executes with privileges to access the secret. Otherwise the training fails and no secrets are leaked.

## Attribution

The source code originates from the example code provided in the "Spectre Attacks: Exploiting Speculative Execution" paper found here:

https://spectreattack.com/spectre.pdf

The original source code used in this repository was conveniently provided by Erik August's gist, found here: https://gist.github.com/ErikAugust/724d4a969fb2c6ae1bbd7b2a9e3d4bb6

The code has been modified to fix build issues, add workaround for older CPUs, and improve comments where possible.

## Building

The project can be built with GNU Make and GCC.

On debian these are included in the `build-essential` metapackage.

Building is as easy as:

`cd SpectrePoC`

`make`

The output binary are `./spectre` and `./spectre-fail`.

### Mitigations

Several mitigations are available for Spectre.

These can be can be optionally compiled into the binary in order to test their effectiveness on various processors.

#### Intel lfence style mitigation

If you want to build a version with Intel's lfence mitigation included, set your `CFLAGS`

`CFLAGS=-DINTEL_MITIGATION`

in the `Makefile` or build like

`CFLAGS=-DINTEL_MITIGATION make`

#### Linux kernel style mitigation

If you want to build a version with Linux kernel array_index_mask_nospec() mitigation included, set your `CFLAGS`

`CFLAGS=-DLINUX_KERNEL_MITIGATION`

in the `Makefile` or build like

`CFLAGS=-DLINUX_KERNEL_MITIGATION make`

### Building for older CPUs

Depending on the CPU, certain instructions will need to be disabled in order for the program to run correctly.

The instructions in question are:

#### rdtscp:

Introduced with x86-64.
All 32-bit only CPUs, including many Core 2 Duos, will need to disable this instruction.

To build the project without `rdtscp`, define the NORDTSCP cflag:

`CFLAGS=-DNORDTSCP make` 

#### mfence:
Introduced with SSE2.
Most CPUs pre-Pentium 4 will need to disable this instruction.

To build the project without `mfence`, define the NOMFENCE cflag:

`CFLAGS=-DNOMFENCE make`

#### clflush
Introduced with SSE2.
Most CPUs pre-Pentium 4 will need to disable this instruction.

To build the project without `clflush`, define the NOCLFLUSH cflag:

`CFLAGS=-DNOCLFLUSH make`

#### Multiple cflags

To define multiple cflags, separate each cflag with an escaped space. For example:

`CFLAGS=-DNORDTSCP\ -DNOMFENCE\ -DNOCLFLUSH make`

#### SSE2 instruction set

To build the project without all of the above instructions introduced with SSE2, define NOSSE2 cflag:

`CFLAGS=-DNOSSE2 make`

This flag is automatically enabled if the `__SSE__` flag is present but `__SSE2__` is absent.
This means `NOSSE2` shouldn't need to be manually specified when compiling on Clang or GCC on non-SSE2 processors.

#### 'Target specific option mismatch' error

Some 32-bit versions of gcc (e.g. the version used in Ubuntu 14.04) may show the following error while compiling the PoC:

```
/usr/lib/gcc/i686-linux-gnu/5/include/emmintrin.h:1479:1: error:
  inlining failed in call to always_inline
`_mm_clflush`: target specific option mismatch
 _mm_clflush (void const *__A)
 ^
```

In this case architecture build flag `-march=native` is required for compilation for the current CPU:

`CFLAGS=-march=native make`

This flag builds the binary specifically for the current CPU and it may crash after copying to another machine.

### Building it without using the Makefile

If you want to build it manually, make sure to disable all optimisations (aka, don't use -O2), as it will break the program.

## Executing

To run spectre with default cache hit threshold of 80, and the secret example string "The Magic Words are Squeamish Ossifrage." as the target, run `./spectre.out` with no command line arguments.

**Example:** `./spectre`

The cache hit threshold can be specified as the first command line argument. It must be a whole positive integer.

**Example:** `./spectre 80`

A custom target address and length can be given as the second and third command line arguments, respectively.

**Example:** `./spectre 80 12345678 128`

## Tweaking

If you're getting lackluster results, you may need to tweak the cache hit threshold. This can be done by providing a threshold as the first command line argument.

While a value of 80 appears to work for most desktop CPUs, a larger value may be required for slower CPUs, and the newest desktop CPUs can go as low as 15.
For example, on an Intel(R) Core(TM) i7-8650U CPU (Surface Book 2), a value of 20 works well. On a slower, integrated AMD GX-412TC SOC (PC Engines APU3), a value of 100-300 was required to get a good result.

## Contributing

Feel free to add your results to the "Results" issue. Include your cache hit threshold, OS details, CPU details like vendor Id, family, model name, stepping, microcode, MHz, and cache size. The OS can be found by running `uname -a`. CPU info can be found by running `cat /proc/cpuinfo` on Linux, and `sysctl -a | grep machdep.cpu` on OSX.

## Example output

The following was output on an Intel(R) Xeon(R) Gold 6142 CPU CPU, with a cache hit threshold of 218 showing the indifference of results. Any threshold smaller than 218 results in no hits and hence the last item receives all scores.

`./spectre-FAIL 218:`

```
Using a cache hit threshold of 218.
Build: RDTSCP_SUPPORTED MFENCE_SUPPORTED CLFLUSH_SUPPORTED INTEL_MITIGATION_DISABLED LINUX_KERNEL_MITIGATION_DISABLED MPK_TRAINOUTSIDE_ENABLED
Reading 40 bytes:
Reading at malicious_x = 0xfffffffffffff000... Unclear: 0x31=’1’ score=390 (second best: 0x62=’b’ score=332)
Reading at malicious_x = 0xfffffffffffff001... Unclear: 0x31=’1’ score=367 (second best: 0x58=’X’ score=335)
Reading at malicious_x = 0xfffffffffffff002... Unclear: 0x31=’1’ score=386 (second best: 0x58=’X’ score=309)
Reading at malicious_x = 0xfffffffffffff003... Unclear: 0x31=’1’ score=350 (second best: 0x58=’X’ score=336)
Reading at malicious_x = 0xfffffffffffff004... Unclear: 0x31=’1’ score=361 (second best: 0x58=’X’ score=338)
Reading at malicious_x = 0xfffffffffffff005... Unclear: 0x58=’X’ score=348 (second best: 0x31=’1’ score=346)
Reading at malicious_x = 0xfffffffffffff006... Unclear: 0x31=’1’ score=364 (second best: 0x58=’X’ score=343)
Reading at malicious_x = 0xfffffffffffff007... Unclear: 0x31=’1’ score=382 (second best: 0x62=’b’ score=325)
Reading at malicious_x = 0xfffffffffffff008... Unclear: 0x31=’1’ score=355 (second best: 0x58=’X’ score=320)
Reading at malicious_x = 0xfffffffffffff009... Unclear: 0x62=’b’ score=357 (second best: 0x31=’1’ score=357)
Reading at malicious_x = 0xfffffffffffff00a... Unclear: 0x31=’1’ score=382 (second best: 0x58=’X’ score=341)
Reading at malicious_x = 0xfffffffffffff00b... Unclear: 0x31=’1’ score=351 (second best: 0x62=’b’ score=338)
Reading at malicious_x = 0xfffffffffffff00c... Unclear: 0x31=’1’ score=364 (second best: 0x62=’b’ score=348)
Reading at malicious_x = 0xfffffffffffff00d... Unclear: 0x31=’1’ score=359 (second best: 0x62=’b’ score=353)
Reading at malicious_x = 0xfffffffffffff00e... Unclear: 0x31=’1’ score=363 (second best: 0x62=’b’ score=351)
Reading at malicious_x = 0xfffffffffffff00f... Unclear: 0x31=’1’ score=361 (second best: 0x62=’b’ score=331)
Reading at malicious_x = 0xfffffffffffff010... Unclear: 0x31=’1’ score=372 (second best: 0x62=’b’ score=348)
Reading at malicious_x = 0xfffffffffffff011... Unclear: 0x31=’1’ score=381 (second best: 0x58=’X’ score=347)
Reading at malicious_x = 0xfffffffffffff012... Unclear: 0x31=’1’ score=372 (second best: 0x58=’X’ score=340)
Reading at malicious_x = 0xfffffffffffff013... Unclear: 0x31=’1’ score=382 (second best: 0x62=’b’ score=327)
Reading at malicious_x = 0xfffffffffffff014... Unclear: 0x31=’1’ score=384 (second best: 0x58=’X’ score=342)
Reading at malicious_x = 0xfffffffffffff015... Unclear: 0x31=’1’ score=366 (second best: 0x62=’b’ score=332)
Reading at malicious_x = 0xfffffffffffff016... Unclear: 0x31=’1’ score=382 (second best: 0x62=’b’ score=353)
Reading at malicious_x = 0xfffffffffffff017... Unclear: 0x31=’1’ score=383 (second best: 0x62=’b’ score=331)
Reading at malicious_x = 0xfffffffffffff018... Unclear: 0x31=’1’ score=374 (second best: 0x62=’b’ score=343)
Reading at malicious_x = 0xfffffffffffff019... Unclear: 0x31=’1’ score=369 (second best: 0x58=’X’ score=366)
Reading at malicious_x = 0xfffffffffffff01a... Unclear: 0x31=’1’ score=355 (second best: 0x58=’X’ score=341)
Reading at malicious_x = 0xfffffffffffff01b... Unclear: 0x31=’1’ score=369 (second best: 0x62=’b’ score=351)
Reading at malicious_x = 0xfffffffffffff01c... Unclear: 0x58=’X’ score=350 (second best: 0x31=’1’ score=350)
Reading at malicious_x = 0xfffffffffffff01d... Unclear: 0x31=’1’ score=391 (second best: 0x62=’b’ score=361)
Reading at malicious_x = 0xfffffffffffff01e... Unclear: 0x31=’1’ score=366 (second best: 0x58=’X’ score=325)
Reading at malicious_x = 0xfffffffffffff01f... Unclear: 0x31=’1’ score=390 (second best: 0x58=’X’ score=338)
Reading at malicious_x = 0xfffffffffffff020... Unclear: 0x31=’1’ score=350 (second best: 0x62=’b’ score=335)
Reading at malicious_x = 0xfffffffffffff021... Unclear: 0x31=’1’ score=384 (second best: 0x62=’b’ score=345)
Reading at malicious_x = 0xfffffffffffff022... Unclear: 0x31=’1’ score=367 (second best: 0x62=’b’ score=325)
Reading at malicious_x = 0xfffffffffffff023... Unclear: 0x31=’1’ score=375 (second best: 0x62=’b’ score=349)
Reading at malicious_x = 0xfffffffffffff024... Unclear: 0x31=’1’ score=348 (second best: 0x58=’X’ score=326)
Reading at malicious_x = 0xfffffffffffff025... Unclear: 0x31=’1’ score=372 (second best: 0x62=’b’ score=340)
Reading at malicious_x = 0xfffffffffffff026... Unclear: 0x31=’1’ score=369 (second best: 0x58=’X’ score=344)
Reading at malicious_x = 0xfffffffffffff027... Unclear: 0x58=’X’ score=357 (second best: 0x31=’1’ score=310)
```
