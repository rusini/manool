// config.tcc
// Feature-Test Macros (think about ABI-breaking, especially _FILE_OFFSET_BITS if needed)
# ifndef _GNU_SOURCE
# define _GNU_SOURCE // required anyway by libstdc++ (but ignored on BSDs)
# endif
