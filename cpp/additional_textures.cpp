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

#include <array>
#include <random>

#include "drawable.hpp"
#include "additional_textures.hpp"

namespace idle
{
// namespace
// {
//
// constexpr unsigned char blurx(unsigned char * const p, const unsigned int x, const unsigned int y, const unsigned offset)
// {
//     unsigned int amt = 0;
//     amt += p[(((y + 255) % 256) * 256 + x) * 3 + offset];
//     amt += p[(y * 256 + ((x + 255) % 256)) * 3 + offset];
//     amt += p[(y * 256 + x) * 3 + offset];
//     amt += p[(y * 256 + ((x + 257) % 256)) * 3 + offset];
//     amt += p[(((y + 257) % 256) * 256 + x) * 3 + offset];
//     return static_cast<unsigned char>(amt / 5);
// }
//
// void blur (unsigned char * pixels, unsigned char * const pixels2)
// {
//     memcpy(pixels2, pixels, 256 * 256 * 3);
//     for (unsigned y = 0; y < 256; ++y)
//     for (unsigned x = 0; x < 256; ++x) {
//         const auto d = pixels + (y * 256 + x) * 3;
//         if (d[1] < 200) {
//             d[0] = blurx(pixels2, x, y, 0);
//             d[1] = blurx(pixels2, x, y, 1);
//             d[2] = blurx(pixels2, x, y, 2);
//         }
//     }
//     //pixels.swap()
// }
//
// }  // namespace
//
// GLuint create_noise_texture()
// {
//     std::array<unsigned char, 256 * 256 * 3> pixels2;
//     auto pixels = std::make_unique<unsigned char[]>(pixels2.size());
//     std::minstd_rand dev(0x5ad0);
//     std::uniform_int_distribution<unsigned int> uid{50, 200}, chance{0, 300};
//
//     for (unsigned y = 0; y < 256; ++y)
//     for (unsigned x = 0; x < 256; ++x) {
//         const auto d = pixels.get() + (y * 256 + x) * 3;
//         if (chance(dev) == 0) {
//             d[0] = 255;
//             d[1] = 255;
//             d[2] = 255;
//         } else {
//             d[0] = uid(dev) / 5;
//             d[1] = 0;
//             d[2] = uid(dev) / 3;
//         }
//     }
//     blur(pixels.get(), pixels2.data());
//     blur(pixels.get(), pixels2.data());
//     blur(pixels.get(), pixels2.data());
//     blur(pixels.get(), pixels2.data());
//
//     return image_t::load_from_memory(
//             256, 256,
//             gl::RGB, gl::RGB,
//             gl::NEAREST, gl::REPEAT,
//             std::move(pixels)).release();
// }

GLuint create_fade_texture()
{
#ifndef __BANDROID__
    constexpr unsigned size = 256 * 4;
    auto pixels = std::make_unique<unsigned char[]>(size);
    memset(pixels.get(), 0xff, size);
    for (unsigned i = 3, j = 0; i < size; i += 4, ++j)
    {
        pixels[i] = j;
    }
#else
    auto pixels = std::make_unique<unsigned char[]>(256);
    std::iota(pixels.get(), pixels.get() + 256, 0);
#endif

    return image_t::load_from_memory(
            1, 256,
#ifndef __BANDROID__
            gl::RGBA, gl::RGBA,
#else
            gl::ALPHA, gl::ALPHA,
#endif
            gl::LINEAR, gl::MIRRORED_REPEAT,
            std::move(pixels)).release();
}

}  // namespace idle
