/*
 * Copyright © 2013 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <string.h>
#include <stdio.h>

#include "dispatch_common.h"

#if USING_DISPATCH_TABLE
static bool first_context_current = false;
static bool already_switched_to_dispatch_table = false;
#endif

#if PLATFORM_HAS_EGL
/* Fallback when EGL is loaded but client EGL_VERSION is unavailable. */
#define EPOXY_EGL_MAX_CORE_VERSION 15

static int
epoxy_parse_egl_version_string(const char *version_string)
{
    int major, minor;
    int ret;

    if (!version_string)
        return 0;

    ret = sscanf(version_string, "%d.%d", &major, &minor);
    if (ret != 2)
        return 0;

    return major * 10 + minor;
}

static int
epoxy_egl_version_without_display(void)
{
    PFNEGLQUERYSTRINGPROC query_string;
    int version;

    query_string = (PFNEGLQUERYSTRINGPROC)
        epoxy_conservative_egl_dlsym("eglQueryString", false);
    if (!query_string)
        return 0;

    version = epoxy_parse_egl_version_string(
        query_string(EGL_NO_DISPLAY, EGL_VERSION));
    if (version)
        return version;

    return EPOXY_EGL_MAX_CORE_VERSION;
}
#endif /* PLATFORM_HAS_EGL */

int
epoxy_conservative_egl_version(void)
{
#if PLATFORM_HAS_EGL
    EGLDisplay dpy = NULL;
    PFNEGLGETCURRENTDISPLAYPROC get_current_display =
        (PFNEGLGETCURRENTDISPLAYPROC)
        epoxy_conservative_egl_dlsym("eglGetCurrentDisplay", false);

    if (get_current_display)
        dpy = get_current_display();

    if (!dpy)
        return epoxy_egl_version_without_display();

    return epoxy_egl_version(dpy);
#else
    return 0;
#endif
}

/**
 * @brief Returns the version of OpenGL we are using
 *
 * The version is encoded as:
 *
 * ```
 *
 *   version = major * 10 + minor
 *
 * ```
 *
 * So it can be easily used for version comparisons.
 *
 * @param The EGL display
 *
 * @return The encoded version of EGL we are using
 *
 * @see epoxy_gl_version()
 */
int
epoxy_egl_version(EGLDisplay dpy)
{
    return epoxy_parse_egl_version_string(eglQueryString(dpy, EGL_VERSION));
}

bool
epoxy_conservative_has_egl_extension(const char *ext)
{
    return epoxy_has_egl_extension(eglGetCurrentDisplay(), ext);
}

/**
 * @brief Returns true if the given EGL extension is supported in the current context.
 *
 * @param dpy The EGL display
 * @param extension The name of the EGL extension
 *
 * @return `true` if the extension is available
 *
 * @see epoxy_has_gl_extension()
 * @see epoxy_has_glx_extension()
 */
bool
epoxy_has_egl_extension(EGLDisplay dpy, const char *ext)
{
    return epoxy_extension_in_string(eglQueryString(dpy, EGL_EXTENSIONS), ext) || epoxy_extension_in_string(eglQueryString(NULL, EGL_EXTENSIONS), ext);
}

/**
 * @brief Checks whether EGL is available.
 *
 * @return `true` if EGL is available
 *
 * @newin{1,4}
 */
bool
epoxy_has_egl(void)
{
#if !PLATFORM_HAS_EGL
    return false;
#else
    if (epoxy_load_egl(false, true)) {
        EGLDisplay* (* pf_eglGetCurrentDisplay) (void);

        pf_eglGetCurrentDisplay = epoxy_conservative_egl_dlsym("eglGetCurrentDisplay", false);
        if (pf_eglGetCurrentDisplay)
            return true;
    }

    return false;
#endif /* PLATFORM_HAS_EGL */
}

void
epoxy_handle_external_eglMakeCurrent(void)
{
#if USING_DISPATCH_TABLE
    if (!first_context_current) {
        first_context_current = true;
    } else {
        if (!already_switched_to_dispatch_table) {
            already_switched_to_dispatch_table = true;
            gl_switch_to_dispatch_table();
            egl_switch_to_dispatch_table();
        }

        gl_init_dispatch_table();
        egl_init_dispatch_table();
    }
#endif
}

#if USING_DISPATCH_TABLE
WRAPPER_VISIBILITY (EGLBoolean)
WRAPPER(epoxy_eglMakeCurrent)(EGLDisplay dpy,
                              EGLSurface draw,
                              EGLSurface read,
                              EGLContext ctx)
{
    EGLBoolean ret = epoxy_eglMakeCurrent_unwrapped(dpy, draw, read, ctx);

    if (ret == EGL_TRUE)
        epoxy_handle_external_eglMakeCurrent();

    return ret;
}

PFNEGLMAKECURRENTPROC epoxy_eglMakeCurrent = epoxy_eglMakeCurrent_wrapped;
#endif
