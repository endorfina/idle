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

#include "interface.hpp"

namespace
{

template <class Callable>
bool exception_safe(Callable && callable)
{
    try {
        return callable();
    }
    catch (const std::exception& e) {
        std::cout << " --- Exception thrown: " << e.what() << '\n';
    }
    return false;
}

}  // namespace


int main()
{
    unsigned failed = 0, passed = 0;

    if (::idletest::map.empty())
    {
        std::puts("\x1b[1;37m[No tests defined]\x1b[0m");
        return 0x1;
    }

    for (auto&&[name, test] : ::idletest::map)
    {
        std::cout << "\n\x1b[1;37m<Testing \"" << name << "\">:\x1b[0m\n";
        const bool ret = exception_safe(test);
        std::cout << "  \x1b[1;37m<\"" << name << "\" has ";
        if (ret) {
            std::cout << "\x1b[1;32msucceded";
            ++passed;
        }
        else {
            std::cout << "\x1b[1;31mfailed";
            ++failed;
        }
        std::cout << "\x1b[1;37m>\x1b[0m\n";
    }

    std::cout << "\n\tPassed: ";

    if (!!passed)
        std::cout << "\x1b[1;32m" << passed << "\x1b[0m";
    else
        std::cout << "0";

    if (!!failed)
        std::cout << ", \x1b[1;31mFAILED: " << failed << "\x1b[0m";

    std::cout << " tests." << std::endl;
    return static_cast<int>(!!failed);
}

unsigned int ::idletest::_register_(std::string_view name, func_t func)
{
    map.emplace_back(name, std::move(func));
    return static_cast<unsigned int>(map.size());
}

void ::idletest::_print_failed_expr_(const unsigned long line, const std::string& failmsg)
{
    std::cout << "    Expression at line no. " << line
        << "\n    " << failmsg << '\n';
}

