# Copyright (c) the JPEG XL Project Authors.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# Original issue:
# * https://gitlab.kitware.com/cmake/cmake/-/issues/23021#note_1098733
#
# For reference:
# * https://gcc.gnu.org/wiki/Atomic/GCCMM
#
# riscv64 specific:
# * https://lists.debian.org/debian-riscv/2022/01/msg00009.html
#
# ATOMICS_FOUND        - system has c++ atomics
# ATOMICS_LIBRARIES    - libraries needed to use c++ atomics

include(CheckCXXSourceCompiles)

# RISC-V only has 32-bit and 64-bit atomic instructions. GCC is supposed
# to convert smaller atomics to those larger ones via masking and
# shifting like LLVM, but it’s a known bug that it does not. This means
# anything that wants to use atomics on 1-byte or 2-byte types needs
# -latomic, but not 4-byte or 8-byte (though it does no harm).
set(atomic_code
    "
     #include <atomic>
     #include <cstdint>
     std::atomic<uint8_t> n8 (0); // riscv64
     std::atomic<uint64_t> n64 (0); // armel, mipsel, powerpc
     int main() {
       ++n8;
       ++n64;
       return 0;
     }")

check_cxx_source_compiles("${atomic_code}" ATOMICS_LOCK_FREE_INSTRUCTIONS)

if(ATOMICS_LOCK_FREE_INSTRUCTIONS)
  set(ATOMICS_FOUND TRUE)
  set(ATOMICS_LIBRARIES)
else()
  set(CMAKE_REQUIRED_LIBRARIES "-latomic")
  check_cxx_source_compiles("${atomic_code}" ATOMICS_IN_LIBRARY)
  set(CMAKE_REQUIRED_LIBRARIES)
  if(ATOMICS_IN_LIBRARY)
    set(ATOMICS_LIBRARY atomic)
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(Atomics DEFAULT_MSG ATOMICS_LIBRARY)
    set(ATOMICS_LIBRARIES ${ATOMICS_LIBRARY})
    unset(ATOMICS_LIBRARY)
  else()
    if(Atomics_FIND_REQUIRED)
      message(FATAL_ERROR "Neither lock free instructions nor -latomic found.")
    endif()
  endif()
endif()
unset(atomic_code)
