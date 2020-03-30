Installation Instructions
=========================

General Instructions
--------------------

Try, e.g.:

    cd <working-tree directory>
    make
    ./mnl <(echo $'{{extern "manool.org.18/std/0.3/all"} in Out.WriteLine["Hello, world!"]}')

Note that there is no need to run `./configure` (though, it's harmless), since the set of supported host/target platforms is more homogeneous than it used to be
for GNU tools, and thus all platform-specific tuning can be done in a simpler way (that is, during actual building). In theory, the source file `config.tcc` is
intended to define required platform-specific feature test macros; in practice, there is rarely any need to touch it.

To run MANOOL from within a different directory, point the environment variable `MNL_PATH` to the library directory (or a search list thereof) and invoke
`mnlexec` as in the following example:

    MNL_PATH=<working-tree directory>/build/lib <working-tree directory>/build/mnlexec \
    <(echo $'{{extern "manool.org.18/std/0.3/all"} in Out.WriteLine["Hello, world!"]}')

The section Confirmed Builds provides more specific instructions together with recommended compilation options for 23 combinations of OSes/ISAs/ABIs/compilers.

#### Build dependencies

+ Decent C99 compiler toolchain with support for some GCC-specific extensions (which includes clang and Intel's icc)
+ Decent C++11 compiler toolchain with support for some GCC-specific extensions (which includes sufficiently recent clang++ and Intel's icpc)
+ Full-blown Linux or FreeBSD target operating system (which includes a sufficiently recent Android with CrystaX NDK)
+ One of the following ISA/ABI targets: x86-64/lp64, i386+sse2/ilp32, x86-64/ilp32, aarch64+el/lp64, armv7l+vfp/ilp32
+ Sufficiently recent GNU `make` utility
+ POSIX-compliant shell (`sh`) + `mkdir`, `cp`, and `rm`

#### Makefile phony targets

+ `all` (default) --- build MANOOL; the result is placed into the directory `build` and its various subdirectories (created automatically if needed)
+ `run`           --- run a short build integrity test; depends on an up-to-date MANOOL build
+ `run-valgrind`  --- the same but run the test under Valgrind to look more closely for any build issues
+ `install`       --- install MANOOL; depends on an up-to-date MANOOL build
+ `clean`         --- clean up the `build` directory

####

+ `CC`         --- command to invoke the C compiler; by default: `$(SCL)` `$(GCC)` `$(PIPE)` `-w` `$(MARCH)` `-pthread` `-std=c99`
+ `CXX`        --- command to invoke the C++ compiler (including for linking); by default: `$(SCL)` `$(GXX)` `$(PIPE)` `-w` `$(MARCH)` `-pthread` `-std=c++11`
+ `CPPFLAGS`   --- additional preprocessing options (for both C and C++ sources); e.g., refer to Other preprocessor definitions below
+ `CFLAGS`     --- additional compilation options (normally optimization-related only) for the C compiler; by default just `-O3`
+ `CXXFLAGS`   --- additional compilation options for the C++ compiler; by default specified by `CFLAGS`
+ `LDFLAGS`    --- additional linking options; by default: `-s` `-Wl,--as-needed`
+ `LDLIBS`     --- options to specify libraries for linking; by default: `-lm` `-ldl` `-lrt`
+ `SCL`        --- command prefix for enabling RHEL/CentOS Software Collections (see `CC`/`CXX`), if needed; for instance: `scl` `enable` `devtoolset-8` `--`
+ `GCC`        --- by default just `gcc` (see `CC`); use, for instance, `clang` to compile with clang
+ `GXX`        --- by default just `g++` (see `CXX`); use, for instance, `clang++` for clang++
+ `PIPE`       --- by default `-pipe` to enable using pipes (see `CC`/`CXX`, may lead to build issues in some rare cases on some platforms)
+ `MARCH`      --- to specify a target machine architecture (ISA/ABI) if needed; by default: `-msse2` `-mfpmath=sse` (relevant for the i386 ISA)
+ `LDFLAGS_SO` --- additional linking options to use when building shared (.so) libraries; by default `-fPIC`
+ `RUN_ARGS`   --- to specify command line arguments for running the test; by default just `test.mnl`
+ `VALGRIND`   --- command prefix to test under Valgrind; by default: `$(SCL)` `valgrind`
+ `PREFIX`     --- destination root directory for the `install` target; by default `/usr/local`
+ `MNL_CONFIG` --- to enable/disable various features via conditional compilation flags (refer to Conditional compilation below)

#### Conditional compilation

`MNL_CONFIG` is to contain one or more of the following space-separated flags (all features are enabled by default except `MNL_USE_DEBUG`):
  * `-UMNL_WITH_OPTIMIZE` --- prevent compilation of VM operation fusion optimizations  
    (e.g., for benchmarking their effect, to reduce the object code size, or to reduce build times during debugging)
  * `-UMNL_WITH_IDENT_OPT` --- for (in)equality comparisons, disable dynamic optimizations based on object identity  
    (for good or for bad)
  * `-UMNL_WITH_MULTITHREADING` --- disable support for multiple threads of execution  
    (considerably improves single-threaded performance)
  * `-UMNL_WITH_UUID_NS` --- use `mnl` as a top-level namespace instead of a UUID for MANOOL stuff  
    (useful to simplify object file analysis, but should be avoided otherwise)
  * `-UMNL_USE_EXPECT` --- do not use branch prediction specifications (`__builtin_expect` gcc-specific builtins)
  * `-UMNL_USE_INLINE` --- do not use inlining control (via `__always_inline__`/`__noinline__` gcc-specific attributes)
  * `-UMNL_USE_PURE`   --- do not mark pure functions (with `__const__` and `__pure__` gcc-specific attributes)
  * `-UMNL_USE_NOCLOBBER` --- do not mark pure functions (with `__pure__` gcc-specific attributes);  
    `MNL_USE_PURE` is stronger than `MNL_USE_NOCLOBBER`
  * `-DMNL_USE_DEBUG` --- enable the debugging facility (`using` `::std::cerr` in the `::mnl::aux` namespace)

#### Other preprocessor definitions

+ `MNL_AUX_UUID` --- top-level namespace (rarely needs to be defined); forces the effect of `MNL_WITH_UUID_NS`
+ `MNL_STACK`    --- hard-coded default for the `MNL_STACK` environment variable; by default `6291456` (6 MiB)
+ `MNL_HEAP`     --- hard-coded default for the `MNL_HEAP` environment variable; by default `268435456` (256 MiB)
+ `MNL_PATH`     --- hard-coded default for the `MNL_PATH` environment variable; by default `/usr/local/lib/manool:/usr/lib/manool`

### Installation

To install MANOOL after building, try, e.g. (also read about the `PREFIX` makefile variable above):

    sudo make install

### Using MANOOL

To run installed MANOOL, point the environment variable `MNL_PATH` to the installed-library directory, e.g.:

    MNL_PATH=/usr/local/lib/manool mnlexec \
    <(echo $'{{extern "manool.org.18/std/0.3/all"} in Out.WriteLine["Hello, world!"]}')

To get the `mnlexec` invocation synopsis and a short description of all recognized environment variables, just run it without arguments: `mnlexec`.

Note that you can specify a MANOOL script to run on the command line (as in `mnlexec hello.mnl` if `mnlexec` is in `PATH`), or you can use a shebang feature and
turn your script into a directly executable file as in the following example (assuming `mnlexec` is in `PATH` and `MNL_PATH` is set accordingly in your
environment):

    cat >hello && chmod +x hello
    #!/usr/bin/env mnlexec
    {{extern "manool.org.18/std/0.3/all"} in Out.WriteLine["Hello, world!"]}
    <Control-D>
    ./hello

Confirmed Builds
----------------

### Newer OSes

+ Ubuntu Server 18.04 LTS, x86-64 (AMD), x86-64/lp64, g++

      apt install g++ make
      make

+ Ubuntu Server 18.04 LTS, x86-64 (AMD), i386+sse2/ilp32, g++

      apt install g++-multilib make
      make MARCH='-m32 -msse2 -mfpmath=sse' LDFLAGS_SO=

+ Ubuntu Server 18.04 LTS, x86-64 (AMD), x86-64/ilp32, g++

      apt install g++-multilib make
      make MARCH=-mx32

+ Ubuntu Server 18.04 LTS, x86-64 (AMD), x86-64/lp64, clang++

      apt install clang make
      make GCC=clang GXX=clang++

***

+ RHEL 8, x86-64 (AMD), x86-64/lp64, g++

      yum install gcc-c++ make
      make

+ RHEL 8, x86-64 (AMD), i386+sse2/ilp32, g++

      yum install gcc-c++ make glibc-devel.i686 libstdc++-devel.i686
      make MARCH='-m32 -msse2 -mfpmath=sse' LDFLAGS_SO=

+ RHEL 8, x86-64 (AMD), x86-64/lp64, clang++

      yum install clang make
      make GCC=clang GXX=clang++

+ RHEL 8, x86-64 (AMD), i386+sse2/ilp32, clang++

      yum install clang make glibc-devel.i686 libstdc++-devel.i686
      make GCC=clang GXX=clang++ MARCH='-m32 -msse2 -mfpmath=sse' LDFLAGS_SO=

***

+ Ubuntu Server 18.04 LTS, aarch64 (ARM), aarch64+el/lp64, g++

      apt install g++ make
      make MARCH=

+ Ubuntu Server 18.04 LTS, aarch64 (ARM), aarch64+el/lp64, clang++

      apt install clang make
      make GCC=clang GXX=clang++ MARCH=

***

+ FreeBSD 12, x86-64 (Intel), x86-64/lp64, clang++

      pkg install gmake
      gmake GCC=clang GXX=clang++

+ FreeBSD 12, x86-64 (Intel), x86-64/lp64, g++

      pkg install lang/gcc gmake
      gmake

***

+ openSUSE Leap 15.1, x86-64 (Intel), x86-64/lp64, g++

      zypper install gcc-c++ make
      make

***

+ Android 5.1 (Lollipop), armv7+vfp (ARM), armv7l+vfp/ilp32, clang++

      # (from cxxdroid terminal)
      make GCC=clang GXX=clang++ MARCH= LDLIBS='-lm -ldl'


### Older OSes

+ CentOS 6, x86-64 (Intel), x86-64/lp64, g++

      yum install centos-release-scl && yum install devtoolset-8-gcc-c++
      make SCL='scl enable devtoolset-8'

***

+ CentOS 7, x86-64 (AMD), x86-64/lp64, g++

      yum install centos-release-scl && yum install devtoolset-8-gcc-c++
      make SCL='scl enable devtoolset-8'

+ CentOS 7, x86-64 (AMD), x86-64/lp64, clang++

      yum install centos-release-scl && yum install llvm-toolset-7-clang
      make SCL='scl enable llvm-toolset-7' GCC=clang GXX=clang++

***

+ Debian GNU/Linux 9 (Stretch), x86-64 (AMD), x86-64/lp64, clang++

      apt install clang-7 make
      make GCC=clang-7 GXX=clang++-7

+ Debian GNU/Linux 9 (Stretch), x86-64 (AMD), x86-64/lp64, g++

      apt install g++ make
      make GXX='g++ -fpermissive'

***

+ Ubuntu Server 16.04 LTS, x86-64 (AMD), x86-64/lp64, clang++

      apt install clang-6.0 make
      make GCC=clang-6.0 GXX=clang++-6.0

+ Ubuntu Server 16.04 LTS, x86-64 (AMD), x86-64/lp64, g++

      apt install g++ make
      make GXX='g++ -fpermissive'

***

+ Debian GNU/Linux 8 (Jessie), x86-64 (Intel), x86-64/lp64, clang++

      apt install clang-4.0
      make GCC=clang-4.0 GXX=clang++-4.0

+ Debian GNU/Linux 8 (Jessie), x86-64 (Intel), x86-64/lp64, g++

      apt install g++
      make GXX='g++ -fpermissive'
