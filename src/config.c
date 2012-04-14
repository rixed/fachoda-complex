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
#include <string.h>
#include <strings.h>    // for strcasecmp
#include <ctype.h>
#include "file.h"
#include "config.h"

/*
 * List of defined config variables
 */

static struct config_var {
    char const *name;
    char const *value;
    struct config_var *next;
} *first_var;

/*
 * Querying
 */

char const *config_get_string(char const *name, char const *dflt)
{
    for (struct config_var *var = first_var; var; var = var->next) {
        if (0 == strcasecmp(name, var->name)) return var->value;
    }
    return dflt;
}

unsigned config_get_uint(char const *name, unsigned dflt)
{
    char const *vstr = config_get_string(name, NULL);
    if (! vstr) return dflt;

    char *end;
    unsigned const v = strtoul(vstr, &end, 0);
    if (*end != '\0') {
        fprintf(stderr,
            "Cannot parse value for %s: '%s' should be an integger\n"
            "Using default of %u instead...\n", name, vstr, dflt);
        return dflt;
    }
    return v;
}

/*
 * Init
 */

static void add_var(char const *name, char const *value)
{
    struct config_var *var = malloc(sizeof(*var));
    if (! var) abort(); // not very different from what would happen next line, but just to show I care...
    var->name = strdup(name);
    var->value = strdup(value);
    var->next = first_var;
    first_var = var;
}

static int isname(int c)
{
    return isalnum(c) || c == '-' || c == '_';
}

static int isvalue(int c)
{
    return isprint(c) && c != '#' && c != '\n';
}

static int isdelim(int c)
{
    return c == '=' || c == ':';
}

void config_load(void)
{
    FILE *f = file_open_try(".fachodarc", getenv("HOME"), "r");
    if (! f) return;

    char line[1024];
    while (NULL != fgets(line, sizeof(line), f)) {

        // first non blank char is the name start
        char *name_start = line;
        while (isblank(*name_start)) name_start++;

        // last alphanum char is the name end
        char *name_stop = name_start;
        while (isname(*name_stop)) name_stop++;

        // first non blank char is =
        char *eq_start = name_stop;
        while (isblank(*eq_start)) eq_start++;

        // then accept as many = or : as there are
        char *eq_stop = eq_start;
        while (isdelim(*eq_stop)) eq_stop++;

        // then skip the blanks
        char *value_start = eq_stop;
        while (isblank(*value_start)) value_start++;

        // then everything that's not a comment nor the end of line is the value
        char *value_stop = value_start;
        while (isvalue(*value_stop)) value_stop++;
        // then chop blanks at the end
        while (value_stop > value_start && isblank(value_stop[-1])) value_stop--;

        // then we should have only blanks up to the next nul, newline or comment
        char *eol = value_stop;
        while (isblank(*eol)) eol++;

        if (*eol == '\0') {
            fprintf(stderr,
                "warning: missing end of line at the end of .fachodarc\n"
                "         (vim wouldn't have done that ;-p)\n");
        }

        // Now, check what we have caught
        if (
            name_stop > name_start &&
            eq_stop > eq_start &&
            value_stop > value_start &&
            (*eol == '\n' || *eol == '#' || *eol == '\0')
        ) {
            *name_stop = '\0';
            *value_stop = '\0';
            add_var(name_start, value_start);
        } else if (
            name_start == name_stop &&
            eq_start == eq_stop &&
            value_start == value_stop &&
            (*eol == '\n' || *eol == '#' || *eol == '\0')
        ) {
            // empty line
        } else {
            fprintf(stderr, "Cannot parse .fachodarc line '%s'\n", line);
        }
    }
}

