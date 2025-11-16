// src/config.inc.c --- to be included first with -include

/*    Copyright (C) 2018-2025 Alexey Protasov (AKA Alex or rusini)

   This file is part of MANOOL.

   MANOOL is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
   as published by the Free Software Foundation (and only version 3).

   MANOOL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with MANOOL.  If not, see <https://www.gnu.org/licenses/>.  */


# if __STDC_VERSION__ < 201112/*C11*/ || !__STDC_HOSTED__ || !__GNUC__/*gcc/clang/icx...*/ || !__STRICT_ANSI__/*-std=cNN*/
   # error "Unsupported C compiler or compiler mode"
# endif
# if __STDC_VERSION__ > 201112
   # warning "C compiler mode enabling a more recent spec may be backward-incompatible"
# endif

// Feature-Test Macros (think about ABI-breaking, especially _FILE_OFFSET_BITS if needed)
# define _GNU_SOURCE // just ignored on many platforms not using glibc

# include <assert.h>
# include <stdbool.h>

# include <limits.h>
# include <stdint.h>

