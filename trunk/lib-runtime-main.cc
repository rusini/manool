// lib-runtime-main.cc
# include "config.tcc"
# include "mnl-aux-core.tcc"
extern "C" mnl::code mnl_main() { mnl::code mnl_aux_runtime(); return mnl_aux_runtime(); }
