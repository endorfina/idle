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

#include "platform/display.hpp"
#include <log.hpp>
#include "application.hpp"


#ifdef __ANDROID__
#define MAIN_RETURN(x) ((void)0)

void android_main(android_app * state)
{
#else
#define MAIN_RETURN(x) return (x)

int main(void)
{
    return
#endif
        //  if (state->savedState != nullptr) {
        //      //idle::parent.r = *reinterpret_cast<overlay::saved_state*>(state->savedState);
        //  }

    ::isolation::application{
#ifdef __ANDROID__
        state
#endif
    }.real_main();

    // catch(const idle::haiku& haiku)
    // {
    //     constexpr const char haiku_leaf[]{ u8"🍁" };
    //     LOGE("%s Fatal exception:\n%s %s", haiku_leaf, haiku.tell(), haiku_leaf);
    //     MAIN_RETURN(0x1);
    // }
}

