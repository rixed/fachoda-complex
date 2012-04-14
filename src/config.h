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
#ifndef CONFIG_H_120414
#define CONFIG_H_120414
/* Read only configuration from the .fachodarc file.
 * This fachodarc file is a mere list of "name = value" lines,
 * where definitions are separated by newlines, white_spaces are trimmed,
 * '#' starts a comment, and name are restricted to alphenumeric chars.
 *
 * The file is read at initialization and the config can later be queried
 * with config_get_*() functions, which all take a default value so that
 * no error are ever reported. */

char const *config_get_string(char const *name, char const *dflt);
unsigned config_get_uint(char const *name, unsigned dflt);

// Init functions
void config_load(void);

#endif
