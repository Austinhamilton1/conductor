#if defined(__GNUC__) || defined(__clang__)
    #define CONDUCTOR_PREFETCH_READ(address) \
        __builtin_prefetch((address), 0, 3)
#elif defined(_MSC_VER)
    #include <intrin.h>

    #define CONDUCTOR_PREFETCH_READ(address) \
        _mm_prefetch(reinterpret_cast<const char *>(address), _MM_HINT_TO)
#else
    #define CONDUCTOR_PREFETCH_READ(address) ((void)0)
#endif