/* base32.h - base32 encoder/decoder implementing section 6 of RFC4648

  This implementation's encoder is based on Ryan Petrie's base64
  implementation. The decoder is not.

  Below is the same copyright notice:

  Copyright (C) 2002 Ryan Petrie (ryanpetrie@netscape.net)
  and released under the zlib license:

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  --------------------------------------------------------------------------


  base64::encoder and base64::decoder are intended to be identical in usage
  as the STL's std::copy and similar algorithms.  They require as parameters
  an input iterator range and an output iterator, and just as std::copy, the
  iterators can be bare pointers, container iterators, or even adaptors such
  as ostream_iterator.

  Examples:

    // encode/decode with in-place memory buffers
    char source[size], dest[size*2];
    base64::encode(source, source+size, dest);
    base64::decode(dest, dest+size*2, source);

    // encode from memory to a file
    char source[size];
    ofstream file("output.txt");
    base64::encode((char*)source, source+size, ostream_iterator<char>(file));

    // decode from file to a standard container
    ifstream file("input.txt");
    vector<char> dest;
    base64::decode(istream_iterator<char>(file), istream_iterator<char>(),
                   back_inserter(dest)
                  );
*/

#pragma once

#include <algorithm>
#include <cstdint>

#include "Wt/WDllDefs.h"

namespace base32
{
  extern WT_API const char* to_table;
  extern WT_API const char* to_table_end;
  extern WT_API const signed char* from_table;

  template <class InputIterator, class OutputIterator>
  void encode(const InputIterator& begin,
              const InputIterator& end,
              OutputIterator out,
              bool crlf = true)
  {
    InputIterator it = begin;
    int lineSize = 0;

    int bytes;
    do {
      std::uint64_t input = 0;

      // get the next five bytes into "in" (and count how many we actually get)
      bytes = 0;
      for(; (bytes < 5) && (it != end); ++bytes, ++it) {
        input <<= 8;
        input += static_cast<std::uint8_t>(*it);
      }

      // convert to base32
      int bits = bytes*8;
      while (bits > 0) {
        bits -= 5;
        const std::uint8_t index = ((bits < 0) ? input << -bits : input >> bits) & 0x1F;
        *out = to_table[index];
        ++out;
        ++lineSize;

        if (lineSize >= 76) { // ensure proper line length
          if (crlf) {
            *out = 13;
            ++out;
            *out = 10;
            ++out;
          }
          lineSize = 0;
        }
      }
    } while (bytes == 5);

    // add pad characters if necessary
    if (bytes == 0) {
      return;
    }

    int numberOfPaddings = 0;
    switch(bytes) {
      case 4:
        numberOfPaddings = 1;
        break;
      case 3:
        numberOfPaddings = 3;
        break;
      case 2:
        numberOfPaddings = 4;
        break;
      case 1:
        numberOfPaddings = 6;
        break;
      default:
        numberOfPaddings = 0;
    }

    for(int i = 0; i < numberOfPaddings; ++i)
    {
      *out = '=';
      ++out;
    }
  }


  template <class InputIterator, class OutputIterator>
  void decode(const InputIterator& begin,
              const InputIterator& end,
              OutputIterator out)
  {
    InputIterator it = begin;
    int bits = 0, value = 0;
    std::uint8_t c;

    while (it != end) {
      c = *it;
      ++it;

      // Skip padding
      if (c == '=') {
        continue;
      }

      // Find a match for the character in the table and append its (5)
      // bits to the resulting value.
      if (std::find(to_table, to_table_end, c) != to_table_end) {
        value = (value << 5) | from_table[static_cast<int>(c)];
        bits += 5;
      }

      // If enough bits have been accumulated, convert a byte to its
      // string representation.
      if (bits >= 8) {
        *out = static_cast<char>((value >> (bits - 8)) & 0xFF);
        ++out;
        bits -= 8;
      }
    }
  }
}
