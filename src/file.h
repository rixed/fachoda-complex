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
#ifndef FILE_H_120209
#define FILE_H_120209

#include <stdlib.h>
#include <stdio.h>

// FIXME: use unistd instead of sloppy stdio

/* Open file 'name' from dir 'dir' (or current dir if dir is NULL), or file
 * 'name' from current dir if not found. */
FILE *file_open(char const *name, char const *dir, char const *mode);

/* Same as above but do not print error message if the file can't be found. */
FILE *file_open_try(char const *name, char const *dir, char const *mode);

/* Read this size from file. Missing bytes will be filled with 0. Abort on error. */
void file_read(void *ptr, size_t size, FILE *f);

/* Write these bytes, aborting on error. */
void file_write(void *ptr, size_t size, FILE *f);

#endif
