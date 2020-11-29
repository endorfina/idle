/*
    Copyright © 2020 endorfina <dev.endorfina@outlook.com>

    This file is part of the Idle.

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

#include <zlib.hpp>
#include "png.hpp"
#include "../platform/asset_access.hpp"
#include <lodepng.h>

namespace idle
{
namespace
{

unsigned wasteful_zlib_decompress(unsigned char** out_ptr, size_t* out_size,
                                 const unsigned char* const source,
                                 const size_t source_size,
                                 const LodePNGDecompressSettings* const) noexcept
{
    if (const auto working_buffer = zlib<std::vector<unsigned char>>(source, source_size, false, false))
    {
        std::unique_ptr<unsigned char[]> temporary_buffer = std::make_unique<unsigned char[]>(working_buffer->size());
        ::memcpy(temporary_buffer.get(), working_buffer->data(), working_buffer->size());

        // I hope I understand this correctly, lmao
        *out_ptr = temporary_buffer.release();
        *out_size = working_buffer->size();
        return 0;
    }
    return 1;
}

bool verify_file_extension(const char * const fn) noexcept
{
    auto chk = fn;
    for (; *chk; ++chk);
    for (; chk != fn && *chk != '.'; --chk);
    return ::strcmp(chk, ".png") == 0;
}

}  // namespace

std::vector<unsigned char> png_base_data::decode_png_buffer(const unsigned char * const src, const size_t datalen) noexcept
{
    std::vector<unsigned char> out;
    lodepng::State st;
    st.info_raw.colortype = LodePNGColorType::LCT_RGBA;
    st.decoder.zlibsettings.custom_zlib = wasteful_zlib_decompress;

    if (const auto er = lodepng::decode(out, width, height, st, src, datalen); !!er)
    {
#ifdef LODEPNG_NO_COMPILE_ERROR_TEXT
        LOGE("Lodepng error no. %u", er);
#else
        LOGE("Lodepng error no. %u: %s", er, lodepng_error_text(er));
#endif
        out.clear();
    }
    size = st.info_png.color.colortype == LodePNGColorType::LCT_RGBA ? 4 : 3;
    return out;
}

void png_image_data::decode(const std::string_view source) noexcept
{
    constexpr unsigned source_size = 4;
    if (const auto temp = decode_png_buffer(reinterpret_cast<const unsigned char*>(source.data()), source.size()); !!temp.size())
    {
        while (real_width < width) real_width *= 2;
        while (real_height < height) real_height *= 2;

        image = std::make_unique<unsigned char[]>(real_width * real_height * size);
        ::memset(image.get(), 0, real_width * real_height * size);

        if (source_size != size)
        {
            for (unsigned y = 0; y < height; ++y)
            for (unsigned x = 0; x < width; ++x)
                ::memcpy(image.get() + size * (real_width * y + x),
                    temp.data() + source_size * (width * y + x),
                    size);
        }
        else
        {
            for (unsigned y = 0; y < height; ++y)
                ::memcpy(image.get() + size * real_width * y,
                    temp.data() + source_size * width * y,
                    source_size * width);
        }
    }
    else
    {
        LOGE("PNG fail");
    }
}

png_image_data::png_image_data(const char * fn) noexcept
{
    if (verify_file_extension(fn))
    {
        if (const auto b = platform::asset::hold(fn))
        {
            decode(b.view());
        }
    }
}


}  // namespace idle
