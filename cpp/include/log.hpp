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
#ifdef __ANDROID__ //Android build
#include <jni.h>
#include <android/sensor.h>

#include <android/log.h>
#include <android_native_app_glue.h>

#define LOGCAT_COMMENT "na.idle"

#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, LOGCAT_COMMENT, __VA_ARGS__))
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, LOGCAT_COMMENT, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, LOGCAT_COMMENT, __VA_ARGS__))

#else

#include <cstdio>

#define LOGE(...) ((void)std::fputs(u8"🚨", stderr));\
                    ((void)std::fprintf(stderr, __VA_ARGS__));\
                    ((void)std::putc('\n', stderr))

#define LOGI(...) ((void)std::fputs(u8"ℹ️", stdout));\
                    ((void)std::fprintf(stdout, __VA_ARGS__));\
                    ((void)std::putc('\n', stdout))

#define LOGW(...) ((void)std::fputs(u8"⚠️", stderr));\
                    ((void)std::fprintf(stderr, __VA_ARGS__));\
                    ((void)std::putc('\n', stderr))

#endif

#ifdef DEBUG
#define LOGD(...) ((void)std::putc(' ', stdout));\
                    ((void)std::fprintf(stdout, __VA_ARGS__));\
                    ((void)std::putc('\n', stdout))
#else
#define LOGD(...) ((void)0)
#endif
