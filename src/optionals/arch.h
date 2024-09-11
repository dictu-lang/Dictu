#ifndef dictu_system_arch_h
#define dictu_system_arch_h

// https://stackoverflow.com/a/66249936

#if defined(__x86_64__) || defined(_M_X64)
#define SYSTEM_ARCH "x86_64"
#elif defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
#define SYSTEM_ARCH "x86_32"
#elif defined(__ARM_ARCH_2__)
#define SYSTEM_ARCH "arm2"
#elif defined(__ARM_ARCH_3__) || defined(__ARM_ARCH_3M__)
#define SYSTEM_ARCH "arm3"
#elif defined(__ARM_ARCH_4T__) || defined(__TARGET_ARM_4T)
#define SYSTEM_ARCH "arm4t"
#elif defined(__ARM_ARCH_5_) || defined(__ARM_ARCH_5E_)
#define SYSTEM_ARCH "arm6"
#elif defined(__ARM_ARCH_6T2_) || defined(__ARM_ARCH_6T2_)
#define SYSTEM_ARCH "arm6t2"
#elif defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) ||                   \
    defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6Z__) ||                    \
    defined(__ARM_ARCH_6ZK__)
#define SYSTEM_ARCH "arm6"
#elif defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) ||                   \
    defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) ||                    \
    defined(__ARM_ARCH_7S__)
#define SYSTEM_ARCH "arm7"
#elif defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) ||                  \
    defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
#define SYSTEM_ARCH "arm7a"
#elif defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) ||                  \
    defined(__ARM_ARCH_7S__)
#define SYSTEM_ARCH "arm7r"
#elif defined(__ARM_ARCH_7M__)
#define SYSTEM_ARCH "arm7m"
#elif defined(__ARM_ARCH_7S__)
#define SYSTEM_ARCH "arm7s"
#elif defined(__aarch64__) || defined(_M_ARM64)
#define SYSTEM_ARCH "arm64"
#elif defined(mips) || defined(__mips__) || defined(__mips)
#define SYSTEM_ARCH "mips"
#elif defined(__sh__)
#define SYSTEM_ARCH "superh"
#elif defined(__powerpc) || defined(__powerpc__) || defined(__powerpc64__) ||  \
    defined(__POWERPC__) || defined(__ppc__) || defined(__PPC__) ||            \
    defined(_ARCH_PPC)
#define SYSTEM_ARCH "powerpc"
#elif defined(__PPC64__) || defined(__ppc64__) || defined(_ARCH_PPC64)
#define SYSTEM_ARCH "powerpc_x64"
#elif defined(__sparc__) || defined(__sparc)
#define SYSTEM_ARCH "spark"
#elif defined(__m68k__)
#define SYSTEM_ARCH "m68k"
#else
#define SYSTEM_ARCH "unknown"
#endif

#endif