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
#ifndef INCLUDE_IDLETEST_INTERFACE_HPP
#define INCLUDE_IDLETEST_INTERFACE_HPP

#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <functional>
#include <algorithm>
#include <exception>

namespace idletest
{

using func_t = std::function<bool(void)>;
unsigned _register_(std::string_view, func_t);

using test_map_t = std::vector<std::pair<std::string_view, func_t>>;

inline test_map_t map;


#define IDLETEST_FUNCTION_NAME_(testname) idle_testcase_##testname##_

#define IDLETEST_FUNCTION_ID_(testname) idle_testcase_##testname##_id_

#define IDLETEST_CLASSNAME_(classname, testname) idle_classtest_##classname##_testcase_##testname##_

#define IDLETEST_FAILSI_ _idletest_fail_counter_

constexpr bool _test_for_illegal_characters_(const std::string_view name)
{
    for (auto c : name)
    {
        if (!(
                (c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') ||
                c == '_'
             ))
            return false;
    }
    return true;
}

#define IDLETEST_NAME_ASSERT(name) static_assert(::idletest::_test_for_illegal_characters_(#name));



#define TEST(testname) IDLETEST_NAME_ASSERT(testname) \
static void IDLETEST_FUNCTION_NAME_(testname) (unsigned&); \
static const unsigned IDLETEST_FUNCTION_ID_(testname) = \
        ::idletest::_register_(#testname, []()->bool \
        { \
            unsigned count = 0; \
            IDLETEST_FUNCTION_NAME_(testname) (count); \
            return count == 0; \
        }); \
static void IDLETEST_FUNCTION_NAME_(testname) (unsigned& IDLETEST_FAILSI_)



#define TEST_C(classname, testname) IDLETEST_NAME_ASSERT(classname) IDLETEST_NAME_ASSERT(testname) \
class IDLETEST_CLASSNAME_(classname, testname) : classname { \
public: \
    bool test_head() { \
        unsigned count = 0; \
        test_body(count); \
        return count == 0; \
    } \
private: \
    void test_body(unsigned&); \
    static const unsigned _id; \
}; \
const unsigned IDLETEST_CLASSNAME_(classname, testname)::_id \
    = ::idletest::_register_(#classname ":" #testname, \
        []()->bool { return IDLETEST_CLASSNAME_(classname, testname){}.test_head(); }); \
void IDLETEST_CLASSNAME_(classname, testname)::test_body(unsigned& IDLETEST_FAILSI_)



void _print_failed_expr_(unsigned long line, const std::string& failmsg);


#define IDLETEST_EXPECTATION(pred, failmsg) do if (!(pred)) { \
    ++(IDLETEST_FAILSI_); \
    ::idletest::_print_failed_expr_(__LINE__, failmsg); \
} while (false)



#define EXPECT_TRUE(x) IDLETEST_EXPECTATION(!!(x), #x " does not evaluate to true")

#define EXPECT_FALSE(x) IDLETEST_EXPECTATION(!(x), #x " does not evaluate to false")

#define EXPECT_EQUAL(x, y) IDLETEST_EXPECTATION((x) == (y), #x " and " #y " values aren't equal")

#define EXPECT_UNEQUAL(x, y) IDLETEST_EXPECTATION((x) != (y), #x " and " #y " values are equal")

}

#endif
