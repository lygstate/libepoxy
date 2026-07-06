/*
 * Copyright (C) 2026
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

/**
 * @file egl_client_extension_nodisplay.c
 *
 * Catches infinite recursion when querying EGL client extensions with no
 * display bound (eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS)).
 */

#include <stdio.h>
#include "epoxy/egl.h"

int
main(int argc, char **argv)
{
    bool has;

    (void) argc;
    (void) argv;

    has = epoxy_has_egl_extension(NULL, "EGL_EXT_platform_base");
    printf("EGL_EXT_platform_base (client): %d\n", has);

    if (has) {
        void *pfn = (void *) eglGetProcAddress("eglGetPlatformDisplayEXT");

        printf("eglGetPlatformDisplayEXT: %p\n", pfn);
    }

    if (!eglBindAPI(EGL_OPENGL_API)) {
        fprintf(stderr, "eglBindAPI failed\n");
        return 1;
    }
    printf("eglBindAPI(EGL_OPENGL_API): ok\n");

    return 0;
}
