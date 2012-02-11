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
#ifndef ROBOT_H_120209
#define ROBOT_H_120209

char const *aerobatic_2_str(enum aerobatic);
char const *maneuver_2_str(enum maneuver);
void armstate(int bot);
void newnav(int bot);
double cap(double x, double y);
void robot_safe(int bot);
void robot_autopilot(int bot);
void robotvehic(int tank);
void armstate(int bot);
void robot(int bot);

#endif
