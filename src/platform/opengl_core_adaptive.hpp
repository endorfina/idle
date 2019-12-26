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

/*
    Generated with glLoadGen and extended for compatibility
*/

#ifndef POINTER_CPP_GENERATED_HEADEROPENGL_HPP
#define POINTER_CPP_GENERATED_HEADEROPENGL_HPP

#if defined(__glew_h__) || defined(__GLEW_H__)
#error Attempt to include auto-generated header after including glew.h
#endif
#if defined(__gl_h_) || defined(__GL_H__)
#error Attempt to include auto-generated header after including gl.h
#endif
#if defined(__gl2_h_) || defined(__GL2_H__)
#error Attempt to include auto-generated header after including gl2.h
#endif
#if defined(__glext_h_) || defined(__GLEXT_H_)
#error Attempt to include auto-generated header after including glext.h
#endif
#if defined(__gltypes_h_)
#error Attempt to include auto-generated header after gltypes.h
#endif
#if defined(__gl_ATI_h_)
#error Attempt to include auto-generated header after including glATI.h
#endif

#define __glew_h__
#define __GLEW_H__
#define __gl_h_
#define __gl2_h_
#define __GL_H__
#define __glext_h_
#define __GLEXT_H_
#define __gltypes_h_
#define __gl_ATI_h_

#ifndef APIENTRY
    #if defined(__MINGW32__)
        #ifndef WIN32_LEAN_AND_MEAN
            #define WIN32_LEAN_AND_MEAN 1
        #endif
        #ifndef NOMINMAX
            #define NOMINMAX
        #endif
        #include <windows.h>
    #elif (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED) || defined(__BORLANDC__)
        #ifndef WIN32_LEAN_AND_MEAN
            #define WIN32_LEAN_AND_MEAN 1
        #endif
        #ifndef NOMINMAX
            #define NOMINMAX
        #endif
        #include <windows.h>
    #else
        #define APIENTRY
    #endif
#endif /*APIENTRY*/

#ifndef CODEGEN_FUNCPTR
    #define CODEGEN_REMOVE_FUNCPTR
    #if defined(_WIN32)
        #define CODEGEN_FUNCPTR APIENTRY
    #else
        #define CODEGEN_FUNCPTR
    #endif
#endif /*CODEGEN_FUNCPTR*/

#ifndef GLAPI
    #define GLAPI extern
#endif


#ifndef GL_LOAD_GEN_BASIC_OPENGL_TYPEDEFS
#define GL_LOAD_GEN_BASIC_OPENGL_TYPEDEFS


#endif /*GL_LOAD_GEN_BASIC_OPENGL_TYPEDEFS*/

#include <stddef.h>
#ifndef GLEXT_64_TYPES_DEFINED
/* This code block is duplicated in glxext.h, so must be protected */
#define GLEXT_64_TYPES_DEFINED
/* Define int32_t, int64_t, and uint64_t types for UST/MSC */
/* (as used in the GL_EXT_timer_query extension). */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#include <inttypes.h>
#elif defined(__sun__) || defined(__digital__)
#include <inttypes.h>
#if defined(__STDC__)
#if defined(__arch64__) || defined(_LP64)
typedef long int int64_t;
typedef unsigned long int uint64_t;
#else
typedef long long int int64_t;
typedef unsigned long long int uint64_t;
#endif /* __arch64__ */
#endif /* __STDC__ */
#elif defined( __VMS ) || defined(__sgi)
#include <inttypes.h>
#elif defined(__SCO__) || defined(__USLC__)
#include <stdint.h>
#elif defined(__UNIXOS2__) || defined(__SOL64__)
typedef long int int32_t;
typedef long long int int64_t;
typedef unsigned long long int uint64_t;
#elif defined(_WIN32) && defined(__GNUC__)
#include <stdint.h>
#elif defined(_WIN32)
typedef __int32 int32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
/* Fallback if nothing above works */
#include <inttypes.h>
#endif
#endif
    typedef unsigned int GLenum;
    typedef unsigned char GLboolean;
    typedef unsigned int GLbitfield;
    typedef void GLvoid;
    typedef signed char GLbyte;
    typedef short GLshort;
    typedef int GLint;
    typedef unsigned char GLubyte;
    typedef unsigned short GLushort;
    typedef unsigned int GLuint;
    typedef int GLsizei;
    typedef float GLfloat;
    typedef float GLclampf;
    typedef double GLdouble;
    typedef double GLclampd;
    typedef char GLchar;
    typedef char GLcharARB;
    #ifdef __APPLE__
typedef void *GLhandleARB;
#else
typedef unsigned int GLhandleARB;
#endif
        typedef unsigned short GLhalfARB;
        typedef unsigned short GLhalf;
        typedef GLint GLfixed;
        typedef ptrdiff_t GLintptr;
        typedef ptrdiff_t GLsizeiptr;
        typedef int64_t GLint64;
        typedef uint64_t GLuint64;
        typedef ptrdiff_t GLintptrARB;
        typedef ptrdiff_t GLsizeiptrARB;
        typedef int64_t GLint64EXT;
        typedef uint64_t GLuint64EXT;
        typedef struct __GLsync *GLsync;
        struct _cl_context;
        struct _cl_event;
        typedef void (APIENTRY *GLDEBUGPROC)(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam);
        typedef void (APIENTRY *GLDEBUGPROCARB)(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam);
        typedef void (APIENTRY *GLDEBUGPROCAMD)(GLuint id,GLenum category,GLenum severity,GLsizei length,const GLchar *message,void *userParam);
        typedef unsigned short GLhalfNV;
        typedef GLintptr GLvdpauSurfaceNV;

namespace gl
{
    namespace exts
    {
        class LoadTest
        {
        private:
            //Safe bool idiom. Joy!
            typedef void (LoadTest::*bool_type)() const;
            void big_long_name_that_really_doesnt_matter() const {}

        public:
            operator bool_type() const
            {
                return m_isLoaded ? &LoadTest::big_long_name_that_really_doesnt_matter : 0;
            }

            int GetNumMissing() const {return m_numMissing;}

            LoadTest() : m_isLoaded(false), m_numMissing(0) {}
            LoadTest(bool isLoaded, int numMissing) : m_isLoaded(isLoaded), m_numMissing(numMissing) {}
        private:
            bool m_isLoaded;
            int m_numMissing;
        };

    } //namespace exts
    enum
    {
        ACTIVE_ATTRIBUTES                  = 0x8b89,
        ACTIVE_ATTRIBUTE_MAX_LENGTH        = 0x8b8a,
        ACTIVE_TEXTURE                     = 0x84e0,
        ACTIVE_UNIFORMS                    = 0x8b86,
        ACTIVE_UNIFORM_MAX_LENGTH          = 0x8b87,
        ALIASED_LINE_WIDTH_RANGE           = 0x846e,
        ALPHA                              = 0x1906,
        ALWAYS                             = 0x207,
        ARRAY_BUFFER                       = 0x8892,
        ARRAY_BUFFER_BINDING               = 0x8894,
        ATTACHED_SHADERS                   = 0x8b85,
        BACK                               = 0x405,
        BLEND                              = 0xbe2,
        BLEND_COLOR                        = 0x8005,
        BLEND_DST_ALPHA                    = 0x80ca,
        BLEND_DST_RGB                      = 0x80c8,
        BLEND_EQUATION_ALPHA               = 0x883d,
        BLEND_EQUATION_RGB                 = 0x8009,
        BLEND_SRC_ALPHA                    = 0x80cb,
        BLEND_SRC_RGB                      = 0x80c9,
        BOOL                               = 0x8b56,
        BOOL_VEC2                          = 0x8b57,
        BOOL_VEC3                          = 0x8b58,
        BOOL_VEC4                          = 0x8b59,
        BUFFER_SIZE                        = 0x8764,
        BUFFER_USAGE                       = 0x8765,
        BYTE                               = 0x1400,
        CCW                                = 0x901,
        CLAMP_TO_EDGE                      = 0x812f,
        COLOR_ATTACHMENT0                  = 0x8ce0,
        COLOR_BUFFER_BIT                   = 0x4000,
        COLOR_CLEAR_VALUE                  = 0xc22,
        COLOR_WRITEMASK                    = 0xc23,
        COMPILE_STATUS                     = 0x8b81,
        COMPRESSED_TEXTURE_FORMATS         = 0x86a3,
        CONSTANT_ALPHA                     = 0x8003,
        CONSTANT_COLOR                     = 0x8001,
        CULL_FACE                          = 0xb44,
        CULL_FACE_MODE                     = 0xb45,
        CURRENT_PROGRAM                    = 0x8b8d,
        CURRENT_VERTEX_ATTRIB              = 0x8626,
        CW                                 = 0x900,
        DECR                               = 0x1e03,
        DECR_WRAP                          = 0x8508,
        DELETE_STATUS                      = 0x8b80,
        DEPTH_ATTACHMENT                   = 0x8d00,
        DEPTH_BUFFER_BIT                   = 0x100,
        DEPTH_CLEAR_VALUE                  = 0xb73,
        DEPTH_COMPONENT                    = 0x1902,
        DEPTH_COMPONENT16                  = 0x81a5,
        DEPTH_FUNC                         = 0xb74,
        DEPTH_RANGE                        = 0xb70,
        DEPTH_TEST                         = 0xb71,
        DEPTH_WRITEMASK                    = 0xb72,
        DITHER                             = 0xbd0,
        DONT_CARE                          = 0x1100,
        DST_ALPHA                          = 0x304,
        DST_COLOR                          = 0x306,
        DYNAMIC_DRAW                       = 0x88e8,
        ELEMENT_ARRAY_BUFFER               = 0x8893,
        ELEMENT_ARRAY_BUFFER_BINDING       = 0x8895,
        EQUAL                              = 0x202,
        EXTENSIONS                         = 0x1f03,
        FALSE_                             = 0,
        FASTEST                            = 0x1101,
        FLOAT                              = 0x1406,
        FLOAT_MAT2                         = 0x8b5a,
        FLOAT_MAT3                         = 0x8b5b,
        FLOAT_MAT4                         = 0x8b5c,
        FLOAT_VEC2                         = 0x8b50,
        FLOAT_VEC3                         = 0x8b51,
        FLOAT_VEC4                         = 0x8b52,
        FRAGMENT_SHADER                    = 0x8b30,
        FRAMEBUFFER                        = 0x8d40,
        FRAMEBUFFER_ATTACHMENT_OBJECT_NAME = 0x8cd1,
        FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE = 0x8cd0,
        FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE = 0x8cd3,
        FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL = 0x8cd2,
        FRAMEBUFFER_BINDING                = 0x8ca6,
        FRAMEBUFFER_COMPLETE               = 0x8cd5,
        FRAMEBUFFER_INCOMPLETE_ATTACHMENT  = 0x8cd6,
        FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT = 0x8cd7,
        FRAMEBUFFER_UNSUPPORTED            = 0x8cdd,
        FRONT                              = 0x404,
        FRONT_AND_BACK                     = 0x408,
        FRONT_FACE                         = 0xb46,
        FUNC_ADD                           = 0x8006,
        FUNC_REVERSE_SUBTRACT              = 0x800b,
        FUNC_SUBTRACT                      = 0x800a,
        GEQUAL                             = 0x206,
        GREATER                            = 0x204,
        INCR                               = 0x1e02,
        INCR_WRAP                          = 0x8507,
        INFO_LOG_LENGTH                    = 0x8b84,
        INT                                = 0x1404,
        INT_VEC2                           = 0x8b53,
        INT_VEC3                           = 0x8b54,
        INT_VEC4                           = 0x8b55,
        INVALID_ENUM                       = 0x500,
        INVALID_FRAMEBUFFER_OPERATION      = 0x506,
        INVALID_OPERATION                  = 0x502,
        INVALID_VALUE                      = 0x501,
        INVERT                             = 0x150a,
        KEEP                               = 0x1e00,
        LEQUAL                             = 0x203,
        LESS                               = 0x201,
        LINEAR                             = 0x2601,
        LINEAR_MIPMAP_LINEAR               = 0x2703,
        LINEAR_MIPMAP_NEAREST              = 0x2701,
        LINES                              = 1,
        LINE_LOOP                          = 2,
        LINE_STRIP                         = 3,
        LINE_WIDTH                         = 0xb21,
        LINK_STATUS                        = 0x8b82,
        MAX_COMBINED_TEXTURE_IMAGE_UNITS   = 0x8b4d,
        MAX_CUBE_MAP_TEXTURE_SIZE          = 0x851c,
        MAX_RENDERBUFFER_SIZE              = 0x84e8,
        MAX_TEXTURE_IMAGE_UNITS            = 0x8872,
        MAX_TEXTURE_SIZE                   = 0xd33,
        MAX_VERTEX_ATTRIBS                 = 0x8869,
        MAX_VERTEX_TEXTURE_IMAGE_UNITS     = 0x8b4c,
        MAX_VIEWPORT_DIMS                  = 0xd3a,
        MIRRORED_REPEAT                    = 0x8370,
        NEAREST                            = 0x2600,
        NEAREST_MIPMAP_LINEAR              = 0x2702,
        NEAREST_MIPMAP_NEAREST             = 0x2700,
        NEVER                              = 0x200,
        NICEST                             = 0x1102,
        NONE                               = 0,
        NOTEQUAL                           = 0x205,
        NO_ERROR_                          = 0,
        NUM_COMPRESSED_TEXTURE_FORMATS     = 0x86a2,
        ONE                                = 1,
        ONE_MINUS_CONSTANT_ALPHA           = 0x8004,
        ONE_MINUS_CONSTANT_COLOR           = 0x8002,
        ONE_MINUS_DST_ALPHA                = 0x305,
        ONE_MINUS_DST_COLOR                = 0x307,
        ONE_MINUS_SRC_ALPHA                = 0x303,
        ONE_MINUS_SRC_COLOR                = 0x301,
        OUT_OF_MEMORY                      = 0x505,
        PACK_ALIGNMENT                     = 0xd05,
        POINTS                             = 0,
        POLYGON_OFFSET_FACTOR              = 0x8038,
        POLYGON_OFFSET_FILL                = 0x8037,
        POLYGON_OFFSET_UNITS               = 0x2a00,
        RENDERBUFFER                       = 0x8d41,
        RENDERBUFFER_ALPHA_SIZE            = 0x8d53,
        RENDERBUFFER_BINDING               = 0x8ca7,
        RENDERBUFFER_BLUE_SIZE             = 0x8d52,
        RENDERBUFFER_DEPTH_SIZE            = 0x8d54,
        RENDERBUFFER_GREEN_SIZE            = 0x8d51,
        RENDERBUFFER_HEIGHT                = 0x8d43,
        RENDERBUFFER_INTERNAL_FORMAT       = 0x8d44,
        RENDERBUFFER_RED_SIZE              = 0x8d50,
        RENDERBUFFER_STENCIL_SIZE          = 0x8d55,
        RENDERBUFFER_WIDTH                 = 0x8d42,
        RENDERER                           = 0x1f01,
        REPEAT                             = 0x2901,
        REPLACE                            = 0x1e01,
        RGB                                = 0x1907,
        RGB5_A1                            = 0x8057,
        RGBA                               = 0x1908,
        RGBA4                              = 0x8056,
        SAMPLER_2D                         = 0x8b5e,
        SAMPLER_CUBE                       = 0x8b60,
        SAMPLES                            = 0x80a9,
        SAMPLE_ALPHA_TO_COVERAGE           = 0x809e,
        SAMPLE_BUFFERS                     = 0x80a8,
        SAMPLE_COVERAGE                    = 0x80a0,
        SAMPLE_COVERAGE_INVERT             = 0x80ab,
        SAMPLE_COVERAGE_VALUE              = 0x80aa,
        SCISSOR_BOX                        = 0xc10,
        SCISSOR_TEST                       = 0xc11,
        SHADER_SOURCE_LENGTH               = 0x8b88,
        SHADER_TYPE                        = 0x8b4f,
        SHADING_LANGUAGE_VERSION           = 0x8b8c,
        SHORT                              = 0x1402,
        SRC_ALPHA                          = 0x302,
        SRC_ALPHA_SATURATE                 = 0x308,
        SRC_COLOR                          = 0x300,
        STATIC_DRAW                        = 0x88e4,
        STENCIL_ATTACHMENT                 = 0x8d20,
        STENCIL_BACK_FAIL                  = 0x8801,
        STENCIL_BACK_FUNC                  = 0x8800,
        STENCIL_BACK_PASS_DEPTH_FAIL       = 0x8802,
        STENCIL_BACK_PASS_DEPTH_PASS       = 0x8803,
        STENCIL_BACK_REF                   = 0x8ca3,
        STENCIL_BACK_VALUE_MASK            = 0x8ca4,
        STENCIL_BACK_WRITEMASK             = 0x8ca5,
        STENCIL_BUFFER_BIT                 = 0x400,
        STENCIL_CLEAR_VALUE                = 0xb91,
        STENCIL_FAIL                       = 0xb94,
        STENCIL_FUNC                       = 0xb92,
        STENCIL_INDEX8                     = 0x8d48,
        STENCIL_PASS_DEPTH_FAIL            = 0xb95,
        STENCIL_PASS_DEPTH_PASS            = 0xb96,
        STENCIL_REF                        = 0xb97,
        STENCIL_TEST                       = 0xb90,
        STENCIL_VALUE_MASK                 = 0xb93,
        STENCIL_WRITEMASK                  = 0xb98,
        STREAM_DRAW                        = 0x88e0,
        SUBPIXEL_BITS                      = 0xd50,
        TEXTURE                            = 0x1702,
        TEXTURE0                           = 0x84c0,
        TEXTURE1                           = 0x84c1,
        TEXTURE10                          = 0x84ca,
        TEXTURE11                          = 0x84cb,
        TEXTURE12                          = 0x84cc,
        TEXTURE13                          = 0x84cd,
        TEXTURE14                          = 0x84ce,
        TEXTURE15                          = 0x84cf,
        TEXTURE16                          = 0x84d0,
        TEXTURE17                          = 0x84d1,
        TEXTURE18                          = 0x84d2,
        TEXTURE19                          = 0x84d3,
        TEXTURE2                           = 0x84c2,
        TEXTURE20                          = 0x84d4,
        TEXTURE21                          = 0x84d5,
        TEXTURE22                          = 0x84d6,
        TEXTURE23                          = 0x84d7,
        TEXTURE24                          = 0x84d8,
        TEXTURE25                          = 0x84d9,
        TEXTURE26                          = 0x84da,
        TEXTURE27                          = 0x84db,
        TEXTURE28                          = 0x84dc,
        TEXTURE29                          = 0x84dd,
        TEXTURE3                           = 0x84c3,
        TEXTURE30                          = 0x84de,
        TEXTURE31                          = 0x84df,
        TEXTURE4                           = 0x84c4,
        TEXTURE5                           = 0x84c5,
        TEXTURE6                           = 0x84c6,
        TEXTURE7                           = 0x84c7,
        TEXTURE8                           = 0x84c8,
        TEXTURE9                           = 0x84c9,
        TEXTURE_2D                         = 0xde1,
        TEXTURE_BINDING_2D                 = 0x8069,
        TEXTURE_BINDING_CUBE_MAP           = 0x8514,
        TEXTURE_CUBE_MAP                   = 0x8513,
        TEXTURE_CUBE_MAP_NEGATIVE_X        = 0x8516,
        TEXTURE_CUBE_MAP_NEGATIVE_Y        = 0x8518,
        TEXTURE_CUBE_MAP_NEGATIVE_Z        = 0x851a,
        TEXTURE_CUBE_MAP_POSITIVE_X        = 0x8515,
        TEXTURE_CUBE_MAP_POSITIVE_Y        = 0x8517,
        TEXTURE_CUBE_MAP_POSITIVE_Z        = 0x8519,
        TEXTURE_MAG_FILTER                 = 0x2800,
        TEXTURE_MIN_FILTER                 = 0x2801,
        TEXTURE_WRAP_S                     = 0x2802,
        TEXTURE_WRAP_T                     = 0x2803,
        TRIANGLES                          = 4,
        TRIANGLE_FAN                       = 6,
        TRIANGLE_STRIP                     = 5,
        TRUE_                              = 1,
        UNPACK_ALIGNMENT                   = 0xcf5,
        UNSIGNED_BYTE                      = 0x1401,
        UNSIGNED_INT                       = 0x1405,
        UNSIGNED_SHORT                     = 0x1403,
        UNSIGNED_SHORT_4_4_4_4             = 0x8033,
        UNSIGNED_SHORT_5_5_5_1             = 0x8034,
        UNSIGNED_SHORT_5_6_5               = 0x8363,
        VALIDATE_STATUS                    = 0x8b83,
        VENDOR                             = 0x1f00,
        VERSION                            = 0x1f02,
        VERTEX_ATTRIB_ARRAY_BUFFER_BINDING = 0x889f,
        VERTEX_ATTRIB_ARRAY_ENABLED        = 0x8622,
        VERTEX_ATTRIB_ARRAY_NORMALIZED     = 0x886a,
        VERTEX_ATTRIB_ARRAY_POINTER        = 0x8645,
        VERTEX_ATTRIB_ARRAY_SIZE           = 0x8623,
        VERTEX_ATTRIB_ARRAY_STRIDE         = 0x8624,
        VERTEX_ATTRIB_ARRAY_TYPE           = 0x8625,
        VERTEX_SHADER                      = 0x8b31,
        VIEWPORT                           = 0xba2,
        ZERO                               = 0,
#ifndef __ANDROID__
        ACTIVE_UNIFORM_BLOCKS              = 0x8a36,
        ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH = 0x8a35,
        ALREADY_SIGNALED                   = 0x911a,
        AND                                = 0x1501,
        AND_INVERTED                       = 0x1504,
        AND_REVERSE                        = 0x1502,
        BACK_LEFT                          = 0x402,
        BACK_RIGHT                         = 0x403,
        BGR                                = 0x80e0,
        BGRA                               = 0x80e1,
        BGRA_INTEGER                       = 0x8d9b,
        BGR_INTEGER                        = 0x8d9a,
        BLEND_DST                          = 0xbe0,
        BLEND_SRC                          = 0xbe1,
        BLUE                               = 0x1905,
        BLUE_INTEGER                       = 0x8d96,
        BUFFER_ACCESS                      = 0x88bb,
        BUFFER_ACCESS_FLAGS                = 0x911f,
        BUFFER_MAPPED                      = 0x88bc,
        BUFFER_MAP_LENGTH                  = 0x9120,
        BUFFER_MAP_OFFSET                  = 0x9121,
        BUFFER_MAP_POINTER                 = 0x88bd,
        CLAMP_READ_COLOR                   = 0x891c,
        CLAMP_TO_BORDER                    = 0x812d,
        CLEAR                              = 0x1500,
        CLIP_DISTANCE0                     = 0x3000,
        CLIP_DISTANCE1                     = 0x3001,
        CLIP_DISTANCE2                     = 0x3002,
        CLIP_DISTANCE3                     = 0x3003,
        CLIP_DISTANCE4                     = 0x3004,
        CLIP_DISTANCE5                     = 0x3005,
        CLIP_DISTANCE6                     = 0x3006,
        CLIP_DISTANCE7                     = 0x3007,
        COLOR                              = 0x1800,
        COLOR_ATTACHMENT1                  = 0x8ce1,
        COLOR_ATTACHMENT10                 = 0x8cea,
        COLOR_ATTACHMENT11                 = 0x8ceb,
        COLOR_ATTACHMENT12                 = 0x8cec,
        COLOR_ATTACHMENT13                 = 0x8ced,
        COLOR_ATTACHMENT14                 = 0x8cee,
        COLOR_ATTACHMENT15                 = 0x8cef,
        COLOR_ATTACHMENT16                 = 0x8cf0,
        COLOR_ATTACHMENT17                 = 0x8cf1,
        COLOR_ATTACHMENT18                 = 0x8cf2,
        COLOR_ATTACHMENT19                 = 0x8cf3,
        COLOR_ATTACHMENT2                  = 0x8ce2,
        COLOR_ATTACHMENT20                 = 0x8cf4,
        COLOR_ATTACHMENT21                 = 0x8cf5,
        COLOR_ATTACHMENT22                 = 0x8cf6,
        COLOR_ATTACHMENT23                 = 0x8cf7,
        COLOR_ATTACHMENT24                 = 0x8cf8,
        COLOR_ATTACHMENT25                 = 0x8cf9,
        COLOR_ATTACHMENT26                 = 0x8cfa,
        COLOR_ATTACHMENT27                 = 0x8cfb,
        COLOR_ATTACHMENT28                 = 0x8cfc,
        COLOR_ATTACHMENT29                 = 0x8cfd,
        COLOR_ATTACHMENT3                  = 0x8ce3,
        COLOR_ATTACHMENT30                 = 0x8cfe,
        COLOR_ATTACHMENT31                 = 0x8cff,
        COLOR_ATTACHMENT4                  = 0x8ce4,
        COLOR_ATTACHMENT5                  = 0x8ce5,
        COLOR_ATTACHMENT6                  = 0x8ce6,
        COLOR_ATTACHMENT7                  = 0x8ce7,
        COLOR_ATTACHMENT8                  = 0x8ce8,
        COLOR_ATTACHMENT9                  = 0x8ce9,
        COLOR_LOGIC_OP                     = 0xbf2,
        COMPARE_REF_TO_TEXTURE             = 0x884e,
        COMPRESSED_RED                     = 0x8225,
        COMPRESSED_RED_RGTC1               = 0x8dbb,
        COMPRESSED_RG                      = 0x8226,
        COMPRESSED_RGB                     = 0x84ed,
        COMPRESSED_RGBA                    = 0x84ee,
        COMPRESSED_RG_RGTC2                = 0x8dbd,
        COMPRESSED_SIGNED_RED_RGTC1        = 0x8dbc,
        COMPRESSED_SIGNED_RG_RGTC2         = 0x8dbe,
        COMPRESSED_SRGB                    = 0x8c48,
        COMPRESSED_SRGB_ALPHA              = 0x8c49,
        CONDITION_SATISFIED                = 0x911c,
        CONTEXT_COMPATIBILITY_PROFILE_BIT  = 2,
        CONTEXT_CORE_PROFILE_BIT           = 1,
        CONTEXT_FLAGS                      = 0x821e,
        CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT = 1,
        CONTEXT_PROFILE_MASK               = 0x9126,
        COPY                               = 0x1503,
        COPY_INVERTED                      = 0x150c,
        COPY_READ_BUFFER                   = 0x8f36,
        COPY_WRITE_BUFFER                  = 0x8f37,
        CURRENT_QUERY                      = 0x8865,
        DEPTH                              = 0x1801,
        DEPTH24_STENCIL8                   = 0x88f0,
        DEPTH32F_STENCIL8                  = 0x8cad,
        DEPTH_CLAMP                        = 0x864f,
        DEPTH_COMPONENT24                  = 0x81a6,
        DEPTH_COMPONENT32                  = 0x81a7,
        DEPTH_COMPONENT32F                 = 0x8cac,
        DEPTH_STENCIL                      = 0x84f9,
        DEPTH_STENCIL_ATTACHMENT           = 0x821a,
        DOUBLE                             = 0x140a,
        DOUBLEBUFFER                       = 0xc32,
        DRAW_BUFFER                        = 0xc01,
        DRAW_BUFFER0                       = 0x8825,
        DRAW_BUFFER1                       = 0x8826,
        DRAW_BUFFER10                      = 0x882f,
        DRAW_BUFFER11                      = 0x8830,
        DRAW_BUFFER12                      = 0x8831,
        DRAW_BUFFER13                      = 0x8832,
        DRAW_BUFFER14                      = 0x8833,
        DRAW_BUFFER15                      = 0x8834,
        DRAW_BUFFER2                       = 0x8827,
        DRAW_BUFFER3                       = 0x8828,
        DRAW_BUFFER4                       = 0x8829,
        DRAW_BUFFER5                       = 0x882a,
        DRAW_BUFFER6                       = 0x882b,
        DRAW_BUFFER7                       = 0x882c,
        DRAW_BUFFER8                       = 0x882d,
        DRAW_BUFFER9                       = 0x882e,
        DRAW_FRAMEBUFFER                   = 0x8ca9,
        DRAW_FRAMEBUFFER_BINDING           = 0x8ca6,
        DYNAMIC_COPY                       = 0x88ea,
        DYNAMIC_READ                       = 0x88e9,
        EQUIV                              = 0x1509,
        FILL                               = 0x1b02,
        FIRST_VERTEX_CONVENTION            = 0x8e4d,
        FIXED_ONLY                         = 0x891d,
        FLOAT_32_UNSIGNED_INT_24_8_REV     = 0x8dad,
        FLOAT_MAT2x3                       = 0x8b65,
        FLOAT_MAT2x4                       = 0x8b66,
        FLOAT_MAT3x2                       = 0x8b67,
        FLOAT_MAT3x4                       = 0x8b68,
        FLOAT_MAT4x2                       = 0x8b69,
        FLOAT_MAT4x3                       = 0x8b6a,
        FRAGMENT_SHADER_DERIVATIVE_HINT    = 0x8b8b,
        FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE  = 0x8215,
        FRAMEBUFFER_ATTACHMENT_BLUE_SIZE   = 0x8214,
        FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING = 0x8210,
        FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE = 0x8211,
        FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE  = 0x8216,
        FRAMEBUFFER_ATTACHMENT_GREEN_SIZE  = 0x8213,
        FRAMEBUFFER_ATTACHMENT_LAYERED     = 0x8da7,
        FRAMEBUFFER_ATTACHMENT_RED_SIZE    = 0x8212,
        FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE = 0x8217,
        FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER = 0x8cd4,
        FRAMEBUFFER_DEFAULT                = 0x8218,
        FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER = 0x8cdb,
        FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS = 0x8da8,
        FRAMEBUFFER_INCOMPLETE_MULTISAMPLE = 0x8d56,
        FRAMEBUFFER_INCOMPLETE_READ_BUFFER = 0x8cdc,
        FRAMEBUFFER_SRGB                   = 0x8db9,
        FRAMEBUFFER_UNDEFINED              = 0x8219,
        FRONT_LEFT                         = 0x400,
        FRONT_RIGHT                        = 0x401,
        GEOMETRY_INPUT_TYPE                = 0x8917,
        GEOMETRY_OUTPUT_TYPE               = 0x8918,
        GEOMETRY_SHADER                    = 0x8dd9,
        GEOMETRY_VERTICES_OUT              = 0x8916,
        GREEN                              = 0x1904,
        GREEN_INTEGER                      = 0x8d95,
        HALF_FLOAT                         = 0x140b,
        INTERLEAVED_ATTRIBS                = 0x8c8c,
        INT_SAMPLER_1D                     = 0x8dc9,
        INT_SAMPLER_1D_ARRAY               = 0x8dce,
        INT_SAMPLER_2D                     = 0x8dca,
        INT_SAMPLER_2D_ARRAY               = 0x8dcf,
        INT_SAMPLER_2D_MULTISAMPLE         = 0x9109,
        INT_SAMPLER_2D_MULTISAMPLE_ARRAY   = 0x910c,
        INT_SAMPLER_2D_RECT                = 0x8dcd,
        INT_SAMPLER_3D                     = 0x8dcb,
        INT_SAMPLER_BUFFER                 = 0x8dd0,
        INT_SAMPLER_CUBE                   = 0x8dcc,
        INVALID_INDEX                      = 0xffffffff,
        LAST_VERTEX_CONVENTION             = 0x8e4e,
        LEFT                               = 0x406,
        LINE                               = 0x1b01,
        LINES_ADJACENCY                    = 0xa,
        LINE_SMOOTH                        = 0xb20,
        LINE_SMOOTH_HINT                   = 0xc52,
        LINE_STRIP_ADJACENCY               = 0xb,
        LINE_WIDTH_GRANULARITY             = 0xb23,
        LINE_WIDTH_RANGE                   = 0xb22,
        LOGIC_OP_MODE                      = 0xbf0,
        LOWER_LEFT                         = 0x8ca1,
        MAJOR_VERSION                      = 0x821b,
        MAP_FLUSH_EXPLICIT_BIT             = 0x10,
        MAP_INVALIDATE_BUFFER_BIT          = 8,
        MAP_INVALIDATE_RANGE_BIT           = 4,
        MAP_READ_BIT                       = 1,
        MAP_UNSYNCHRONIZED_BIT             = 0x20,
        MAP_WRITE_BIT                      = 2,
        MAX                                = 0x8008,
        MAX_3D_TEXTURE_SIZE                = 0x8073,
        MAX_ARRAY_TEXTURE_LAYERS           = 0x88ff,
        MAX_CLIP_DISTANCES                 = 0xd32,
        MAX_COLOR_ATTACHMENTS              = 0x8cdf,
        MAX_COLOR_TEXTURE_SAMPLES          = 0x910e,
        MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS = 0x8a33,
        MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS = 0x8a32,
        MAX_COMBINED_UNIFORM_BLOCKS        = 0x8a2e,
        MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS = 0x8a31,
        MAX_DEPTH_TEXTURE_SAMPLES          = 0x910f,
        MAX_DRAW_BUFFERS                   = 0x8824,
        MAX_ELEMENTS_INDICES               = 0x80e9,
        MAX_ELEMENTS_VERTICES              = 0x80e8,
        MAX_FRAGMENT_INPUT_COMPONENTS      = 0x9125,
        MAX_FRAGMENT_UNIFORM_BLOCKS        = 0x8a2d,
        MAX_FRAGMENT_UNIFORM_COMPONENTS    = 0x8b49,
        MAX_GEOMETRY_INPUT_COMPONENTS      = 0x9123,
        MAX_GEOMETRY_OUTPUT_COMPONENTS     = 0x9124,
        MAX_GEOMETRY_OUTPUT_VERTICES       = 0x8de0,
        MAX_GEOMETRY_TEXTURE_IMAGE_UNITS   = 0x8c29,
        MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS = 0x8de1,
        MAX_GEOMETRY_UNIFORM_BLOCKS        = 0x8a2c,
        MAX_GEOMETRY_UNIFORM_COMPONENTS    = 0x8ddf,
        MAX_INTEGER_SAMPLES                = 0x9110,
        MAX_PROGRAM_TEXEL_OFFSET           = 0x8905,
        MAX_RECTANGLE_TEXTURE_SIZE         = 0x84f8,
        MAX_SAMPLES                        = 0x8d57,
        MAX_SAMPLE_MASK_WORDS              = 0x8e59,
        MAX_SERVER_WAIT_TIMEOUT            = 0x9111,
        MAX_TEXTURE_BUFFER_SIZE            = 0x8c2b,
        MAX_TEXTURE_LOD_BIAS               = 0x84fd,
        MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS = 0x8c8a,
        MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS = 0x8c8b,
        MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS = 0x8c80,
        MAX_UNIFORM_BLOCK_SIZE             = 0x8a30,
        MAX_UNIFORM_BUFFER_BINDINGS        = 0x8a2f,
        MAX_VARYING_COMPONENTS             = 0x8b4b,
        MAX_VARYING_FLOATS                 = 0x8b4b,
        MAX_VERTEX_OUTPUT_COMPONENTS       = 0x9122,
        MAX_VERTEX_UNIFORM_BLOCKS          = 0x8a2b,
        MAX_VERTEX_UNIFORM_COMPONENTS      = 0x8b4a,
        MIN                                = 0x8007,
        MINOR_VERSION                      = 0x821c,
        MIN_PROGRAM_TEXEL_OFFSET           = 0x8904,
        MULTISAMPLE                        = 0x809d,
        NAND                               = 0x150e,
        NOOP                               = 0x1505,
        NOR                                = 0x1508,
        NUM_EXTENSIONS                     = 0x821d,
        OBJECT_TYPE                        = 0x9112,
        OR                                 = 0x1507,
        OR_INVERTED                        = 0x150d,
        OR_REVERSE                         = 0x150b,
        PACK_IMAGE_HEIGHT                  = 0x806c,
        PACK_LSB_FIRST                     = 0xd01,
        PACK_ROW_LENGTH                    = 0xd02,
        PACK_SKIP_IMAGES                   = 0x806b,
        PACK_SKIP_PIXELS                   = 0xd04,
        PACK_SKIP_ROWS                     = 0xd03,
        PACK_SWAP_BYTES                    = 0xd00,
        PIXEL_PACK_BUFFER                  = 0x88eb,
        PIXEL_PACK_BUFFER_BINDING          = 0x88ed,
        PIXEL_UNPACK_BUFFER                = 0x88ec,
        PIXEL_UNPACK_BUFFER_BINDING        = 0x88ef,
        POINT                              = 0x1b00,
        POINT_FADE_THRESHOLD_SIZE          = 0x8128,
        POINT_SIZE                         = 0xb11,
        POINT_SIZE_GRANULARITY             = 0xb13,
        POINT_SIZE_RANGE                   = 0xb12,
        POINT_SPRITE_COORD_ORIGIN          = 0x8ca0,
        POLYGON_MODE                       = 0xb40,
        POLYGON_OFFSET_LINE                = 0x2a02,
        POLYGON_OFFSET_POINT               = 0x2a01,
        POLYGON_SMOOTH                     = 0xb41,
        POLYGON_SMOOTH_HINT                = 0xc53,
        PRIMITIVES_GENERATED               = 0x8c87,
        PRIMITIVE_RESTART                  = 0x8f9d,
        PRIMITIVE_RESTART_INDEX            = 0x8f9e,
        PROGRAM_POINT_SIZE                 = 0x8642,
        PROVOKING_VERTEX                   = 0x8e4f,
        PROXY_TEXTURE_1D                   = 0x8063,
        PROXY_TEXTURE_1D_ARRAY             = 0x8c19,
        PROXY_TEXTURE_2D                   = 0x8064,
        PROXY_TEXTURE_2D_ARRAY             = 0x8c1b,
        PROXY_TEXTURE_2D_MULTISAMPLE       = 0x9101,
        PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY = 0x9103,
        PROXY_TEXTURE_3D                   = 0x8070,
        PROXY_TEXTURE_CUBE_MAP             = 0x851b,
        PROXY_TEXTURE_RECTANGLE            = 0x84f7,
        QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION = 0x8e4c,
        QUERY_BY_REGION_NO_WAIT            = 0x8e16,
        QUERY_BY_REGION_WAIT               = 0x8e15,
        QUERY_COUNTER_BITS                 = 0x8864,
        QUERY_NO_WAIT                      = 0x8e14,
        QUERY_RESULT                       = 0x8866,
        QUERY_RESULT_AVAILABLE             = 0x8867,
        QUERY_WAIT                         = 0x8e13,
        R11F_G11F_B10F                     = 0x8c3a,
        R16                                = 0x822a,
        R16F                               = 0x822d,
        R16I                               = 0x8233,
        R16UI                              = 0x8234,
        R16_SNORM                          = 0x8f98,
        R32F                               = 0x822e,
        R32I                               = 0x8235,
        R32UI                              = 0x8236,
        R3_G3_B2                           = 0x2a10,
        R8                                 = 0x8229,
        R8I                                = 0x8231,
        R8UI                               = 0x8232,
        R8_SNORM                           = 0x8f94,
        RASTERIZER_DISCARD                 = 0x8c89,
        READ_BUFFER                        = 0xc02,
        READ_FRAMEBUFFER                   = 0x8ca8,
        READ_FRAMEBUFFER_BINDING           = 0x8caa,
        READ_ONLY                          = 0x88b8,
        READ_WRITE                         = 0x88ba,
        RED                                = 0x1903,
        RED_INTEGER                        = 0x8d94,
        RENDERBUFFER_SAMPLES               = 0x8cab,
        RG                                 = 0x8227,
        RG16                               = 0x822c,
        RG16F                              = 0x822f,
        RG16I                              = 0x8239,
        RG16UI                             = 0x823a,
        RG16_SNORM                         = 0x8f99,
        RG32F                              = 0x8230,
        RG32I                              = 0x823b,
        RG32UI                             = 0x823c,
        RG8                                = 0x822b,
        RG8I                               = 0x8237,
        RG8UI                              = 0x8238,
        RG8_SNORM                          = 0x8f95,
        RGB10                              = 0x8052,
        RGB10_A2                           = 0x8059,
        RGB12                              = 0x8053,
        RGB16                              = 0x8054,
        RGB16F                             = 0x881b,
        RGB16I                             = 0x8d89,
        RGB16UI                            = 0x8d77,
        RGB16_SNORM                        = 0x8f9a,
        RGB32F                             = 0x8815,
        RGB32I                             = 0x8d83,
        RGB32UI                            = 0x8d71,
        RGB4                               = 0x804f,
        RGB5                               = 0x8050,
        RGB8                               = 0x8051,
        RGB8I                              = 0x8d8f,
        RGB8UI                             = 0x8d7d,
        RGB8_SNORM                         = 0x8f96,
        RGB9_E5                            = 0x8c3d,
        RGBA12                             = 0x805a,
        RGBA16                             = 0x805b,
        RGBA16F                            = 0x881a,
        RGBA16I                            = 0x8d88,
        RGBA16UI                           = 0x8d76,
        RGBA16_SNORM                       = 0x8f9b,
        RGBA2                              = 0x8055,
        RGBA32F                            = 0x8814,
        RGBA32I                            = 0x8d82,
        RGBA32UI                           = 0x8d70,
        RGBA8                              = 0x8058,
        RGBA8I                             = 0x8d8e,
        RGBA8UI                            = 0x8d7c,
        RGBA8_SNORM                        = 0x8f97,
        RGBA_INTEGER                       = 0x8d99,
        RGB_INTEGER                        = 0x8d98,
        RG_INTEGER                         = 0x8228,
        RIGHT                              = 0x407,
        SAMPLER_1D                         = 0x8b5d,
        SAMPLER_1D_ARRAY                   = 0x8dc0,
        SAMPLER_1D_ARRAY_SHADOW            = 0x8dc3,
        SAMPLER_1D_SHADOW                  = 0x8b61,
        SAMPLER_2D_ARRAY                   = 0x8dc1,
        SAMPLER_2D_ARRAY_SHADOW            = 0x8dc4,
        SAMPLER_2D_MULTISAMPLE             = 0x9108,
        SAMPLER_2D_MULTISAMPLE_ARRAY       = 0x910b,
        SAMPLER_2D_RECT                    = 0x8b63,
        SAMPLER_2D_RECT_SHADOW             = 0x8b64,
        SAMPLER_2D_SHADOW                  = 0x8b62,
        SAMPLER_3D                         = 0x8b5f,
        SAMPLER_BUFFER                     = 0x8dc2,
        SAMPLER_CUBE_SHADOW                = 0x8dc5,
        SAMPLES_PASSED                     = 0x8914,
        SAMPLE_ALPHA_TO_ONE                = 0x809f,
        SAMPLE_MASK                        = 0x8e51,
        SAMPLE_MASK_VALUE                  = 0x8e52,
        SAMPLE_POSITION                    = 0x8e50,
        SEPARATE_ATTRIBS                   = 0x8c8d,
        SET                                = 0x150f,
        SIGNALED                           = 0x9119,
        SIGNED_NORMALIZED                  = 0x8f9c,
        SMOOTH_LINE_WIDTH_GRANULARITY      = 0xb23,
        SMOOTH_LINE_WIDTH_RANGE            = 0xb22,
        SMOOTH_POINT_SIZE_GRANULARITY      = 0xb13,
        SMOOTH_POINT_SIZE_RANGE            = 0xb12,
        SRC1_ALPHA                         = 0x8589,
        SRGB                               = 0x8c40,
        SRGB8                              = 0x8c41,
        SRGB8_ALPHA8                       = 0x8c43,
        SRGB_ALPHA                         = 0x8c42,
        STATIC_COPY                        = 0x88e6,
        STATIC_READ                        = 0x88e5,
        STENCIL                            = 0x1802,
        STENCIL_INDEX                      = 0x1901,
        STENCIL_INDEX1                     = 0x8d46,
        STENCIL_INDEX16                    = 0x8d49,
        STENCIL_INDEX4                     = 0x8d47,
        STEREO                             = 0xc33,
        STREAM_COPY                        = 0x88e2,
        STREAM_READ                        = 0x88e1,
        SYNC_CONDITION                     = 0x9113,
        SYNC_FENCE                         = 0x9116,
        SYNC_FLAGS                         = 0x9115,
        SYNC_FLUSH_COMMANDS_BIT            = 1,
        SYNC_GPU_COMMANDS_COMPLETE         = 0x9117,
        SYNC_STATUS                        = 0x9114,
        TEXTURE_1D                         = 0xde0,
        TEXTURE_1D_ARRAY                   = 0x8c18,
        TEXTURE_2D_ARRAY                   = 0x8c1a,
        TEXTURE_2D_MULTISAMPLE             = 0x9100,
        TEXTURE_2D_MULTISAMPLE_ARRAY       = 0x9102,
        TEXTURE_3D                         = 0x806f,
        TEXTURE_ALPHA_SIZE                 = 0x805f,
        TEXTURE_ALPHA_TYPE                 = 0x8c13,
        TEXTURE_BASE_LEVEL                 = 0x813c,
        TEXTURE_BINDING_1D                 = 0x8068,
        TEXTURE_BINDING_1D_ARRAY           = 0x8c1c,
        TEXTURE_BINDING_2D_ARRAY           = 0x8c1d,
        TEXTURE_BINDING_2D_MULTISAMPLE     = 0x9104,
        TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY = 0x9105,
        TEXTURE_BINDING_3D                 = 0x806a,
        TEXTURE_BINDING_BUFFER             = 0x8c2c,
        TEXTURE_BINDING_RECTANGLE          = 0x84f6,
        TEXTURE_BLUE_SIZE                  = 0x805e,
        TEXTURE_BLUE_TYPE                  = 0x8c12,
        TEXTURE_BORDER_COLOR               = 0x1004,
        TEXTURE_BUFFER                     = 0x8c2a,
        TEXTURE_BUFFER_DATA_STORE_BINDING  = 0x8c2d,
        TEXTURE_COMPARE_FUNC               = 0x884d,
        TEXTURE_COMPARE_MODE               = 0x884c,
        TEXTURE_COMPRESSED                 = 0x86a1,
        TEXTURE_COMPRESSED_IMAGE_SIZE      = 0x86a0,
        TEXTURE_COMPRESSION_HINT           = 0x84ef,
        TEXTURE_CUBE_MAP_SEAMLESS          = 0x884f,
        TEXTURE_DEPTH                      = 0x8071,
        TEXTURE_DEPTH_SIZE                 = 0x884a,
        TEXTURE_DEPTH_TYPE                 = 0x8c16,
        TEXTURE_FIXED_SAMPLE_LOCATIONS     = 0x9107,
        TEXTURE_GREEN_SIZE                 = 0x805d,
        TEXTURE_GREEN_TYPE                 = 0x8c11,
        TEXTURE_HEIGHT                     = 0x1001,
        TEXTURE_INTERNAL_FORMAT            = 0x1003,
        TEXTURE_LOD_BIAS                   = 0x8501,
        TEXTURE_MAX_LEVEL                  = 0x813d,
        TEXTURE_MAX_LOD                    = 0x813b,
        TEXTURE_MIN_LOD                    = 0x813a,
        TEXTURE_RECTANGLE                  = 0x84f5,
        TEXTURE_RED_SIZE                   = 0x805c,
        TEXTURE_RED_TYPE                   = 0x8c10,
        TEXTURE_SAMPLES                    = 0x9106,
        TEXTURE_SHARED_SIZE                = 0x8c3f,
        TEXTURE_STENCIL_SIZE               = 0x88f1,
        TEXTURE_WIDTH                      = 0x1000,
        TEXTURE_WRAP_R                     = 0x8072,
        TIMEOUT_EXPIRED                    = 0x911b,
        TIMEOUT_IGNORED                    = 0xffffffffffffffff,
        TRANSFORM_FEEDBACK_BUFFER          = 0x8c8e,
        TRANSFORM_FEEDBACK_BUFFER_BINDING  = 0x8c8f,
        TRANSFORM_FEEDBACK_BUFFER_MODE     = 0x8c7f,
        TRANSFORM_FEEDBACK_BUFFER_SIZE     = 0x8c85,
        TRANSFORM_FEEDBACK_BUFFER_START    = 0x8c84,
        TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN = 0x8c88,
        TRANSFORM_FEEDBACK_VARYINGS        = 0x8c83,
        TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH = 0x8c76,
        TRIANGLES_ADJACENCY                = 0xc,
        TRIANGLE_STRIP_ADJACENCY           = 0xd,
        UNIFORM_ARRAY_STRIDE               = 0x8a3c,
        UNIFORM_BLOCK_ACTIVE_UNIFORMS      = 0x8a42,
        UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES = 0x8a43,
        UNIFORM_BLOCK_BINDING              = 0x8a3f,
        UNIFORM_BLOCK_DATA_SIZE            = 0x8a40,
        UNIFORM_BLOCK_INDEX                = 0x8a3a,
        UNIFORM_BLOCK_NAME_LENGTH          = 0x8a41,
        UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER = 0x8a46,
        UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADER = 0x8a45,
        UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER = 0x8a44,
        UNIFORM_BUFFER                     = 0x8a11,
        UNIFORM_BUFFER_BINDING             = 0x8a28,
        UNIFORM_BUFFER_OFFSET_ALIGNMENT    = 0x8a34,
        UNIFORM_BUFFER_SIZE                = 0x8a2a,
        UNIFORM_BUFFER_START               = 0x8a29,
        UNIFORM_IS_ROW_MAJOR               = 0x8a3e,
        UNIFORM_MATRIX_STRIDE              = 0x8a3d,
        UNIFORM_NAME_LENGTH                = 0x8a39,
        UNIFORM_OFFSET                     = 0x8a3b,
        UNIFORM_SIZE                       = 0x8a38,
        UNIFORM_TYPE                       = 0x8a37,
        UNPACK_IMAGE_HEIGHT                = 0x806e,
        UNPACK_LSB_FIRST                   = 0xcf1,
        UNPACK_ROW_LENGTH                  = 0xcf2,
        UNPACK_SKIP_IMAGES                 = 0x806d,
        UNPACK_SKIP_PIXELS                 = 0xcf4,
        UNPACK_SKIP_ROWS                   = 0xcf3,
        UNPACK_SWAP_BYTES                  = 0xcf0,
        UNSIGNALED                         = 0x9118,
        UNSIGNED_BYTE_2_3_3_REV            = 0x8362,
        UNSIGNED_BYTE_3_3_2                = 0x8032,
        UNSIGNED_INT_10F_11F_11F_REV       = 0x8c3b,
        UNSIGNED_INT_10_10_10_2            = 0x8036,
        UNSIGNED_INT_24_8                  = 0x84fa,
        UNSIGNED_INT_2_10_10_10_REV        = 0x8368,
        UNSIGNED_INT_5_9_9_9_REV           = 0x8c3e,
        UNSIGNED_INT_8_8_8_8               = 0x8035,
        UNSIGNED_INT_8_8_8_8_REV           = 0x8367,
        UNSIGNED_INT_SAMPLER_1D            = 0x8dd1,
        UNSIGNED_INT_SAMPLER_1D_ARRAY      = 0x8dd6,
        UNSIGNED_INT_SAMPLER_2D            = 0x8dd2,
        UNSIGNED_INT_SAMPLER_2D_ARRAY      = 0x8dd7,
        UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE = 0x910a,
        UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY = 0x910d,
        UNSIGNED_INT_SAMPLER_2D_RECT       = 0x8dd5,
        UNSIGNED_INT_SAMPLER_3D            = 0x8dd3,
        UNSIGNED_INT_SAMPLER_BUFFER        = 0x8dd8,
        UNSIGNED_INT_SAMPLER_CUBE          = 0x8dd4,
        UNSIGNED_INT_VEC2                  = 0x8dc6,
        UNSIGNED_INT_VEC3                  = 0x8dc7,
        UNSIGNED_INT_VEC4                  = 0x8dc8,
        UNSIGNED_NORMALIZED                = 0x8c17,
        UNSIGNED_SHORT_1_5_5_5_REV         = 0x8366,
        UNSIGNED_SHORT_4_4_4_4_REV         = 0x8365,
        UNSIGNED_SHORT_5_6_5_REV           = 0x8364,
        UPPER_LEFT                         = 0x8ca2,
        VERTEX_ARRAY_BINDING               = 0x85b5,
        VERTEX_ATTRIB_ARRAY_INTEGER        = 0x88fd,
        VERTEX_PROGRAM_POINT_SIZE          = 0x8642,
        WAIT_FAILED_                       = 0x911d,
        WRITE_ONLY                         = 0x88b9,
        XOR                                = 0x1506,
#else
        ALIASED_POINT_SIZE_RANGE           = 0x846d,
        ALPHA_BITS                         = 0xd55,
        BLEND_EQUATION                     = 0x8009,
        BLUE_BITS                          = 0xd54,
        DEPTH_BITS                         = 0xd56,
        FIXED                              = 0x140c,
        FRAMEBUFFER_INCOMPLETE_DIMENSIONS  = 0x8cd9,
        GENERATE_MIPMAP_HINT               = 0x8192,
        GREEN_BITS                         = 0xd53,
        HIGH_FLOAT                         = 0x8df2,
        HIGH_INT                           = 0x8df5,
        IMPLEMENTATION_COLOR_READ_FORMAT   = 0x8b9b,
        IMPLEMENTATION_COLOR_READ_TYPE     = 0x8b9a,
        LOW_FLOAT                          = 0x8df0,
        LOW_INT                            = 0x8df3,
        LUMINANCE                          = 0x1909,
        LUMINANCE_ALPHA                    = 0x190a,
        MAX_FRAGMENT_UNIFORM_VECTORS       = 0x8dfd,
        MAX_VARYING_VECTORS                = 0x8dfc,
        MAX_VERTEX_UNIFORM_VECTORS         = 0x8dfb,
        MEDIUM_FLOAT                       = 0x8df1,
        MEDIUM_INT                         = 0x8df4,
        NUM_SHADER_BINARY_FORMATS          = 0x8df9,
        RED_BITS                           = 0xd52,
        RGB565                             = 0x8d62,
        SHADER_BINARY_FORMATS              = 0x8df8,
        SHADER_COMPILER                    = 0x8dfa,
        STENCIL_BITS                       = 0xd57,
#endif
    };

    extern void (CODEGEN_FUNCPTR *ActiveTexture)(GLenum texture);
    extern void (CODEGEN_FUNCPTR *AttachShader)(GLuint program, GLuint shader);
    extern void (CODEGEN_FUNCPTR *BindAttribLocation)(GLuint program, GLuint index, const GLchar * name);
    extern void (CODEGEN_FUNCPTR *BindBuffer)(GLenum target, GLuint buffer);
    extern void (CODEGEN_FUNCPTR *BindFramebuffer)(GLenum target, GLuint framebuffer);
    extern void (CODEGEN_FUNCPTR *BindRenderbuffer)(GLenum target, GLuint renderbuffer);
    extern void (CODEGEN_FUNCPTR *BindTexture)(GLenum target, GLuint texture);
    extern void (CODEGEN_FUNCPTR *BlendColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    extern void (CODEGEN_FUNCPTR *BlendEquation)(GLenum mode);
    extern void (CODEGEN_FUNCPTR *BlendEquationSeparate)(GLenum modeRGB, GLenum modeAlpha);
    extern void (CODEGEN_FUNCPTR *BlendFunc)(GLenum sfactor, GLenum dfactor);
    extern void (CODEGEN_FUNCPTR *BlendFuncSeparate)(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
    extern void (CODEGEN_FUNCPTR *BufferData)(GLenum target, GLsizeiptr size, const void * data, GLenum usage);
    extern void (CODEGEN_FUNCPTR *BufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, const void * data);
    extern GLenum (CODEGEN_FUNCPTR *CheckFramebufferStatus)(GLenum target);
    extern void (CODEGEN_FUNCPTR *Clear)(GLbitfield mask);
    extern void (CODEGEN_FUNCPTR *ClearColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    extern void (CODEGEN_FUNCPTR *ClearStencil)(GLint s);
    extern void (CODEGEN_FUNCPTR *ColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
    extern void (CODEGEN_FUNCPTR *CompileShader)(GLuint shader);
    extern void (CODEGEN_FUNCPTR *CompressedTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void * data);
    extern void (CODEGEN_FUNCPTR *CompressedTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void * data);
    extern void (CODEGEN_FUNCPTR *CopyTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
    extern void (CODEGEN_FUNCPTR *CopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
    extern GLuint (CODEGEN_FUNCPTR *CreateProgram)(void);
    extern GLuint (CODEGEN_FUNCPTR *CreateShader)(GLenum type);
    extern void (CODEGEN_FUNCPTR *CullFace)(GLenum mode);
    extern void (CODEGEN_FUNCPTR *DeleteBuffers)(GLsizei n, const GLuint * buffers);
    extern void (CODEGEN_FUNCPTR *DeleteFramebuffers)(GLsizei n, const GLuint * framebuffers);
    extern void (CODEGEN_FUNCPTR *DeleteProgram)(GLuint program);
    extern void (CODEGEN_FUNCPTR *DeleteRenderbuffers)(GLsizei n, const GLuint * renderbuffers);
    extern void (CODEGEN_FUNCPTR *DeleteShader)(GLuint shader);
    extern void (CODEGEN_FUNCPTR *DeleteTextures)(GLsizei n, const GLuint * textures);
    extern void (CODEGEN_FUNCPTR *DepthFunc)(GLenum func);
    extern void (CODEGEN_FUNCPTR *DepthMask)(GLboolean flag);
    extern void (CODEGEN_FUNCPTR *DetachShader)(GLuint program, GLuint shader);
    extern void (CODEGEN_FUNCPTR *Disable)(GLenum cap);
    extern void (CODEGEN_FUNCPTR *DisableVertexAttribArray)(GLuint index);
    extern void (CODEGEN_FUNCPTR *DrawArrays)(GLenum mode, GLint first, GLsizei count);
    extern void (CODEGEN_FUNCPTR *DrawElements)(GLenum mode, GLsizei count, GLenum type, const void * indices);
    extern void (CODEGEN_FUNCPTR *Enable)(GLenum cap);
    extern void (CODEGEN_FUNCPTR *EnableVertexAttribArray)(GLuint index);
    extern void (CODEGEN_FUNCPTR *Finish)(void);
    extern void (CODEGEN_FUNCPTR *Flush)(void);
    extern void (CODEGEN_FUNCPTR *FramebufferRenderbuffer)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
    extern void (CODEGEN_FUNCPTR *FramebufferTexture2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
    extern void (CODEGEN_FUNCPTR *FrontFace)(GLenum mode);
    extern void (CODEGEN_FUNCPTR *GenBuffers)(GLsizei n, GLuint * buffers);
    extern void (CODEGEN_FUNCPTR *GenFramebuffers)(GLsizei n, GLuint * framebuffers);
    extern void (CODEGEN_FUNCPTR *GenRenderbuffers)(GLsizei n, GLuint * renderbuffers);
    extern void (CODEGEN_FUNCPTR *GenTextures)(GLsizei n, GLuint * textures);
    extern void (CODEGEN_FUNCPTR *GenerateMipmap)(GLenum target);
    extern void (CODEGEN_FUNCPTR *GetActiveAttrib)(GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLint * size, GLenum * type, GLchar * name);
    extern void (CODEGEN_FUNCPTR *GetActiveUniform)(GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLint * size, GLenum * type, GLchar * name);
    extern void (CODEGEN_FUNCPTR *GetAttachedShaders)(GLuint program, GLsizei maxCount, GLsizei * count, GLuint * shaders);
    extern GLint (CODEGEN_FUNCPTR *GetAttribLocation)(GLuint program, const GLchar * name);
    extern void (CODEGEN_FUNCPTR *GetBooleanv)(GLenum pname, GLboolean * data);
    extern void (CODEGEN_FUNCPTR *GetBufferParameteriv)(GLenum target, GLenum pname, GLint * params);
    extern GLenum (CODEGEN_FUNCPTR *GetError)(void);
    extern void (CODEGEN_FUNCPTR *GetFloatv)(GLenum pname, GLfloat * data);
    extern void (CODEGEN_FUNCPTR *GetFramebufferAttachmentParameteriv)(GLenum target, GLenum attachment, GLenum pname, GLint * params);
    extern void (CODEGEN_FUNCPTR *GetIntegerv)(GLenum pname, GLint * data);
    extern void (CODEGEN_FUNCPTR *GetProgramInfoLog)(GLuint program, GLsizei bufSize, GLsizei * length, GLchar * infoLog);
    extern void (CODEGEN_FUNCPTR *GetProgramiv)(GLuint program, GLenum pname, GLint * params);
    extern void (CODEGEN_FUNCPTR *GetRenderbufferParameteriv)(GLenum target, GLenum pname, GLint * params);
    extern void (CODEGEN_FUNCPTR *GetShaderInfoLog)(GLuint shader, GLsizei bufSize, GLsizei * length, GLchar * infoLog);
    extern void (CODEGEN_FUNCPTR *GetShaderSource)(GLuint shader, GLsizei bufSize, GLsizei * length, GLchar * source);
    extern void (CODEGEN_FUNCPTR *GetShaderiv)(GLuint shader, GLenum pname, GLint * params);
    extern const GLubyte * (CODEGEN_FUNCPTR *GetString)(GLenum name);
    extern void (CODEGEN_FUNCPTR *GetTexParameterfv)(GLenum target, GLenum pname, GLfloat * params);
    extern void (CODEGEN_FUNCPTR *GetTexParameteriv)(GLenum target, GLenum pname, GLint * params);
    extern GLint (CODEGEN_FUNCPTR *GetUniformLocation)(GLuint program, const GLchar * name);
    extern void (CODEGEN_FUNCPTR *GetUniformfv)(GLuint program, GLint location, GLfloat * params);
    extern void (CODEGEN_FUNCPTR *GetUniformiv)(GLuint program, GLint location, GLint * params);
    extern void (CODEGEN_FUNCPTR *GetVertexAttribPointerv)(GLuint index, GLenum pname, void ** pointer);
    extern void (CODEGEN_FUNCPTR *GetVertexAttribfv)(GLuint index, GLenum pname, GLfloat * params);
    extern void (CODEGEN_FUNCPTR *GetVertexAttribiv)(GLuint index, GLenum pname, GLint * params);
    extern void (CODEGEN_FUNCPTR *Hint)(GLenum target, GLenum mode);
    extern GLboolean (CODEGEN_FUNCPTR *IsBuffer)(GLuint buffer);
    extern GLboolean (CODEGEN_FUNCPTR *IsEnabled)(GLenum cap);
    extern GLboolean (CODEGEN_FUNCPTR *IsFramebuffer)(GLuint framebuffer);
    extern GLboolean (CODEGEN_FUNCPTR *IsProgram)(GLuint program);
    extern GLboolean (CODEGEN_FUNCPTR *IsRenderbuffer)(GLuint renderbuffer);
    extern GLboolean (CODEGEN_FUNCPTR *IsShader)(GLuint shader);
    extern GLboolean (CODEGEN_FUNCPTR *IsTexture)(GLuint texture);
    extern void (CODEGEN_FUNCPTR *LineWidth)(GLfloat width);
    extern void (CODEGEN_FUNCPTR *LinkProgram)(GLuint program);
    extern void (CODEGEN_FUNCPTR *PixelStorei)(GLenum pname, GLint param);
    extern void (CODEGEN_FUNCPTR *PolygonOffset)(GLfloat factor, GLfloat units);
    extern void (CODEGEN_FUNCPTR *ReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void * pixels);
    extern void (CODEGEN_FUNCPTR *RenderbufferStorage)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
    extern void (CODEGEN_FUNCPTR *SampleCoverage)(GLfloat value, GLboolean invert);
    extern void (CODEGEN_FUNCPTR *Scissor)(GLint x, GLint y, GLsizei width, GLsizei height);
    extern void (CODEGEN_FUNCPTR *ShaderSource)(GLuint shader, GLsizei count, const GLchar *const* string, const GLint * length);
    extern void (CODEGEN_FUNCPTR *StencilFunc)(GLenum func, GLint ref, GLuint mask);
    extern void (CODEGEN_FUNCPTR *StencilFuncSeparate)(GLenum face, GLenum func, GLint ref, GLuint mask);
    extern void (CODEGEN_FUNCPTR *StencilMask)(GLuint mask);
    extern void (CODEGEN_FUNCPTR *StencilMaskSeparate)(GLenum face, GLuint mask);
    extern void (CODEGEN_FUNCPTR *StencilOp)(GLenum fail, GLenum zfail, GLenum zpass);
    extern void (CODEGEN_FUNCPTR *StencilOpSeparate)(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
    extern void (CODEGEN_FUNCPTR *TexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void * pixels);
    extern void (CODEGEN_FUNCPTR *TexParameterf)(GLenum target, GLenum pname, GLfloat param);
    extern void (CODEGEN_FUNCPTR *TexParameterfv)(GLenum target, GLenum pname, const GLfloat * params);
    extern void (CODEGEN_FUNCPTR *TexParameteri)(GLenum target, GLenum pname, GLint param);
    extern void (CODEGEN_FUNCPTR *TexParameteriv)(GLenum target, GLenum pname, const GLint * params);
    extern void (CODEGEN_FUNCPTR *TexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void * pixels);
    extern void (CODEGEN_FUNCPTR *Uniform1f)(GLint location, GLfloat v0);
    extern void (CODEGEN_FUNCPTR *Uniform1fv)(GLint location, GLsizei count, const GLfloat * value);
    extern void (CODEGEN_FUNCPTR *Uniform1i)(GLint location, GLint v0);
    extern void (CODEGEN_FUNCPTR *Uniform1iv)(GLint location, GLsizei count, const GLint * value);
    extern void (CODEGEN_FUNCPTR *Uniform2f)(GLint location, GLfloat v0, GLfloat v1);
    extern void (CODEGEN_FUNCPTR *Uniform2fv)(GLint location, GLsizei count, const GLfloat * value);
    extern void (CODEGEN_FUNCPTR *Uniform2i)(GLint location, GLint v0, GLint v1);
    extern void (CODEGEN_FUNCPTR *Uniform2iv)(GLint location, GLsizei count, const GLint * value);
    extern void (CODEGEN_FUNCPTR *Uniform3f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
    extern void (CODEGEN_FUNCPTR *Uniform3fv)(GLint location, GLsizei count, const GLfloat * value);
    extern void (CODEGEN_FUNCPTR *Uniform3i)(GLint location, GLint v0, GLint v1, GLint v2);
    extern void (CODEGEN_FUNCPTR *Uniform3iv)(GLint location, GLsizei count, const GLint * value);
    extern void (CODEGEN_FUNCPTR *Uniform4f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
    extern void (CODEGEN_FUNCPTR *Uniform4fv)(GLint location, GLsizei count, const GLfloat * value);
    extern void (CODEGEN_FUNCPTR *Uniform4i)(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
    extern void (CODEGEN_FUNCPTR *Uniform4iv)(GLint location, GLsizei count, const GLint * value);
    extern void (CODEGEN_FUNCPTR *UniformMatrix2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
    extern void (CODEGEN_FUNCPTR *UniformMatrix3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
    extern void (CODEGEN_FUNCPTR *UniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
    extern void (CODEGEN_FUNCPTR *UseProgram)(GLuint program);
    extern void (CODEGEN_FUNCPTR *ValidateProgram)(GLuint program);
    extern void (CODEGEN_FUNCPTR *VertexAttrib1f)(GLuint index, GLfloat x);
    extern void (CODEGEN_FUNCPTR *VertexAttrib1fv)(GLuint index, const GLfloat * v);
    extern void (CODEGEN_FUNCPTR *VertexAttrib2f)(GLuint index, GLfloat x, GLfloat y);
    extern void (CODEGEN_FUNCPTR *VertexAttrib2fv)(GLuint index, const GLfloat * v);
    extern void (CODEGEN_FUNCPTR *VertexAttrib3f)(GLuint index, GLfloat x, GLfloat y, GLfloat z);
    extern void (CODEGEN_FUNCPTR *VertexAttrib3fv)(GLuint index, const GLfloat * v);
    extern void (CODEGEN_FUNCPTR *VertexAttrib4f)(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    extern void (CODEGEN_FUNCPTR *VertexAttrib4fv)(GLuint index, const GLfloat * v);
    extern void (CODEGEN_FUNCPTR *VertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void * pointer);
    extern void (CODEGEN_FUNCPTR *Viewport)(GLint x, GLint y, GLsizei width, GLsizei height);
#ifdef GL_USE_ALL_AVAILABLE_EXT
#ifndef __ANDROID__
    extern void (CODEGEN_FUNCPTR *BeginConditionalRender)(GLuint id, GLenum mode);
    extern void (CODEGEN_FUNCPTR *BeginQuery)(GLenum target, GLuint id);
    extern void (CODEGEN_FUNCPTR *BeginTransformFeedback)(GLenum primitiveMode);
    extern void (CODEGEN_FUNCPTR *BindBufferBase)(GLenum target, GLuint index, GLuint buffer);
    extern void (CODEGEN_FUNCPTR *BindBufferRange)(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
    extern void (CODEGEN_FUNCPTR *BindFragDataLocation)(GLuint program, GLuint color, const GLchar * name);
    extern void (CODEGEN_FUNCPTR *BindVertexArray)(GLuint ren_array);
    extern void (CODEGEN_FUNCPTR *BlitFramebuffer)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
    extern void (CODEGEN_FUNCPTR *ClampColor)(GLenum target, GLenum clamp);
    extern void (CODEGEN_FUNCPTR *ClearBufferfi)(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
    extern void (CODEGEN_FUNCPTR *ClearBufferfv)(GLenum buffer, GLint drawbuffer, const GLfloat * value);
    extern void (CODEGEN_FUNCPTR *ClearBufferiv)(GLenum buffer, GLint drawbuffer, const GLint * value);
    extern void (CODEGEN_FUNCPTR *ClearBufferuiv)(GLenum buffer, GLint drawbuffer, const GLuint * value);
    extern void (CODEGEN_FUNCPTR *ClearDepth)(GLdouble depth);
    extern GLenum (CODEGEN_FUNCPTR *ClientWaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout);
    extern void (CODEGEN_FUNCPTR *ColorMaski)(GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
    extern void (CODEGEN_FUNCPTR *CompressedTexImage1D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void * data);
    extern void (CODEGEN_FUNCPTR *CompressedTexImage3D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void * data);
    extern void (CODEGEN_FUNCPTR *CompressedTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void * data);
    extern void (CODEGEN_FUNCPTR *CompressedTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void * data);
    extern void (CODEGEN_FUNCPTR *CopyBufferSubData)(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
    extern void (CODEGEN_FUNCPTR *CopyTexImage1D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
    extern void (CODEGEN_FUNCPTR *CopyTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
    extern void (CODEGEN_FUNCPTR *CopyTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
    extern void (CODEGEN_FUNCPTR *DeleteQueries)(GLsizei n, const GLuint * ids);
    extern void (CODEGEN_FUNCPTR *DeleteSync)(GLsync sync);
    extern void (CODEGEN_FUNCPTR *DeleteVertexArrays)(GLsizei n, const GLuint * arrays);
    extern void (CODEGEN_FUNCPTR *DepthRange)(GLdouble ren_near, GLdouble ren_far);
    extern void (CODEGEN_FUNCPTR *Disablei)(GLenum target, GLuint index);
    extern void (CODEGEN_FUNCPTR *DrawArraysInstanced)(GLenum mode, GLint first, GLsizei count, GLsizei instancecount);
    extern void (CODEGEN_FUNCPTR *DrawBuffer)(GLenum buf);
    extern void (CODEGEN_FUNCPTR *DrawBuffers)(GLsizei n, const GLenum * bufs);
    extern void (CODEGEN_FUNCPTR *DrawElementsBaseVertex)(GLenum mode, GLsizei count, GLenum type, const void * indices, GLint basevertex);
    extern void (CODEGEN_FUNCPTR *DrawElementsInstanced)(GLenum mode, GLsizei count, GLenum type, const void * indices, GLsizei instancecount);
    extern void (CODEGEN_FUNCPTR *DrawElementsInstancedBaseVertex)(GLenum mode, GLsizei count, GLenum type, const void * indices, GLsizei instancecount, GLint basevertex);
    extern void (CODEGEN_FUNCPTR *DrawRangeElements)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void * indices);
    extern void (CODEGEN_FUNCPTR *DrawRangeElementsBaseVertex)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void * indices, GLint basevertex);
    extern void (CODEGEN_FUNCPTR *Enablei)(GLenum target, GLuint index);
    extern void (CODEGEN_FUNCPTR *EndConditionalRender)(void);
    extern void (CODEGEN_FUNCPTR *EndQuery)(GLenum target);
    extern void (CODEGEN_FUNCPTR *EndTransformFeedback)(void);
    extern GLsync (CODEGEN_FUNCPTR *FenceSync)(GLenum condition, GLbitfield flags);
    extern void (CODEGEN_FUNCPTR *FlushMappedBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length);
    extern void (CODEGEN_FUNCPTR *FramebufferTexture)(GLenum target, GLenum attachment, GLuint texture, GLint level);
    extern void (CODEGEN_FUNCPTR *FramebufferTexture1D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
    extern void (CODEGEN_FUNCPTR *FramebufferTexture3D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
    extern void (CODEGEN_FUNCPTR *FramebufferTextureLayer)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
    extern void (CODEGEN_FUNCPTR *GenQueries)(GLsizei n, GLuint * ids);
    extern void (CODEGEN_FUNCPTR *GenVertexArrays)(GLsizei n, GLuint * arrays);
    extern void (CODEGEN_FUNCPTR *GetActiveUniformBlockName)(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei * length, GLchar * uniformBlockName);
    extern void (CODEGEN_FUNCPTR *GetActiveUniformBlockiv)(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint * params);
    extern void (CODEGEN_FUNCPTR *GetActiveUniformName)(GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei * length, GLchar * uniformName);
    extern void (CODEGEN_FUNCPTR *GetActiveUniformsiv)(GLuint program, GLsizei uniformCount, const GLuint * uniformIndices, GLenum pname, GLint * params);
    extern void (CODEGEN_FUNCPTR *GetBooleani_v)(GLenum target, GLuint index, GLboolean * data);
    extern void (CODEGEN_FUNCPTR *GetBufferParameteri64v)(GLenum target, GLenum pname, GLint64 * params);
    extern void (CODEGEN_FUNCPTR *GetBufferPointerv)(GLenum target, GLenum pname, void ** params);
    extern void (CODEGEN_FUNCPTR *GetBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, void * data);
    extern void (CODEGEN_FUNCPTR *GetCompressedTexImage)(GLenum target, GLint level, void * img);
    extern void (CODEGEN_FUNCPTR *GetDoublev)(GLenum pname, GLdouble * data);
    extern GLint (CODEGEN_FUNCPTR *GetFragDataLocation)(GLuint program, const GLchar * name);
    extern void (CODEGEN_FUNCPTR *GetInteger64i_v)(GLenum target, GLuint index, GLint64 * data);
    extern void (CODEGEN_FUNCPTR *GetInteger64v)(GLenum pname, GLint64 * data);
    extern void (CODEGEN_FUNCPTR *GetIntegeri_v)(GLenum target, GLuint index, GLint * data);
    extern void (CODEGEN_FUNCPTR *GetMultisamplefv)(GLenum pname, GLuint index, GLfloat * val);
    extern void (CODEGEN_FUNCPTR *GetQueryObjectiv)(GLuint id, GLenum pname, GLint * params);
    extern void (CODEGEN_FUNCPTR *GetQueryObjectuiv)(GLuint id, GLenum pname, GLuint * params);
    extern void (CODEGEN_FUNCPTR *GetQueryiv)(GLenum target, GLenum pname, GLint * params);
    extern const GLubyte * (CODEGEN_FUNCPTR *GetStringi)(GLenum name, GLuint index);
    extern void (CODEGEN_FUNCPTR *GetSynciv)(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * values);
    extern void (CODEGEN_FUNCPTR *GetTexImage)(GLenum target, GLint level, GLenum format, GLenum type, void * pixels);
    extern void (CODEGEN_FUNCPTR *GetTexLevelParameterfv)(GLenum target, GLint level, GLenum pname, GLfloat * params);
    extern void (CODEGEN_FUNCPTR *GetTexLevelParameteriv)(GLenum target, GLint level, GLenum pname, GLint * params);
    extern void (CODEGEN_FUNCPTR *GetTexParameterIiv)(GLenum target, GLenum pname, GLint * params);
    extern void (CODEGEN_FUNCPTR *GetTexParameterIuiv)(GLenum target, GLenum pname, GLuint * params);
    extern void (CODEGEN_FUNCPTR *GetTransformFeedbackVarying)(GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLsizei * size, GLenum * type, GLchar * name);
    extern GLuint (CODEGEN_FUNCPTR *GetUniformBlockIndex)(GLuint program, const GLchar * uniformBlockName);
    extern void (CODEGEN_FUNCPTR *GetUniformIndices)(GLuint program, GLsizei uniformCount, const GLchar *const* uniformNames, GLuint * uniformIndices);
    extern void (CODEGEN_FUNCPTR *GetUniformuiv)(GLuint program, GLint location, GLuint * params);
    extern void (CODEGEN_FUNCPTR *GetVertexAttribIiv)(GLuint index, GLenum pname, GLint * params);
    extern void (CODEGEN_FUNCPTR *GetVertexAttribIuiv)(GLuint index, GLenum pname, GLuint * params);
    extern void (CODEGEN_FUNCPTR *GetVertexAttribdv)(GLuint index, GLenum pname, GLdouble * params);
    extern GLboolean (CODEGEN_FUNCPTR *IsEnabledi)(GLenum target, GLuint index);
    extern GLboolean (CODEGEN_FUNCPTR *IsQuery)(GLuint id);
    extern GLboolean (CODEGEN_FUNCPTR *IsSync)(GLsync sync);
    extern GLboolean (CODEGEN_FUNCPTR *IsVertexArray)(GLuint ren_array);
    extern void (CODEGEN_FUNCPTR *LogicOp)(GLenum opcode);
    extern void * (CODEGEN_FUNCPTR *MapBuffer)(GLenum target, GLenum access);
    extern void * (CODEGEN_FUNCPTR *MapBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
    extern void (CODEGEN_FUNCPTR *MultiDrawArrays)(GLenum mode, const GLint * first, const GLsizei * count, GLsizei drawcount);
    extern void (CODEGEN_FUNCPTR *MultiDrawElements)(GLenum mode, const GLsizei * count, GLenum type, const void *const* indices, GLsizei drawcount);
    extern void (CODEGEN_FUNCPTR *MultiDrawElementsBaseVertex)(GLenum mode, const GLsizei * count, GLenum type, const void *const* indices, GLsizei drawcount, const GLint * basevertex);
    extern void (CODEGEN_FUNCPTR *PixelStoref)(GLenum pname, GLfloat param);
    extern void (CODEGEN_FUNCPTR *PointParameterf)(GLenum pname, GLfloat param);
    extern void (CODEGEN_FUNCPTR *PointParameterfv)(GLenum pname, const GLfloat * params);
    extern void (CODEGEN_FUNCPTR *PointParameteri)(GLenum pname, GLint param);
    extern void (CODEGEN_FUNCPTR *PointParameteriv)(GLenum pname, const GLint * params);
    extern void (CODEGEN_FUNCPTR *PointSize)(GLfloat size);
    extern void (CODEGEN_FUNCPTR *PolygonMode)(GLenum face, GLenum mode);
    extern void (CODEGEN_FUNCPTR *PrimitiveRestartIndex)(GLuint index);
    extern void (CODEGEN_FUNCPTR *ProvokingVertex)(GLenum mode);
    extern void (CODEGEN_FUNCPTR *ReadBuffer)(GLenum src);
    extern void (CODEGEN_FUNCPTR *RenderbufferStorageMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
    extern void (CODEGEN_FUNCPTR *SampleMaski)(GLuint maskNumber, GLbitfield mask);
    extern void (CODEGEN_FUNCPTR *TexBuffer)(GLenum target, GLenum internalformat, GLuint buffer);
    extern void (CODEGEN_FUNCPTR *TexImage1D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void * pixels);
    extern void (CODEGEN_FUNCPTR *TexImage2DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
    extern void (CODEGEN_FUNCPTR *TexImage3D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void * pixels);
    extern void (CODEGEN_FUNCPTR *TexImage3DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
    extern void (CODEGEN_FUNCPTR *TexParameterIiv)(GLenum target, GLenum pname, const GLint * params);
    extern void (CODEGEN_FUNCPTR *TexParameterIuiv)(GLenum target, GLenum pname, const GLuint * params);
    extern void (CODEGEN_FUNCPTR *TexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void * pixels);
    extern void (CODEGEN_FUNCPTR *TexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void * pixels);
    extern void (CODEGEN_FUNCPTR *TransformFeedbackVaryings)(GLuint program, GLsizei count, const GLchar *const* varyings, GLenum bufferMode);
    extern void (CODEGEN_FUNCPTR *Uniform1ui)(GLint location, GLuint v0);
    extern void (CODEGEN_FUNCPTR *Uniform1uiv)(GLint location, GLsizei count, const GLuint * value);
    extern void (CODEGEN_FUNCPTR *Uniform2ui)(GLint location, GLuint v0, GLuint v1);
    extern void (CODEGEN_FUNCPTR *Uniform2uiv)(GLint location, GLsizei count, const GLuint * value);
    extern void (CODEGEN_FUNCPTR *Uniform3ui)(GLint location, GLuint v0, GLuint v1, GLuint v2);
    extern void (CODEGEN_FUNCPTR *Uniform3uiv)(GLint location, GLsizei count, const GLuint * value);
    extern void (CODEGEN_FUNCPTR *Uniform4ui)(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
    extern void (CODEGEN_FUNCPTR *Uniform4uiv)(GLint location, GLsizei count, const GLuint * value);
    extern void (CODEGEN_FUNCPTR *UniformBlockBinding)(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
    extern void (CODEGEN_FUNCPTR *UniformMatrix2x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
    extern void (CODEGEN_FUNCPTR *UniformMatrix2x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
    extern void (CODEGEN_FUNCPTR *UniformMatrix3x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
    extern void (CODEGEN_FUNCPTR *UniformMatrix3x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
    extern void (CODEGEN_FUNCPTR *UniformMatrix4x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
    extern void (CODEGEN_FUNCPTR *UniformMatrix4x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
    extern GLboolean (CODEGEN_FUNCPTR *UnmapBuffer)(GLenum target);
    extern void (CODEGEN_FUNCPTR *VertexAttrib1d)(GLuint index, GLdouble x);
    extern void (CODEGEN_FUNCPTR *VertexAttrib1dv)(GLuint index, const GLdouble * v);
    extern void (CODEGEN_FUNCPTR *VertexAttrib1s)(GLuint index, GLshort x);
    extern void (CODEGEN_FUNCPTR *VertexAttrib1sv)(GLuint index, const GLshort * v);
    extern void (CODEGEN_FUNCPTR *VertexAttrib2d)(GLuint index, GLdouble x, GLdouble y);
    extern void (CODEGEN_FUNCPTR *VertexAttrib2dv)(GLuint index, const GLdouble * v);
    extern void (CODEGEN_FUNCPTR *VertexAttrib2s)(GLuint index, GLshort x, GLshort y);
    extern void (CODEGEN_FUNCPTR *VertexAttrib2sv)(GLuint index, const GLshort * v);
    extern void (CODEGEN_FUNCPTR *VertexAttrib3d)(GLuint index, GLdouble x, GLdouble y, GLdouble z);
    extern void (CODEGEN_FUNCPTR *VertexAttrib3dv)(GLuint index, const GLdouble * v);
    extern void (CODEGEN_FUNCPTR *VertexAttrib3s)(GLuint index, GLshort x, GLshort y, GLshort z);
    extern void (CODEGEN_FUNCPTR *VertexAttrib3sv)(GLuint index, const GLshort * v);
    extern void (CODEGEN_FUNCPTR *VertexAttrib4Nbv)(GLuint index, const GLbyte * v);
    extern void (CODEGEN_FUNCPTR *VertexAttrib4Niv)(GLuint index, const GLint * v);
    extern void (CODEGEN_FUNCPTR *VertexAttrib4Nsv)(GLuint index, const GLshort * v);
    extern void (CODEGEN_FUNCPTR *VertexAttrib4Nub)(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
    extern void (CODEGEN_FUNCPTR *VertexAttrib4Nubv)(GLuint index, const GLubyte * v);
    extern void (CODEGEN_FUNCPTR *VertexAttrib4Nuiv)(GLuint index, const GLuint * v);
    extern void (CODEGEN_FUNCPTR *VertexAttrib4Nusv)(GLuint index, const GLushort * v);
    extern void (CODEGEN_FUNCPTR *VertexAttrib4bv)(GLuint index, const GLbyte * v);
    extern void (CODEGEN_FUNCPTR *VertexAttrib4d)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    extern void (CODEGEN_FUNCPTR *VertexAttrib4dv)(GLuint index, const GLdouble * v);
    extern void (CODEGEN_FUNCPTR *VertexAttrib4iv)(GLuint index, const GLint * v);
    extern void (CODEGEN_FUNCPTR *VertexAttrib4s)(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
    extern void (CODEGEN_FUNCPTR *VertexAttrib4sv)(GLuint index, const GLshort * v);
    extern void (CODEGEN_FUNCPTR *VertexAttrib4ubv)(GLuint index, const GLubyte * v);
    extern void (CODEGEN_FUNCPTR *VertexAttrib4uiv)(GLuint index, const GLuint * v);
    extern void (CODEGEN_FUNCPTR *VertexAttrib4usv)(GLuint index, const GLushort * v);
    extern void (CODEGEN_FUNCPTR *VertexAttribI1i)(GLuint index, GLint x);
    extern void (CODEGEN_FUNCPTR *VertexAttribI1iv)(GLuint index, const GLint * v);
    extern void (CODEGEN_FUNCPTR *VertexAttribI1ui)(GLuint index, GLuint x);
    extern void (CODEGEN_FUNCPTR *VertexAttribI1uiv)(GLuint index, const GLuint * v);
    extern void (CODEGEN_FUNCPTR *VertexAttribI2i)(GLuint index, GLint x, GLint y);
    extern void (CODEGEN_FUNCPTR *VertexAttribI2iv)(GLuint index, const GLint * v);
    extern void (CODEGEN_FUNCPTR *VertexAttribI2ui)(GLuint index, GLuint x, GLuint y);
    extern void (CODEGEN_FUNCPTR *VertexAttribI2uiv)(GLuint index, const GLuint * v);
    extern void (CODEGEN_FUNCPTR *VertexAttribI3i)(GLuint index, GLint x, GLint y, GLint z);
    extern void (CODEGEN_FUNCPTR *VertexAttribI3iv)(GLuint index, const GLint * v);
    extern void (CODEGEN_FUNCPTR *VertexAttribI3ui)(GLuint index, GLuint x, GLuint y, GLuint z);
    extern void (CODEGEN_FUNCPTR *VertexAttribI3uiv)(GLuint index, const GLuint * v);
    extern void (CODEGEN_FUNCPTR *VertexAttribI4bv)(GLuint index, const GLbyte * v);
    extern void (CODEGEN_FUNCPTR *VertexAttribI4i)(GLuint index, GLint x, GLint y, GLint z, GLint w);
    extern void (CODEGEN_FUNCPTR *VertexAttribI4iv)(GLuint index, const GLint * v);
    extern void (CODEGEN_FUNCPTR *VertexAttribI4sv)(GLuint index, const GLshort * v);
    extern void (CODEGEN_FUNCPTR *VertexAttribI4ubv)(GLuint index, const GLubyte * v);
    extern void (CODEGEN_FUNCPTR *VertexAttribI4ui)(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
    extern void (CODEGEN_FUNCPTR *VertexAttribI4uiv)(GLuint index, const GLuint * v);
    extern void (CODEGEN_FUNCPTR *VertexAttribI4usv)(GLuint index, const GLushort * v);
    extern void (CODEGEN_FUNCPTR *VertexAttribIPointer)(GLuint index, GLint size, GLenum type, GLsizei stride, const void * pointer);
    extern void (CODEGEN_FUNCPTR *WaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout);
#else
    extern void (CODEGEN_FUNCPTR *ClearDepthf)(GLfloat d);
    extern void (CODEGEN_FUNCPTR *DepthRangef)(GLfloat n, GLfloat f);
    extern void (CODEGEN_FUNCPTR *GetShaderPrecisionFormat)(GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision);
    extern void (CODEGEN_FUNCPTR *ReleaseShaderCompiler)(void);
    extern void (CODEGEN_FUNCPTR *ShaderBinary)(GLsizei count, const GLuint *shaders, GLenum binaryformat, const void *binary, GLsizei length);
#endif
#endif

    namespace sys
    {

        exts::LoadTest LoadFunctions();

        int GetMinorVersion();
        int GetMajorVersion();
        bool IsVersionGEQ(int majorVersion, int minorVersion);

    } //namespace sys
} //namespace gl
#endif //POINTER_CPP_GENERATED_HEADEROPENGL_HPP
