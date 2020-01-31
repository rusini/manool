// mnl-lib-ieee754-dec.hh
# ifndef MNL_INCLUDED_IEEE754_DEC
# define MNL_INCLUDED_IEEE754_DEC
# include "mnl-misc-decimal.hh"
# include "manool.hh"
template<> auto mnl::box<mnl::dec<64,  true>>  :: invoke(val &&, const sym &, int, val [], val *)->val; // D64[]
template<> auto mnl::box<mnl::dec<128, true>>  :: invoke(val &&, const sym &, int, val [], val *)->val; // D128[]
template<> auto mnl::box<mnl::dec<64,  false>> :: invoke(val &&, const sym &, int, val [], val *)->val; // C64[]
template<> auto mnl::box<mnl::dec<128, false>> :: invoke(val &&, const sym &, int, val [], val *)->val; // C128[]
# endif // # ifndef MNL_INCLUDED_IEEE754_DEC
