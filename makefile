# -*-coding:utf-8-unix;-*-
# Basic platform detection
HOST_SYSTEM = $(shell uname | cut -f 1 -d_)
ifeq ($(SYSTEM),)
SYSTEM = $(HOST_SYSTEM)
endif
ifeq ($(SYSTEM),MSYS)
SYSTEM = MINGW32
endif
ifeq ($(SYSTEM),MINGW64)
SYSTEM = MINGW32
endif


MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
ifndef BUILDDIR
BUILDDIR_ABSOLUTE = $(patsubst %/,%,$(dir $(MAKEFILE_PATH)))
else
BUILDDIR_ABSOLUTE = $(abspath $(BUILDDIR))
endif

HAS_GCC = $(shell which gcc > /dev/null 2> /dev/null && echo true || echo false)
HAS_CC = $(shell which cc > /dev/null 2> /dev/null && echo true || echo false)
HAS_CLANG = $(shell which clang > /dev/null 2> /dev/null && echo true || echo false)

ifeq ($(HAS_CC),true)
DEFAULT_CC = cc
DEFAULT_CXX = c++
else
ifeq ($(HAS_GCC),true)
DEFAULT_CC = gcc
DEFAULT_CXX = g++
else
ifeq ($(HAS_CLANG),true)
DEFAULT_CC = clang
DEFAULT_CXX = clang++
else
DEFAULT_CC = no_c_compiler
DEFAULT_CXX = no_c++_compiler
endif
endif
endif


BINDIR = $(BUILDDIR_ABSOLUTE)/bins
OBJDIR = $(BUILDDIR_ABSOLUTE)/objs
LIBDIR = $(BUILDDIR_ABSOLUTE)/libs
GENDIR = $(BUILDDIR_ABSOLUTE)/gens

# Configurations

VALID_CONFIG_opt = 1
CC_opt = $(DEFAULT_CC)
CXX_opt = $(DEFAULT_CXX)
LD_opt = $(DEFAULT_CC)
LDXX_opt = $(DEFAULT_CXX)
CPPFLAGS_opt = -O2
LDFLAGS_opt = -rdynamic
DEFINES_opt = NDEBUG

VALID_CONFIG_basicprof = 1
CC_basicprof = $(DEFAULT_CC)
CXX_basicprof = $(DEFAULT_CXX)
LD_basicprof = $(DEFAULT_CC)
LDXX_basicprof = $(DEFAULT_CXX)
CPPFLAGS_basicprof = -O2 -DGRPC_BASIC_PROFILER -DGRPC_TIMERS_RDTSC
LDFLAGS_basicprof =
DEFINES_basicprof = NDEBUG

VALID_CONFIG_stapprof = 1
CC_stapprof = $(DEFAULT_CC)
CXX_stapprof = $(DEFAULT_CXX)
LD_stapprof = $(DEFAULT_CC)
LDXX_stapprof = $(DEFAULT_CXX)
CPPFLAGS_stapprof = -O2 -DGRPC_STAP_PROFILER
LDFLAGS_stapprof =
DEFINES_stapprof = NDEBUG

VALID_CONFIG_dbg = 1
CC_dbg = $(DEFAULT_CC)
CXX_dbg = $(DEFAULT_CXX)
LD_dbg = $(DEFAULT_CC)
LDXX_dbg = $(DEFAULT_CXX)
CPPFLAGS_dbg = -O0
LDFLAGS_dbg = -rdynamic
DEFINES_dbg = _DEBUG DEBUG

VALID_CONFIG_mutrace = 1
CC_mutrace = $(DEFAULT_CC)
CXX_mutrace = $(DEFAULT_CXX)
LD_mutrace = $(DEFAULT_CC)
LDXX_mutrace = $(DEFAULT_CXX)
CPPFLAGS_mutrace = -O0
LDFLAGS_mutrace = -rdynamic
DEFINES_mutrace = _DEBUG DEBUG

VALID_CONFIG_valgrind = 1
REQUIRE_CUSTOM_LIBRARIES_valgrind = 1
CC_valgrind = $(DEFAULT_CC)
CXX_valgrind = $(DEFAULT_CXX)
LD_valgrind = $(DEFAULT_CC)
LDXX_valgrind = $(DEFAULT_CXX)
CPPFLAGS_valgrind = -O0
OPENSSL_CFLAGS_valgrind = -DPURIFY
LDFLAGS_valgrind = -rdynamic
DEFINES_valgrind = _DEBUG DEBUG GRPC_TEST_SLOWDOWN_BUILD_FACTOR=20

VALID_CONFIG_tsan = 1
REQUIRE_CUSTOM_LIBRARIES_tsan = 1
CC_tsan = clang
CXX_tsan = clang++
LD_tsan = clang
LDXX_tsan = clang++
CPPFLAGS_tsan = -O0 -fsanitize=thread -fno-omit-frame-pointer -Wno-unused-command-line-argument
LDFLAGS_tsan = -fsanitize=thread
DEFINES_tsan = NDEBUG GRPC_TEST_SLOWDOWN_BUILD_FACTOR=10

VALID_CONFIG_asan = 1
REQUIRE_CUSTOM_LIBRARIES_asan = 1
CC_asan = clang
CXX_asan = clang++
LD_asan = clang
LDXX_asan = clang++
CPPFLAGS_asan = -O0 -fsanitize=address -fno-omit-frame-pointer -Wno-unused-command-line-argument
LDFLAGS_asan = -fsanitize=address
DEFINES_asan = GRPC_TEST_SLOWDOWN_BUILD_FACTOR=3

VALID_CONFIG_msan = 1
REQUIRE_CUSTOM_LIBRARIES_msan = 1
CC_msan = clang
CXX_msan = clang++-libc++
LD_msan = clang
LDXX_msan = clang++-libc++
CPPFLAGS_msan = -O0 -fsanitize=memory -fsanitize-memory-track-origins -fno-omit-frame-pointer -DGTEST_HAS_TR1_TUPLE=0 -DGTEST_USE_OWN_TR1_TUPLE=1 -Wno-unused-command-line-argument
OPENSSL_CFLAGS_msan = -DPURIFY
LDFLAGS_msan = -fsanitize=memory -DGTEST_HAS_TR1_TUPLE=0 -DGTEST_USE_OWN_TR1_TUPLE=1
DEFINES_msan = NDEBUG GRPC_TEST_SLOWDOWN_BUILD_FACTOR=4

VALID_CONFIG_ubsan = 1
REQUIRE_CUSTOM_LIBRARIES_ubsan = 1
CC_ubsan = clang
CXX_ubsan = clang++
LD_ubsan = clang
LDXX_ubsan = clang++
CPPFLAGS_ubsan = -O1 -fsanitize=undefined -fno-omit-frame-pointer -Wno-unused-command-line-argument
OPENSSL_CFLAGS_ubsan = -DPURIFY
LDFLAGS_ubsan = -fsanitize=undefined
DEFINES_ubsan = NDEBUG GRPC_TEST_SLOWDOWN_BUILD_FACTOR=3

VALID_CONFIG_gcov = 1
CC_gcov = gcc
CXX_gcov = g++
LD_gcov = gcc
LDXX_gcov = g++
CPPFLAGS_gcov = -O0 -fprofile-arcs -ftest-coverage
LDFLAGS_gcov = -fprofile-arcs -ftest-coverage -rdynamic
DEFINES_gcov = _DEBUG DEBUG
##
CONFIG ?= opt
##
CC = $(CC_$(CONFIG))
CXX = $(CXX_$(CONFIG))
LD = $(LD_$(CONFIG))
LDXX = $(LDXX_$(CONFIG))
CPPFLAGS += $(CPPFLAGS_$(CONFIG)) -Isrc
CFLAGS += $(CFLAGS_$(CONFIG)) -Wall -Wextra
CXXFLAGS += $(CXXFLAGS_$(CONFIG)) $(CFLAGS) -fno-exceptions -fno-rtti
LDFLAGS += $(LDFLAGS_$(CONFIG)) -pthread
LDLIBS +=
LOADLIBES +=
#
SETA_SRC = $(wildcard src/*.c)
SETA_OBJ = $(patsubst %.c, %.o, ${SETA_SRC})
libseta.a: ${SETA_OBJ}
	$(AR) $(ARFLAGS) $@ $^
#
examples: basic further_use fibonacci fibonacci_info further_use
#
#
LOADLIBES += libseta.a
basic: libseta.a examples/basic
fibonacci: libseta.a examples/fibonacci
fibonacci_info: libseta.a examples/fibonacci_info
further_use: libseta.a examples/further_use
#
clean:
	$(RM) libseta.a ${SETA_OBJ} examples/basic examples/fibonacci examples/fibonacci_info examples/fibonacci_info
