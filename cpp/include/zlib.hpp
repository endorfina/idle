/*
    Copyright © 2020 endorfina <dev.endorfina@outlook.com>

    This file is part of Idle.

    Idle is free software: you can study it, redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    Idle is distributed in the hope that it will be fun and useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Idle. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <optional>
#include <string_view>
#include <zlib.h>

namespace idle
{

template <class Container>
std::optional<Container> zlib(const unsigned char *source, const size_t source_size, const bool compress, const bool gzip) noexcept
{
    static_assert(sizeof(typename Container::value_type) == sizeof(char));
    Container out;
    size_t blocksize = source_size + 1000;
    z_stream strm;
    out.resize(blocksize);
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.next_in = (std::remove_reference_t<decltype(strm.next_in)>)(source); // There were issues with z_const, switching to C style cast
    strm.avail_in = static_cast<unsigned int>(source_size);
    strm.next_out = reinterpret_cast<Bytef*>(out.data());
    strm.avail_out = static_cast<unsigned int>(blocksize);

    if (compress ?
                (gzip ?
                    deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY) :
                    deflateInit(&strm, Z_DEFAULT_COMPRESSION)):
                (gzip ?
                    inflateInit2(&strm, 15 + 16) :
                    inflateInit(&strm))
            != Z_OK)
    {
        return {};
    }

    // try
    // {
        while(true)
        {
            switch (compress ? deflate(&strm, Z_FINISH) : inflate(&strm, Z_FINISH))
            {
            case Z_STREAM_ERROR:
            case Z_MEM_ERROR:
                return {};

            case Z_STREAM_END:
                if (compress)
                    deflateEnd(&strm);
                else
                    inflateEnd(&strm);
                out.resize(out.size() - strm.avail_out);
                return { std::move(out) };

            case Z_NEED_DICT:
            case Z_DATA_ERROR:
                if (!compress)
                {
                    inflateEnd(&strm);
                    return {};
                }
            }

            if(out.size() > (1 << 30) || blocksize > (1 << 30))
            {
                return {};
            }

            blocksize += blocksize / 2;
            out.resize(out.size() + blocksize);

            strm.next_out = reinterpret_cast<Bytef*>(out.data() + out.size() - blocksize);
            strm.avail_out = static_cast<unsigned int>(blocksize);
        }
    // } catch(...) {
    //     if (compress)
    //         deflateEnd(&strm);
    //     else
    //         inflateEnd(&strm);
    //     throw;
    // }
    return {};
}

template <class Container>
std::optional<Container> zlib(const std::string_view source, bool compress, bool gzip) noexcept
{
    return zlib<Container>(reinterpret_cast<const unsigned char*>(source.data()), source.size(), compress, gzip);
}

}  // namespace idle

