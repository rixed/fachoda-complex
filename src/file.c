// -*- c-basic-offset: 4; c-backslash-column: 79; indent-tabs-mode: nil -*-
// vim:sw=4 ts=4 sts=4 expandtab
/* Copyright 2012
 * This file is part of Fachoda.
 *
 * Fachoda is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Fachoda is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Fachoda.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include "file.h"

FILE *file_open_try(char const *name, char const *dir, char const *mode)
{
    char path[2048];
    snprintf(path, sizeof(path), "%s/%s", dir ? dir:".", name);

    return fopen(path, mode);
}

FILE *file_open(char const *name, char const *dir, char const *mode)
{
    FILE *f = file_open_try(name, dir, mode);
    if (! f) {
        fprintf(stderr, "Cannot open %s for %s: %s\n", name, mode, strerror(errno));
    }

    return f;
}

void file_read(void *ptr, size_t size, FILE *f)
{
    clearerr(f);
    size_t ret = fread(ptr, size, 1, f);
    if (ret != 1) {
        if (feof(f)) {
            fprintf(stderr, "Cannot read %zu bytes: End Of File\n", size);
            memset(ptr, 0, size);   // FIXME: store data in a sane format that does no rely on this behavior!
        } else {
            fprintf(stderr, "Cannot read %zu bytes: %s\n", size, strerror(errno));
            abort();
        }
    }
}

