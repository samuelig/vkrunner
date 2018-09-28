/*
 * vkrunner
 *
 * Copyright (C) 2018 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "config.h"

#include <assert.h>

#include "vr-stream.h"
#include "vr-unit-test.h"
#include "vr-temp-file.h"
#include "vr-config.h"

static const char
input[] =
        "normal line\n"
        "windows line\r\n"
        "# the next line is blank\n"
        "\n"
        "line\n"
        "\r\n"
        "split \\\n"
        "line\n"
        "split \\\r\n"
        "line\r\n"
        "backslash in middle \\o/\n"
        "\\\n"
        "\\\r\n"
        "  word\n"
        "last line no terminator";

struct expected_line {
        int lines_consumed;
        const char *data;
};

static struct expected_line
expected[] = {
        { 1, "normal line\n" },
        { 1, "windows line\r\n" },
        { 1, "# the next line is blank\n" },
        { 1, "\n" },
        { 1, "line\n" },
        { 1, "\r\n" },
        { 2, "split line\n" },
        { 2, "split line\r\n" },
        { 1, "backslash in middle \\o/\n" },
        { 3, "  word\n" },
        { 1, "last line no terminator" },
};

static void
test_stream(struct vr_stream *stream)
{
        struct vr_buffer buf = VR_BUFFER_STATIC_INIT;

        for (size_t i = 0; i < VR_N_ELEMENTS(expected); i++) {
                int lines_consumed = vr_stream_read_line(stream, &buf);
                VR_UNIT_TEST_ASSERT(lines_consumed != 0);
                VR_UNIT_TEST_ASSERT(buf.length == strlen(expected[i].data));
                VR_UNIT_TEST_ASSERT(!strcmp((const char *) buf.data,
                                            expected[i].data));
                VR_UNIT_TEST_ASSERT(lines_consumed ==
                                    expected[i].lines_consumed);
        }

        VR_UNIT_TEST_ASSERT(vr_stream_read_line(stream, &buf) == 0);
}

VR_UNIT_TEST(stream_lines,
             {
                     struct vr_stream stream;

                     vr_stream_init_string(&stream, input);

                     test_stream(&stream);

                     struct vr_config config = { .show_disassembly = false };
                     FILE *f;
                     char *filename;
                     VR_UNIT_TEST_ASSERT(vr_temp_file_create_named(&config,
                                                                   &f,
                                                                   &filename));

                     VR_UNIT_TEST_ASSERT(fwrite(input,
                                                1,
                                                (sizeof input) - 1,
                                                f) == (sizeof input) - 1);
                     rewind(f);

                     vr_stream_init_file(&stream, f);

                     test_stream(&stream);

                     fclose(f);
                     vr_free(filename);
             })
