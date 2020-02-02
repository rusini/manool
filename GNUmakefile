# GNUmakefile

#    Copyright (C) 2018, 2019, 2020 Alexey Protasov (AKA Alex or rusini)
#
# This file is part of MANOOL.
#
# MANOOL is free software: you can redistribute it and/or modify it under the terms of the version 3 of the GNU General Public License
# as published by the Free Software Foundation (and only version 3).
#
# MANOOL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with MANOOL.  If not, see <https://www.gnu.org/licenses/>.


# Configuration Variables ######################################################################################################################################
CC       = $(SCL) $(GCC) $(PIPE) -w $(MARCH) -pthread -std=c99
CXX      = $(SCL) $(GXX) $(PIPE) -w $(MARCH) -pthread -std=c++11
CPPFLAGS =
CFLAGS   = -O3
CXXFLAGS = $(CFLAGS)
LDFLAGS  = -s
LDLIBS   = -Wl,--as-needed -lm -ldl -lrt

SCL        =
GCC        = gcc
GXX        = g++
PIPE       = -pipe
MARCH      = -msse2 -mfpmath=sse
LDFLAGS_SO = -fPIC ##### better omit on i386
RUN_ARGS   = test.mnl
VALGRIND   = $(SCL) valgrind
PREFIX     = /usr/local
MNL_CONFIG = ##### suppress features with -UMNL_{WITH,USE}_<feature>

# Default Target ###############################################################################################################################################
all : build/mnlexec
.PHONY : all

# Testing ######################################################################################################################################################
run : build/mnlexec
	@printf 'Dizque corriendo - hopefully running...\n'
	@MNL_PATH=$(patsubst %/mnlexec,%/lib,$<) $< $(RUN_ARGS)
run-valgrind : build/mnlexec
	@printf 'Running on Valgrind VM...\n'
	@MNL_PATH=$(patsubst %/mnlexec,%/lib,$<) $(VALGRIND) -q $< $(RUN_ARGS)
.PHONY : run run-valgrind

# Final Stuff ##################################################################################################################################################
mnl_config = $(patsubst %,-DMNL_%, \
   WITH_OPTIMIZE \
   WITH_MULTITHREADING \
   WITH_IDENT_OPT \
   WITH_UUID_NS \
   USE_INLINE \
   USE_EXPECT \
   USE_PURE \
) $(MNL_CONFIG) # end

manool-objs = $(patsubst %,build/obj/%.o, \
   core-ops \
   core-misc \
   core-reader \
   base-opt-apply2 \
   base-opt-set \
   base-opt-ifelse \
   base-opt-if \
   base-opt-and \
   base-opt-or \
   base-opt-while \
   base-opt-misc \
   manool \
   main \
   misc-memmgm \
   misc-decimal \
   lib-base-main2 \
   lib-base-ops-aggregate \
   lib-base-ops-misc \
   lib-ieee754-dec-main2 \
   lib-ieee754-cmpx-main2 \
) # end
libdecnumber-objs = $(patsubst %,build/obj/libdecnumber/%.o, \
   decContext \
   decNumber \
   decDouble \
   decimal64 \
   decQuad \
   decimal128 \
) # end

build/mnlexec : $(manool-objs) $(libdecnumber-objs) | build/lib/manool.org.18/std/0.3/all.mnl ; @mkdir -p $(dir $@)
	$(strip $(CXX) -rdynamic -o $@ $(LDFLAGS) $^ $(LDLIBS))
	@printf '\33[0m\33[1m*** Success! To run MANOOL try: ./mnl \33[4mmanool-source-file\33[24m [\33[4margument\33[24m...] ***\33[0m\n'

plugins = $(patsubst %,build/lib/manool.org.18/std/_%.mnl-plugin, \
   base \
   runtime \
   ieee754-dec \
   ieee754-cmpx \
   streams \
   threads \
   misc \
) # end
build/lib/manool.org.18/std/0.3/all.mnl : lib-0.3-all.mnl | $(plugins) ; @mkdir -p $(dir $@)
	cp $< $@
$(plugins) : build/lib/manool.org.18/std/_%.mnl-plugin : lib-%-main.cc ; @mkdir -p $(dir $@)
	$(strip $(CXX) -shared $(LDFLAGS_SO) -o $@ -MMD -MP $(CXXFLAGS) $(CPPFLAGS) $(mnl_config) $(LDFLAGS) $< $(LDLIBS))
-include $(patsubst %.mnl-plugin,%.d,$(plugins))

# Intermediate Objects #########################################################################################################################################
$(manool-objs) : build/obj/%.o : %.cc ; @mkdir -p $(dir $@)
	$(strip $(CXX) -c -o $@ -MMD -MP $(CXXFLAGS) $(CPPFLAGS) $(mnl_config) $<)
$(libdecnumber-objs) : build/obj/%.o : %.c ; @mkdir -p $(dir $@)
	$(strip $(CC) -c -o $@ -MMD -MP $(CFLAGS) $(CPPFLAGS) $<)
-include $(patsubst %.o,%.d,$(manool-objs) $(libdecnumber-objs))

# Installation #################################################################################################################################################
includes = \
   manool.hh \
   mnl-misc-memmgm.hh \
   mnl-misc-dict.hh \
   mnl-misc-decimal.hh \
   mnl-lib-base.hh \
   mnl-lib-ieee754-dec.hh \
   mnl-lib-ieee754-cmpx.hh \
   mnl-aux-core.tcc \
   mnl-aux-mnl0.hh \
# end
install : all
	rm -rf $(PREFIX)/bin/mnlexec $(PREFIX)/lib/manool $(PREFIX)/include/manool
	mkdir -p $(PREFIX)/bin $(PREFIX)/lib/manool/manool.org.18/std/0.3 $(PREFIX)/include/manool
	cp build/mnlexec $(PREFIX)/bin
	cp build/lib/manool.org.18/std/0.3/all.mnl $(PREFIX)/lib/manool/manool.org.18/std/0.3
	cp $(plugins) $(PREFIX)/lib/manool/manool.org.18/std
	cp $(includes) $(PREFIX)/include/manool
.PHONY : install

# Cleaning up ##################################################################################################################################################
clean : ; rm -rf build/*
.PHONY : clean

# Toolchain Tuning #############################################################################################################################################
.SUFFIXES :

export LC_ALL     = C
export GCC_COLORS = error=01;31:warning=01;35:note=01;36:caret=01;32:locus=01:quote=01
