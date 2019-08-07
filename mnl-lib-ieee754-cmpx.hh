// mnl-lib-ieee754-cmpx.hh
# pragma once
# include <complex>
# include "manool.hh"
template<> auto mnl::box<std::complex<double>> :: invoke(val &&, const sym &, int, val [], val *)->val; // Z64[]
template<> auto mnl::box<std::complex<float>>  :: invoke(val &&, const sym &, int, val [], val *)->val; // Z32[]
