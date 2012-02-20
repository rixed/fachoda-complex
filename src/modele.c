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
#include "proto.h"
#include "file.h"

//    NOM,                      NOM LIGHT,    PERE,PLAT,BOMB,MOBIL,PLATLIGHT
piece_s zeppelin[] = {
    { "zeppelin/Ballon",        "zeppelin/Ballon",  0,0,0,0,0 },
    { "zeppelin/MoyG",      NULL,                       0,0,0,2,0 },
    { "zeppelin/MoyD",      NULL,                       0,0,0,2,0 },
    { "zeppelin/CharnH",        NULL,                       0,0,0,2,0 },
    { "zeppelin/CharnD",        NULL,                       0,0,0,2,0 },
    { "zeppelin/CharnCan1", NULL,                       0,0,0,1,0 },
    { "zeppelin/CharnCan2", NULL,                       0,0,0,1,0 },
    { "zeppelin/CharnCan3", NULL,                       0,0,0,1,0 },
    { "zeppelin/CharnCan4", NULL,                       0,0,0,1,0 },
    { "zeppelin/CharnCan5", NULL,                       0,0,0,1,0 },
    { "zeppelin/CharnCan6", NULL,                       0,0,0,1,0 },
    { "zeppelin/Cabine",        "zeppelin/Cabine",  0,0,0,1,0 },
    { "zeppelin/FinBallon", "zeppelin/FinBallon",   0,0,0,1,0 },
    { "zeppelin/DebBallon", "zeppelin/DebBallon",   0,0,0,1,0 },
    { "zeppelin/Arc1",      NULL,                       0,0,0,1,0 },
    { "zeppelin/Arc2",      NULL,                       0,0,0,1,0 },
    { "zeppelin/HelG",      "zeppelin/HelG",        1,1,0,1,1 },
    { "zeppelin/HelD",      "zeppelin/HelD",        2,1,0,1,1 },
    { "zeppelin/InclinH",   "zeppelin/InclinH", 3,0,0,1,0 },
    { "zeppelin/InclinB",   "zeppelin/InclinB", 3,0,0,1,0 },
    { "zeppelin/InclinD",   "zeppelin/InclinD", 4,0,0,1,0 },
    { "zeppelin/InclinG",   "zeppelin/InclinG", 4,0,0,1,0 },
    { "zeppelin/GouvH",     "zeppelin/GouvH",       0,0,0,1,0 },
    { "zeppelin/GouvB",     "zeppelin/GouvB",       0,0,0,1,0 },
    { "zeppelin/GouvD",     "zeppelin/GouvD",       0,0,0,1,0 },
    { "zeppelin/GouvG",     "zeppelin/GouvG",       0,0,0,1,0 },
    { "zeppelin/Tige",      NULL,                       0,0,0,1,0 }
};

piece_s snophill[] = {
    { "snophill/Cabine",        "snoplight/Cabine", 0,0,0,0,0 },
    { "snophill/Moyeux",        NULL,   0,0,0,2,0 },
    { "snophill/CharnProf", NULL,   0,0,0,2,0 },
    { "snophill/CharnIG",   NULL,   0,0,0,2,0 },
    { "snophill/CharnID",   NULL,   0,0,0,2,0 },
    { "snophill/Gouv",      "snoplight/Gouv",   0,1,0,1,1 },
    { "snophill/Nez",           "snoplight/Nez",    0,0,0,1,0 },
    { "snophill/Queue",     "snoplight/Queue",0,0,0,1,0 },
    { "snophill/AileD1",        "snoplight/AileD1",0,0,0,1,1 },
    { "snophill/AileD2",        "snoplight/AileD2", 0,0,0,1,1 },
    { "snophill/AileG1",        "snoplight/AileG1",0,0,0,1,1 },
    { "snophill/AileG2",        "snoplight/AileG2", 0,0,0,1,1 },
    { "snophill/AileSup",   "snoplight/AileSup",    0,0,0,1,1 },
    { "snophill/AileSupD",  "snoplight/AileSupD",   0,0,0,1,1 },
    { "snophill/AileSupG",  "snoplight/AileSupG",   0,0,0,1,1 },
    { "snophill/Tube1",     "snoplight/Tube1",  0,0,0,1,1 },
    { "snophill/Tube2",     "snoplight/Tube2",  0,0,0,1,1 },
    { "snophill/Tube3",     NULL,   0,0,0,1,1 },
    { "snophill/Tube4",     "snoplight/Tube4",  0,0,0,1,1 },
    { "snophill/Tube5",     "snoplight/Tube5",  0,0,0,1,1 },
    { "snophill/Tube6",     NULL,   0,0,0,1,1 },
    { "snophill/PRD1",      "snoplight/PRD",    0,0,0,1,1 },
    { "snophill/PRD2",      NULL,   0,0,0,1,1 },
    { "snophill/PRG1",      "snoplight/PRG",    0,0,0,1,1 },
    { "snophill/PRG2",      NULL,   0,0,0,1,1 },
    { "snophill/PRA",           "snoplight/PRA",    0,0,0,1,1 },
    { "snophill/RoueD",     "snoplight/RoueD",  0,0,0,1,1 },
    { "snophill/RoueG",     "snoplight/RoueG",  0,0,0,1,1 },
    { "snophill/RoueA",     NULL,   0,0,0,1,1 },
    { "snophill/StabD",     "snoplight/StabD",  0,0,0,1,1 },
    { "snophill/StabG",     "snoplight/StabG",  0,0,0,1,1 },
    { "snophill/TabBord",   NULL,   0,0,0,1,0 },
    { "snophill/Canon",     NULL,   0,0,0,1,0 },
    { "snophill/Mitrailleuse",  NULL, 0,0,0,1,0 },
    { "snophill/ProfG",     NULL,   2,0,0,1,0 },
    { "snophill/ProfD",     NULL,   2,0,0,1,0 },
    { "snophill/InclinG",   NULL,   3,0,0,1,0 },
    { "snophill/InclinD",   NULL,   4,0,0,1,0 },
    { "snophill/Hel1",      "snoplight/Hel1",   1,1,0,1,1 },
    { "snophill/Hel2",      "snoplight/Hel2",   1,1,0,1,1 },
    { "snophill/Rebord",        NULL, 0,0,0,1,0 },
    { "snophill/Tete",      NULL, 0,0,0,1,0 },
    { "snophill/LunG",      NULL, 0,0,0,1,0 },
    { "snophill/LunD",      NULL, 0,0,0,1,0 },
    { "snophill/PosCam",        NULL,   0,0,0,1,0 }
};
piece_s spitflame[] = {
    { "spitflame/Cabine",       "spitflamelight/Cabine",    0,0,0,0,0 },
    { "spitflame/Moyeux",       NULL,   0,0,0,2,0 },
    { "spitflame/CharnProf",    NULL,   0,0,0,2,0 },
    { "spitflame/CharnInclinG",NULL,    0,0,0,2,0 },
    { "spitflame/CharnInclinD",NULL,    0,0,0,2,0 },
    { "spitflame/B0",               NULL,   0,0,1,1,0 },
    { "spitflame/B2",               NULL,   0,0,1,1,0 },
    { "spitflame/B1",               NULL,   0,0,1,1,0 },
    { "spitflame/B3",               NULL,   0,0,1,1,0 },
    { "spitflame/B4",               NULL,   0,0,1,1,0 },
    { "spitflame/B5",               NULL,   0,0,1,1,0 },
    { "spitflame/B6",               NULL,   0,0,1,1,0 },
    { "spitflame/Hel1",         "spitflamelight/Hel1",  1,1,0,1,1 },
    { "spitflame/Hel2",         "spitflamelight/Hel2",  1,1,0,1,1 },
    { "spitflame/Hel3",         "spitflamelight/Hel3",  1,1,0,1,1 },
    { "spitflame/Gouv",         "spitflamelight/Gouv",  0,0,0,1,1 },
    { "spitflame/FuselageAr",   "spitflamelight/FuselageAr",    0,0,0,1,0 },
    { "spitflame/FuselageAv",   "spitflamelight/FuselageAv",    0,0,0,1,0 },
    { "spitflame/Capot",            "spitflamelight/Capot", 0,0,0,1,0 },
    { "spitflame/AileD1",       "spitflamelight/AileD1",    0,0,0,1,1 },
    { "spitflame/AileD2",       "spitflamelight/AileD2",    0,0,0,1,1 },
    { "spitflame/AileG1",       "spitflamelight/AileG1",    0,0,0,1,1 },
    { "spitflame/AileG2",       "spitflamelight/AileG2",    0,0,0,1,1 },
    { "spitflame/PiedRoues",    "spitflamelight/PiedRoues", 0,1,0,1,1 },
    { "spitflame/RoueD",            "spitflamelight/RoueD", 0,0,0,1,1 },
    { "spitflame/RoueG",            "spitflamelight/RoueG", 0,0,0,1,1 },
    { "spitflame/RoueAr",       "spitflamelight/RoueAr",    0,0,0,1,1 },
    { "spitflame/StabD",            "spitflamelight/StabD", 0,0,0,1,1 },
    { "spitflame/StabG",            "spitflamelight/StabG", 0,0,0,1,1 },
    { "spitflame/TabBord",      NULL,   0,0,0,1,0 },
    { "spitflame/Canon1",       NULL,   0,0,0,1,0 },
    { "spitflame/Canon2",       NULL,   0,0,0,1,0 },
    { "spitflame/Canon3",       NULL,   0,0,0,1,0 },
    { "spitflame/Canon4",       NULL,   0,0,0,1,0 },
    { "spitflame/ProfG",            NULL, 2,0,0,1,0 },
    { "spitflame/ProfD",            NULL, 2,0,0,1,0 },
    { "spitflame/InclinG",      NULL, 3,0,0,1,0 },
    { "spitflame/InclinD",      NULL, 4,0,0,1,0 },
    { "spitflame/B0L",          NULL, 5,0,0,1,0 },
    { "spitflame/B1L",          NULL, 7,0,0,1,0 },
    { "spitflame/B2L",          NULL, 6,0,0,1,0 },
    { "spitflame/B3L",          NULL, 8,0,0,1,0 },
    { "spitflame/B4L",          NULL, 9,0,0,1,0 },
    { "spitflame/B5L",          NULL, 10,0,0,1,0 },
    { "spitflame/B6L",          NULL, 11,0,0,1,0 },
    { "spitflame/Verriere",     "spitflamelight/Verriere", 0,0,0,1,0 },
    { "spitflame/PosCam",       NULL, 0,0,0,1,0 }
};

piece_s piecenaviontoy[] = {
    { "toyvion/Cabine",     "toyvionlight/Cabine",  0,0,0,0,0 },
    { "toyvion/Moyeux",     "toyvionlight/Moyeux",  0,0,0,2,0 },
    { "toyvion/CharnProf",  NULL,   0,0,0,2,0 },
    { "toyvion/CharnG",     NULL,   0,0,0,2,0 },
    { "toyvion/CharnD",     NULL,   0,0,0,2,0 },
    { "toyvion/Bomb1",      "toyvionlight/Bomb1",   0,0,1,1,0 },
    { "toyvion/Bomb2",      "toyvionlight/Bomb2",   0,0,1,1,0 },
    { "toyvion/Bomb3",      "toyvionlight/Bomb3",   0,0,1,1,0 },
    { "toyvion/Hel1",           "toyvionlight/Hel1",    1,1,0,1,1 },
    { "toyvion/Hel2",           "toyvionlight/Hel2",    1,1,0,1,1 },
    { "toyvion/Gouv",           "toyvionlight/Gouv",    0,0,0,1,1 },
    { "toyvion/Queue",      "toyvionlight/Queue",   0,0,0,1,0 },
    { "toyvion/Capot",      "toyvionlight/Capot",   0,0,0,1,0 },
    { "toyvion/AileD",      "toyvionlight/AileD",   0,0,0,1,1 },
    { "toyvion/AileG",      "toyvionlight/AileG",   0,0,0,1,1 },
    { "toyvion/PiedRD",     "toyvionlight/PiedRD",  0,0,0,1,0 },
    { "toyvion/PiedRG",     "toyvionlight/PiedRG",  0,0,0,1,0 },
    { "toyvion/RoueD",      "toyvionlight/RoueD",   0,0,0,1,0 },
    { "toyvion/RoueG",      "toyvionlight/RoueG",   0,0,0,1,0 },
    { "toyvion/RoueA",      NULL,   0,0,0,1,0 },
    { "toyvion/StabD",      "toyvionlight/StabD",   0,0,0,1,1 },
    { "toyvion/StabG",      "toyvionlight/StabG",   0,0,0,1,1 },
    { "toyvion/PiedRA",     NULL,   0,0,0,1,0 },
    { "toyvion/TabBord",        NULL,   0,0,0,1,0 },
    { "toyvion/Canon",      NULL,   0,0,0,1,0 },
    { "toyvion/VProfG",     NULL,   2,0,0,1,0 },
    { "toyvion/VProfD",     NULL,   2,0,0,1,0 },
    { "toyvion/VolG",           NULL,   3,0,0,1,0 },
    { "toyvion/VolD",           NULL,   4,0,0,1,0 },
    { "toyvion/Bomb1Q",     NULL,   5,0,0,1,0 },
    { "toyvion/Bomb2Q",     NULL,   6,0,0,1,0 },
    { "toyvion/Bomb3Q",     NULL,   7,0,0,1,0 },
    { "toyvion/Bomb1A1",        "toyvionlight/Bomb1A1", 5,1,0,1,1 },
    { "toyvion/Bomb1A2",        "toyvionlight/Bomb1A2", 5,1,0,1,1 },
    { "toyvion/Bomb1A3",        "toyvionlight/Bomb1A3", 5,1,0,1,1 },
    { "toyvion/Bomb2A1",        "toyvionlight/Bomb2A1", 6,1,0,1,1 },
    { "toyvion/Bomb2A2",        "toyvionlight/Bomb2A2", 6,1,0,1,1 },
    { "toyvion/Bomb2A3",        "toyvionlight/Bomb2A3", 6,1,0,1,1 },
    { "toyvion/Bomb3A1",        "toyvionlight/Bomb3A1", 7,1,0,1,1 },
    { "toyvion/Bomb3A2",        "toyvionlight/Bomb3A2", 7,1,0,1,1 },
    { "toyvion/Bomb3A3",        "toyvionlight/Bomb3A3", 7,1,0,1,1 },
    { "toyvion/Tube1",      NULL, 0,0,0,1,0 },
    { "toyvion/Tube2",      NULL, 0,0,0,1,0 },
    { "toyvion/Tube3",      NULL, 0,0,0,1,0 },
    { "toyvion/Verierre",   "toyvionlight/Verierre",    0,0,0,1,0 },
    { "toyvion/PosCam",     NULL,   0,0,0,1,0 }
};
piece_s piecenavion3[] = {
    { "corsair/Cabine",               "corsairlight/Cabine",                     0,0,0,0,0 },
    { "corsair/MoyeuPale",            "corsairlight/MoyeuPale",                  0,0,0,2,0 },
    { "corsair/CharniereGauche",      "corsairlight/CharniereGauche",            0,0,0,2,0 },
    { "corsair/CharniereDroite",      "corsairlight/CharniereDroite",            0,0,0,2,0 },
    { "corsair/CharnProf",            NULL,                                    0,0,0,2,0 },
    { "corsair/CharnInclinG",         NULL,                                    0,0,0,2,0 },
    { "corsair/CharnInclinD",         NULL,                                    0,0,0,2,0 },
    { "corsair/HBomb4",               "corsairlight/HBomb4",                     0,0,1,1,0 },
    { "corsair/HBomb1",               "corsairlight/HBomb1",                     0,0,1,1,0 },
    { "corsair/HBomb2",               "corsairlight/HBomb2",                     0,0,1,1,0 },
    { "corsair/HBomb3",               "corsairlight/HBomb3",                     0,0,1,1,0 },
    { "corsair/Pale1",                "corsairlight/Pale1",                      1,1,0,1,1 },
    { "corsair/Pale2",                "corsairlight/Pale2",                      1,1,0,1,1 },
    { "corsair/Pale3",                "corsairlight/Pale3",                      1,1,0,1,1 },
    { "corsair/Gouvernail",           "corsairlight/Gouvernail",                 0,0,0,1,1 },
    { "corsair/FuselageArriere",      "corsairlight/FuselageArriere",            0,0,0,1,0 },
    { "corsair/FuselageAvant",        "corsairlight/FuselageAvant",              0,0,0,1,0 },
    { "corsair/AileD1",               "corsairlight/AileD1",                     0,0,0,1,1 },
    { "corsair/AileG1",               "corsairlight/AileG1",                     0,0,0,1,1 },
    { "corsair/AileG2",               NULL,                                    0,0,0,1,0 },
    { "corsair/AileD2",               NULL,                                    0,0,0,1,0 },
    { "corsair/EssieuDroit",          "corsairlight/EssieuD",                    3,0,0,1,1 },
    { "corsair/EssieuGauche",         "corsairlight/EssieuG",                    2,0,0,1,1 },
    { "corsair/RoueArriere",                "corsairlight/RoueArriere",                 0,0,0,1,1 },
    { "corsair/RoueDroite",             "corsairlight/RoueD",                           3,0,0,1,1 },
    { "corsair/RoueGauche",            "corsairlight/RoueG",                            2,0,0,1,1 },
    { "corsair/AileD3",                     "corsairlight/AileD3",                          0,0,0,1,1 },
    { "corsair/AileG3","corsairlight/AileG3", 0,0,0,1,1},
    { "corsair/AileD4",NULL, 0,0,0,1,0 },
    { "corsair/AileG4",NULL, 0,0,0,1,0 },
    { "corsair/AileronG","corsairlight/AileronG", 0,0,0,1,1 },
    { "corsair/AileronD","corsairlight/AileronD", 0,0,0,1,1 },
    { "corsair/HBomb1A1","corsairlight/HBomb1A1", 8,1,0,1,1 },
    { "corsair/HBomb1A2","corsairlight/HBomb1A2", 8,1,0,1,1 },
    { "corsair/HBomb1A3","corsairlight/HBomb1A3", 8,1,0,1,1 },
    { "corsair/HBomb1A4",NULL, 8,1,0,1,1 },
    { "corsair/HBomb2A1","corsairlight/HBomb2A1", 9,1,0,1, 1 },
    { "corsair/HBomb2A2","corsairlight/HBomb2A2", 9,1,0,1, 1 },
    { "corsair/HBomb2A3","corsairlight/HBomb2A3", 9,1,0,1, 1 },
    { "corsair/HBomb2A4",NULL, 9,1,0,1,1 },
    { "corsair/HBomb3A1","corsairlight/HBomb3A1", 10,1,0,1, 1 },
    { "corsair/HBomb3A2","corsairlight/HBomb3A2", 10,1,0,1, 1 },
    { "corsair/HBomb3A3","corsairlight/HBomb3A3", 10,1,0,1, 1 },
    { "corsair/HBomb3A4",NULL, 10,1,0,1,1 },
    { "corsair/HBomb4A1","corsairlight/HBomb4A1", 7,1,0,1, 1 },
    { "corsair/HBomb4A2","corsairlight/HBomb4A2", 7,1,0,1, 1 },
    { "corsair/HBomb4A3","corsairlight/HBomb4A3", 7,1,0,1, 1 },
    { "corsair/HBomb4A4",NULL, 7,1,0,1,1 },
    { "corsair/PiedArriere","corsairlight/PiedArriere", 0,0,0,1,1 },
    { "corsair/TabBord",NULL, 0,0,0,1,0 },
    { "corsair/Canon1","corsairlight/Canon1", 0,0,0,1,1 },
    { "corsair/Canon3","corsairlight/Canon3", 0,0,0,1,1 },
    { "corsair/Canon2","corsairlight/Canon2", 0,0,0,1,1 },
    { "corsair/Canon4","corsairlight/Canon3", 0,0,0,1,1 },
    { "corsair/VolProfG",NULL, 4,0,0,1,0 },
    { "corsair/VolProfD",NULL, 4,0,0,1,0 },
    { "corsair/VolInclinG",NULL, 5,0,0,1,0 },
    { "corsair/VolInclinD",NULL, 6,0,0, 1,0 },
    { "corsair/Echapement",NULL, 0,0,0, 1,0 },
    { "corsair/Verriere","corsairlight/Verriere", 0,0,0,1,0 },
    { "corsair/PosCam",NULL, 0,0,0,1,0 },
};
piece_s piecenavion2[] = {
    { "d501/Cockpit",           "d501light/Cockpit",                 0,0,0,0,0 },
    { "d501/Moyeu",         "d501light/Moyeu",                   0,0,0,2,0 },
    { "d501/CharnVProf",        NULL,            0,0,0,2,0 },
    { "d501/CharniereIG",   NULL,            0,0,0,2,0 },
    { "d501/CharniereID",   NULL,            0,0,0,2,0 },
    { "d501/HB0",               "d501light/HB0",                         0,0,1,1,0 },
    { "d501/HB1",               "d501light/HB1",                         0,0,1,1,0 },
    { "d501/RoueD",         "d501light/RoueD",                   0,0,0,1,1 },
    { "d501/RoueG",         "d501light/RoueG",                   0,0,0,1,1 },
    { "d501/RoueArriere",   "d501light/RoueArriere",             0,0,0,1,1 },
    { "d501/HB0Queue",      NULL,                5,0,0,1,0 },
    { "d501/HB1Queue",      NULL,                6,0,0,1,0 },
    { "d501/HB0a1",         "d501light/HB0a1",                   5,1,0,1,1 },
    { "d501/HB0a2",         "d501light/HB0a2",                   5,1,0,1,1 },
    { "d501/HB0a3",         "d501light/HB0a3",                   5,1,0,1,1 },
    { "d501/HB0a4",         "d501light/HB0a4",                   5,1,0,1,1 },
    { "d501/HB1a1",         "d501light/HB1a1",                   6,1,0,1,1 },
    { "d501/HB1a2",         "d501light/HB1a2",                   6,1,0,1,1 },
    { "d501/HB1a3",         "d501light/HB1a3",                   6,1,0,1,1 },
    { "d501/HB1a4",         "d501light/HB1a4",                   6,1,0,1,1 },
    { "d501/Helice1",           "d501light/Helice1",                     1,1,0,1,1 },
    { "d501/Helice2",           "d501light/Helice2",                     1,1,0,1,1 },
    { "d501/Fuselage",      "d501light/Fuselage",                0,0,0,1,0 },
    { "d501/FinFuselage",   "d501light/FinFuselage",             0,0,0,1,0 },
    { "d501/Capot",         "d501light/Capot",                   0,0,0,1,0 },
    { "d501/Empenage",      "d501light/Empenage",                0,1,0,1,1 },
    { "d501/TubeD1",            "d501light/TubeD1",                  0,0,0,1,1 },
    { "d501/TubeD2",            "d501light/TubeD2",                  0,0,0,1,1 },
    { "d501/TubeD3",            "d501light/TubeD3",                  0,0,0,1,1 },
    { "d501/TubeG1",            "d501light/TubeG1",                  0,0,0,1,1 },
    { "d501/TubeG2",            "d501light/TubeG2",                  0,0,0,1,1 },
    { "d501/TubeG3",            "d501light/TubeG3",                  0,0,0,1,1 },
    { "d501/CacheRoueD",        "d501light/CacheRoueD",              0,0,0,1,1 },
    { "d501/CacheRoueG",        "d501light/CacheRoueG",              0,0,0,1,1 },
    { "d501/AileD1",            "d501light/AileD1",                  0,0,0,1,1 },
    { "d501/AileD2",            NULL,                    0,0,0,1,0 },
    { "d501/AileD3",            NULL,                    0,0,0,1,0 },
    { "d501/AileD4",            NULL,                    0,0,0,1,0 },
    { "d501/AileG1",            "d501light/AileG1",                  0,0,0,1,1 },
    { "d501/AileG2",            NULL,                    0,0,0,1,0 },
    { "d501/AileG3",            NULL,                    0,0,0,1,0 },
    { "d501/AileG4",            NULL,                    0,0,0,1,0 },
    { "d501/AileronD",      "d501light/AileronD",                0,0,0,1,1 },
    { "d501/AileronG",      "d501light/AileronG",                0,0,0,1,1 },
    { "d501/VProfG",            NULL,                    2,0,0,1,0 },
    { "d501/VProfD",            NULL,                    2,0,0,1,0 },
    { "d501/InclinG",           NULL,                3,0,0,1,0 },
    { "d501/InclinD",           NULL,                4,0,0,1,0 },
    { "d501/Gouvernail",        "d501light/Gouvernail",          0,1,0,1,1 },
    { "d501/RouePied",      "d501light/RouePied",                0,0,0,1,1 },
    { "d501/TabBord",           NULL,                0,0,0,1,0 },
    { "d501/MitSocleG",     "d501light/MitSocleG",               0,0,0,1,1 },
    { "d501/MitSocleD",     "d501light/MitSocleD",               0,0,0,1,1 },
    { "d501/MitrailleuseG", "d501light/MitrailleuseG",       0,0,0,1,0 },
    { "d501/MitrailleuseD", "d501light/MitrailleuseD",       0,0,0,1,0 },
    { "d501/CanonG",            "d501light/CanonG",                  0,0,0,1,1 },
    { "d501/CanonD",            "d501light/CanonD",                  0,0,0,1,1 },
    { "d501/ViseurG",           NULL,                0,0,0,1,0 },
    { "d501/ViseurD",           NULL,                0,0,0,1,0 },
    { "d501/Casque",            NULL,                    0,0,0,1,0 },
    { "d501/LunetteD",      NULL,                0,0,0,1,0 },
    { "d501/LunetteG",      NULL,                0,0,0,1,0 },
    { "d501/Tete",          "d501light/Tete",                    0,0,0,1,0 },
    { "d501/Masque",            NULL,                    0,0,0,1,0 },
    { "d501/PosCam",            NULL,                    0,0,0,1,0 },
};

piece_s piecenavion4[] = {
    { "moshito/Cabine",             "moshitolight/Cabine",                   0,0,0,0,0 },
    { "moshito/MoyeuG",             "moshitolight/MoyeuG",                   0,0,0,2,0 },
    { "moshito/MoyeuD",             "moshitolight/MoyeuD",                   0,0,0,2,0 },
    { "moshito/CharniereG",     "moshitolight/CharniereG",           0,0,0,2,0 },
    { "moshito/CharniereD",     "moshitolight/CharniereD",           0,0,0,2,0 },
    { "moshito/CharniereSG",        NULL,            0,0,0,2,0 },
    { "moshito/CharniereSD",        NULL,            0,0,0,2,0 },
    { "moshito/CharniereA",     "moshitolight/CharniereA",           0,0,0,2,0 },
    { "moshito/CharnProf",          NULL,                0,0,0,2,0 },
    { "moshito/CharnInclinG",       NULL,            0,0,0,2,0 },
    { "moshito/CharnInclinD",       NULL,            0,0,0,2,0 },
    { "moshito/HB4",                    "moshitolight/HB3",                      0,0,1,1,0 },   // 11
    { "moshito/HB1",                    "moshitolight/HB1",                      0,0,1,1,0 },   // 12
    { "moshito/HB2",                    "moshitolight/HB2",                      0,0,1,1,0 },
    { "moshito/HB3",                    "moshitolight/HB3",                      0,0,1,1,0 },
    { "moshito/PaleG1",             "moshitolight/PaleG1",                   1,1,0,1,1 },
    { "moshito/PaleG2",             "moshitolight/PaleG2",                   1,1,0,1,1 },
    { "moshito/PaleG3",             "moshitolight/PaleG3",                   1,1,0,1,1 },
    { "moshito/PaleD1",             "moshitolight/PaleD1",                   2,1,0,1,1 },
    { "moshito/PaleD2",             "moshitolight/PaleD2",                   2,1,0,1,1 },
    { "moshito/PaleD3",             "moshitolight/PaleD3",                   2,1,0,1,1 },
    { "moshito/FuselageAvant",      "moshitolight/FuselageAvant",        0,0,0,1,0 },
    { "moshito/FuselageArriere",    NULL,        0,0,0,1,0 },
    { "moshito/Queue",              "moshitolight/Queue",                    0,0,0,1,1 },
    { "moshito/GouvG",              "moshitolight/GouvG",                    0,0,0,1,1 },
    { "moshito/GouvD",              "moshitolight/GouvD",                    0,0,0,1,1 },
    { "moshito/AileD1",             "moshitolight/AileD1",                   0,0,0,1,1 },
    { "moshito/AileD2",             NULL,    0,0,0,1,0 },
    { "moshito/AileD3",             NULL,    0,0,0,1,0 },
    { "moshito/AileD4",             "moshitolight/AileD4",                   0,0,0,1,1 },
    { "moshito/AileG1",             "moshitolight/AileG1",                   0,0,0,1,1 },
    { "moshito/AileG2",             NULL,                    0,0,0,1,0 },
    { "moshito/AileG3",             NULL,                    0,0,0,1,0 },
    { "moshito/AileG4",             "moshitolight/AileG4",                   0,0,0,1,1 },
    { "moshito/MoteurD",            "moshitolight/MoteurD",              0,0,0,1,0 },
    { "moshito/MoteurG",            "moshitolight/MoteurG",              0,0,0,1,0 },
    { "moshito/EssieuAvant",        "moshitolight/EssieuAvant",          7,0,0,1,1 },
    { "moshito/MachinRoueAvant",    NULL,        7,1,0,1,0 },
    { "moshito/RoueAvant",          "moshitolight/RoueAvant",                7,0,0,1,1 },
    { "moshito/SasAvantD",          NULL,                6,0,0,1,0 },
    { "moshito/SasAvantG",          NULL,                5,0,0,1,0 },
    { "moshito/EssieuD",            "moshitolight/EssieuD",              4,0,0,1,1 },
    { "moshito/RoueD",              "moshitolight/RoueD",                    4,0,0,1,1 },
    { "moshito/EssieuG",            "moshitolight/EssieuG",              3,0,0,1,1 },
    { "moshito/RoueG",              "moshitolight/RoueG",                    3,0,0,1,1 },
    { "moshito/TabBord",            NULL,                0,0,0,1,0 },
    { "moshito/Canon1",             "moshitolight/Canon1",                   0,0,0,1,1 },
    { "moshito/Canon2",             "moshitolight/Canon2",                   0,0,0,1,1 },
    { "moshito/Canon3",             "moshitolight/Canon3",                   0,0,0,1,1 },
    { "moshito/Canon4",             "moshitolight/Canon4",                   0,0,0,1,1 },
    { "moshito/VProf",              NULL,                    8,0,0,1,0 },
    { "moshito/VInclinG",           NULL,                9,0,0,1,0 },
    { "moshito/VInclinD",           NULL,               10,0,0,1,0 },
    { "moshito/Tube",               NULL,                    0,0,0,1,0 },
    { "moshito/Poignee",            NULL,                0,0,0,1,0 },
    { "moshito/Echap",              NULL,                    0,0,0,1,0 },
    { "moshito/HB1A1",              NULL,                    12,1,0,1,0 },
    { "moshito/HB1A2",              NULL,                    12,1,0,1,0 },
    { "moshito/HB1A3",              NULL,                    12,1,0,1,0 },
    { "moshito/HB2A1",              NULL,                    13,1,0,1,0 },
    { "moshito/HB2A2",              NULL,                    13,1,0,1,0 },
    { "moshito/HB2A3",              NULL,                    13,1,0,1,0 },
    { "moshito/HB3A1",              NULL,                    14,1,0,1,0 },
    { "moshito/HB3A2",              NULL,                    14,1,0,1,0 },
    { "moshito/HB3A3",              NULL,                    14,1,0,1,0 },
    { "moshito/HB4A1",              NULL,                    11,1,0,1,0 },
    { "moshito/HB4A2",              NULL,                    11,1,0,1,0 },
    { "moshito/HB4A3",              NULL,                    11,1,0,1,0 },
    { "moshito/Verriere",           "moshitolight/Verriere",                 0,0,0,1,0 },
    { "moshito/PosCam",             NULL,                                0,0,0,1,0 },
};

piece_s piecepiste[] = {
    { "base1/Mark1",                NULL,                    0,0,0,-1,0 },
    { "base1/Mark2",                NULL,                    0,0,0,-1,0 },
    { "base1/Mark3",                NULL,                    0,0,0,-1,0 },
    { "base1/Mark4",                NULL,                    0,0,0,-1,0 },
    { "base1/Mark5",                NULL,                    0,0,0,-1,0 },
    { "base1/Mark6",                NULL,                    0,0,0,-1,0 },
    { "base1/Mark7",                NULL,                    0,0,0,-1,0 },
    { "base1/Mark8",                NULL,                    0,0,0,-1,0 },
    { "base1/Mark9",                NULL,                    0,0,0,-1,0 },
    { "base1/Mark10",               NULL,                                0,0,0,-1,0 },
    { "base1/Mark11",               NULL,                                0,0,0,-1,0 },
    { "base1/Mark12",               NULL,                                0,0,0,-1,0 },
    { "base1/Mark13",               NULL,                                0,0,0,-1,0 },
    { "base1/Mark14",               NULL,                                0,0,0,-1,0 },
    { "base1/Hangar1",          "base1light/Hangar1",        0,0,3,-1,0 },
    { "base1/Hangar2",          "base1light/Hangar2",        0,0,3,-1,0 },
    { "base1/Huit",             NULL,                                0,0,0,-1,0 },
    { "base1/Un",                   NULL,                                0,0,0,-1,0 },
    { "base1/MancheAAir",       "base1/MancheAAir",          0,0,0,-1,0 },
    { "base1/PiedMancheAAir",   "base1/PiedMancheAAir",      0,0,0,-1,0 },
    { "base1/Repere1",          "base1/Repere1",                 0,1,0,-1,0 },
    { "base1/Repere2",          "base1/Repere2",                 0,1,0,-1,0 },
    { "base1/Light1",               "base1/Light1",             0,0,2,-1,0 },
    { "base1/Light2",               "base1/Light2",             0,0,2,-1,0 },
    { "base1/Light3",               "base1/Light3",             0,0,2,-1,0 },
    { "base1/Light4",               "base1/Light4",             0,0,2,-1,0 },
    { "base1/Light5",               "base1/Light5",             0,0,2,-1,0 },
    { "base1/Light6",               "base1/Light6",             0,0,2,-1,0 },
    { "base1/Light7",               "base1/Light7",             0,0,2,-1,0 },
    { "base1/Light8",               "base1/Light8",             0,0,2,-1,0 },
    { "base1/Light9",               "base1/Light9",             0,0,2,-1,0 },
    { "base1/Light10",          "base1/Light10",                0,0,2,-1,0 },
    { "base1/Light11",          "base1/Light11",                0,0,2,-1,0 },
    { "base1/Light12",          "base1/Light12",                0,0,2,-1,0 },
    { "base1/Light13",          "base1/Light13",                0,0,2,-1,0 },
    { "base1/Light14",          "base1/Light14",                0,0,2,-1,0 },
};
piece_s piecetour[] = {
    { "base1/Tour1",    "base1/Tour1",  0,0,3,-1,0 },
    { "base1/Tour2",    "base1/Tour2",  0,0,0,-1,0 }
};
piece_s piecemaison0[] = {
    { "maison/Murs",                "maison/Murs",   0,0,3,-1,0 },
    { "maison/Toit",                "maison/Toit",   0,0,3,-1,0 },
    { "maison/Cheminee2",       NULL,                0,0,0,-1,0 },
    { "maison/Cheminee",            NULL,                0,0,3,-1,0 },
    { "maison/Fenetre",         NULL,                0,0,3,-1,0 },
    { "maison/Fenetre2",            NULL,                0,0,3,-1,0 },
    { "maison/Porte",               NULL,                0,0,3,-1,0 },
    { "maison/Rebord",          NULL,                0,0,3,-1,0 }
};
piece_s reverbere[] = {
    {"reverbere/Pied",          "reverbere/Pied",   0,0,3,-1,0 },
    {"reverbere/Tete",          "reverbere/Tete",   0,0,2,-1,0 }
};
piece_s pieceeglise[] = {
    { "eglise/Nef2",            "egliselight/Nef",          0,0,3,-1,0 },
    { "eglise/Nef1",            NULL,                               0,0,3,-1,0 },
    { "eglise/Nef3",            NULL,                               0,0,3,-1,0 },
    { "eglise/Toit",            NULL,                               0,0,3,-1,0 },
    { "eglise/Clocher",     "egliselight/Clocher",      0,0,3,-1,0 },
    { "eglise/Girouette",   NULL,                               0,1,3,-1,0 },
    { "eglise/Porte",           NULL,                               0,0,3,-1,0 },
    { "eglise/ContreFort1", "egliselight/ContreFort1",  0,0,3,-1,1 },
    { "eglise/ContreFort2", "egliselight/ContreFort2",  0,0,3,-1,1 },
    { "eglise/ContreFort3", "egliselight/ContreFort3",  0,0,3,-1,1 },
    { "eglise/ContreFort4", "egliselight/ContreFort4",  0,0,3,-1,1 },
    { "eglise/ContreFort5", "egliselight/ContreFort5",  0,0,3,-1,1 },
    { "eglise/ContreFort6", "egliselight/ContreFort6",  0,0,3,-1,1 },
    { "eglise/ContreFort7", "egliselight/ContreFort7",  0,0,3,-1,1 },
    { "eglise/ContreFort8", "egliselight/ContreFort8",  0,0,3,-1,1 }
};
piece_s pieceferme[] = {
    { "ferme/Hab1",         "ferme/Hab1",                   0,0,3,-1,0 },
    { "ferme/Hab2",         "ferme/Hab2",                   0,0,3,-1,0 },
    { "ferme/Toit1",            "ferme/Toit1",                  0,0,3,-1,0 },
    { "ferme/Toit2",            "ferme/Toit2",                  0,0,3,-1,0 },
    { "ferme/Grange",           "ferme/Grange",             0,0,3,-1,0 },
    { "ferme/GrangeToit",   "ferme/GrangeToit",         0,0,3,-1,0 },
    { "ferme/Mur1",         "ferme/Mur1",                   0,0,3,-1,0 },
    { "ferme/Mur2",         "ferme/Mur2",                   0,0,3,-1,0 },
    { "ferme/Mur3",         "ferme/Mur3",                   0,0,3,-1,0 },
    { "ferme/Mur4",         "ferme/Mur4",                   0,0,3,-1,0 },
    { "ferme/Poteau1",      "ferme/Poteau1",                0,0,3,-1,0 },
    { "ferme/Poteau2",      "ferme/Poteau2",                0,0,3,-1,0 },
    { "ferme/Poteau3",      "ferme/Poteau3",                0,0,3,-1,0 }
};
piece_s pieceusine[] = {
    { "usine/Usine",    "usine/Usine",  0,0,3,-1,0 },
    { "usine/Cheminee", "usine/Cheminee",   0,0,3,-1,0 }
};
piece_s moulin[] = {
    { "moulin/Moulin",  "moulin/Moulin",    0,0,3,-1,0 },
    { "moulin/Moyeu",       "moulin/Moyeu", 0,0,3,2,0 },    // doit etre bombardable sinon c'est DECO
    { "moulin/Pale1",       "moulin/Pale1", 1,1,0,1,1 },
    { "moulin/Pale2",       "moulin/Pale2", 1,1,0,1,1 },
    { "moulin/Pale3",       "moulin/Pale3", 1,1,0,1,1 },
    { "moulin/Pale4",       "moulin/Pale4", 1,1,0,1,1 },
    { "moulin/Sac1",        "moulin/Sac1",      0,0,0,-1,0 },
    { "moulin/Sac2",        "moulin/Sac2",      0,0,0,-1,0 },
    { "moulin/Sac3",        "moulin/Sac3",      0,0,0,-1,0 },
    { "moulin/Sac4",        "moulin/Sac4",      0,0,0,-1,0 }
};
piece_s piecevehic0[] = {
    { "tank/Carlingue",         "tanklight/Carlingue",      0,0,3,0,0 },
    { "tank/Tourelle",          "tanklight/Tourelle",       0,0,0,2,0 },
    { "tank/Rotor",             NULL,                               1,0,0,2,0 },
    { "tank/Canon1",                "tanklight/Canon1",         2,0,0,1,1 },
    { "tank/Canon2",                "tanklight/Canon2",         2,0,0,1,1 },
    { "tank/Canon3",                "tanklight/Canon3",         2,0,0,1,1 },
    { "tank/Canon4",                "tanklight/Canon4",         2,0,0,1,1 },
    { "tank/ChenilleG",         "tanklight/ChenilleG",      0,0,0,1,0 },
    { "tank/ChenilleD",         "tanklight/ChenilleD",      0,0,0,1,0 },
};
piece_s piecevehic1[] = {
    { "vehic/Camion","vehic/Camion",        0,0,3,0,0 },
    { "vehic/Roues","vehic/Roues",      0,1,3,1,0 },
    { "vehic/Phare1",NULL,                  0,0,2,1,0 },
    { "vehic/Phare2",NULL,                  0,0,2,1,0 }
};
piece_s piecevehic2[] = {
    { "vehic/Camionnette","vehic/Camionnette",0,0,3,0,0 },
    { "vehic/Rouesnette","vehic/Rouesnette",0,1,3,1,0 },
    { "vehic/Pharenette1",NULL,             0,0,2,1,0 },
    { "vehic/Pharenette2",NULL,             0,0,2,1,0 }
};
piece_s tracteur[] = {
    { "tracteur/Corp",      NULL,                       0,0,3,0,0 },
    { "tracteur/Capot",     "tracteur/Capot",       0,0,3,1,0 },
    { "tracteur/RoueArD",   NULL,                       0,0,3,1,0 },
    { "tracteur/RoueArG",   NULL,                       0,0,3,1,0 },
    { "tracteur/EssG",      "tracteur/EssG",        0,1,0,1,0 },
    { "tracteur/EssD",      "tracteur/EssD",        0,1,0,1,0 },
    { "tracteur/BrasD",     NULL,                       0,0,0,1,0 },
    { "tracteur/BrasG",     NULL,                       0,0,0,1,0 },
    { "tracteur/PhareD",        NULL,                       0,0,2,1,0 },
    { "tracteur/RoueAD",        NULL,                       0,0,0,1,0 },    // MYSTERE !
    { "tracteur/RoueAG",        NULL,                       0,0,0,1,0 },
    { "tracteur/PhareG",        NULL,                       0,0,2,1,0 },
    { "tracteur/Volant",        NULL,                       0,0,0,1,0 }
};
piece_s van[] = {
    { "van/Van",    "van/VanLight", 0,0,3,0,0 },
    { "van/RouesAr",    NULL,   0,0,3,1,0 },
    { "van/RouesAv",    NULL,   0,0,3,1,0 },
    { "van/PhareD", NULL,   0,0,2,1,0 },
    { "van/PhareG", NULL,   0,0,2,1,0 }
};
piece_s piecedeco0[] = {
    { "sol/Cratere0","sol/Cratere0", 0,0,0,-1,0 }
};
piece_s piecedeco1[] = {
    { "sol/Tronc0","sol/Tronc0", 0,0,3,-1,0 },
    { "sol/Branche01","sol/Branche01", 0,0,0,-1,0 },
    { "sol/Branche02","sol/Branche02", 0,0,0,-1,0 },
    { "sol/Branche03","sol/Branche03", 0,0,0,-1,0 }
};
piece_s piecearbre2[] = {
    { "arbre2/Tronc",           "arbre2/Tronc",      0,0,3,-1,0 },
    { "arbre2/Branche1",        "arbre2/Branche1",   0,0,0,-1,0 },
    { "arbre2/Branche2",        "arbre2/Branche2",   0,0,0,-1,0 },
    { "arbre2/Branche3",        "arbre2/Branche3",   0,0,0,-1,0 },
    { "arbre2/Feuille1",        "arbre2/Feuille1",   0,0,0,-1,0 },
    { "arbre2/Feuille2",        "arbre2/Feuille2",   0,0,0,-1,0 },
    { "arbre2/Feuille3",        "arbre2/Feuille3",   0,0,0,-1,0 },
    { "arbre2/Feuille4",        "arbre2/Feuille4",   0,0,0,-1,0 }
};
piece_s piecedeco2[] = {
    { "bovide/Bidoche",     "bovide/Bidoche",                    0,0,3,-1,0 },
    { "bovide/Lait",            "bovide/Lait",                       0,0,0,-1,0 },
    { "bovide/OreilleG",        NULL,                    0,1,0,-1,0 },
    { "bovide/OreilleD",        NULL,                    0,1,0,-1,0 },
    { "bovide/PateD",           "bovide/PateD",                      0,0,0,-1,0 },
    { "bovide/PateG",           "bovide/PateG",                      0,0,0,-1,0 },
    { "bovide/Queue",           "bovide/Queue",                      0,0,0,-1,0 },
    { "bovide/Tete",            "bovide/Tete",                       0,0,0,-1,0 },
    { "bovide/Cou",         "bovide/Cou",                        0,0,0,-1,0 },
    { "bovide/RomsteackG",  "bovide/RomsteackG",                 0,0,0,-1,0 },
    { "bovide/RomsteackD",  "bovide/RomsteackD",                 0,0,0,-1,0 },
    { "bovide/CorneG",      NULL,                    0,1,0,-1,0 },
    { "bovide/CorneD",      NULL,                    0,1,0,-1,0 }
};
piece_s piecenuage[] = {
    { "sol/Nuage",NULL, 0,1,0,-1,0 }
};
piece_s piecefumee[] = {
    { "sol/Nuage",NULL, 0,1,0,0,0 }
};
piece_s piecegrav0[] = {
    { "sol/Debris0","sol/Debris0", 0,1,0,0,1 }
};
viondesc_s viondesc[NBNAVIONS] = {
    //                                    motor,lift,drag
    {"dewoitine",5,1000,1,0,0,50,55,2,0,0,1,0.85,1.2,1.1, 600,200000, { 7, 8, 9}},
    {"corsair",  0,5000,1,2,0,49,50,4,0,0,0,1.07,1.0,1.0, 800,500000, {24,25,23}},
    {"moshito",  0,7000,2,4,1,45,46,4,1,1,0,1.30,1.0,0.9,1000,300000, {42,44,38}},
    {"bogoplane",0,2000,1,0,0,23,24,1,0,0,1,0.90,1.0,1.2, 700,300000, {17,18,19}},
    {"spitflame",0,4000,1,0,0,29,30,4,0,0,0,1.00,1.1,1.0,1300,430000, {24,25,26}},
    {"snophill", 3, 800,1,0,0,31,32,1,0,0,1,0.78,1.4,1.5, 500,170000, {26,27,28}}
};
nobjet_s nobjet[] = {
    { 65, piecenavion2,0 }, { 61, piecenavion3,0}, { 70, piecenavion4,0 }, {46, piecenaviontoy,0 }, {47,spitflame,0 }, {45, snophill,0 },
    { 22+14, piecepiste ,0 }, { 2, piecetour ,0 },
    { 8, piecemaison0 ,0 }, { 15, pieceeglise ,0 }, { 13, pieceferme ,0 }, { 2, pieceusine ,0 }, {10, moulin ,0 }, {2, reverbere ,0 },
    {9, piecevehic0,0 }, {4, piecevehic1,0 }, {4, piecevehic2,0 }, {13,tracteur,0 }, {5,van,0 },
    {1,piecenuage,0 }, {1,piecefumee,0 }, {1, piecedeco0,0 }, {4, piecedeco1,0 }, {8,piecearbre2,0 }, {13,piecedeco2,0 }, {1,piecegrav0,0 },
    {27,zeppelin,0 }
};

// load les modèles en les hierarchisant
void loadmodele(int n, char *fn, char *fnlight, int type, int pere, int plat, int plat2, int no, int fixe) {
    FILE *in;
    int i, j;
    facelight ftmp;
    vector u,v;
    char fncol[200];
    in = file_open(fn, DATADIR, "r");
    file_read(&mod[n].offset, sizeof(vector), in);
    file_read(&mod[n].nbpts[0], sizeof(int), in);
    file_read(&mod[n].nbfaces[0], sizeof(int), in);
    mod[n].pts[0]=malloc(mod[n].nbpts[0]*sizeof(vector));
    mod[n].fac[0]=malloc(mod[n].nbfaces[0]*sizeof(face));
    file_read(mod[n].pts[0], sizeof(vector) * mod[n].nbpts[0], in);
    for (i=0; i<mod[n].nbfaces[0]; i++) {
        file_read(&ftmp, sizeof(facelight), in);
        for (j=0; j<3; j++) mod[n].fac[0][i].p[j]=ftmp.p[j];
        // normale à la face
        if (plat)
            copyv(&mod[n].fac[0][i].norm,&vec_zero);
        else {
            subv3(&mod[n].pts[0][mod[n].fac[0][i].p[1]],&mod[n].pts[0][mod[n].fac[0][i].p[0]],&u);
            subv3(&mod[n].pts[0][mod[n].fac[0][i].p[2]],&mod[n].pts[0][mod[n].fac[0][i].p[0]],&v);
            prodvect(&v,&u,&mod[n].fac[0][i].norm);
            renorme(&mod[n].fac[0][i].norm);
        }
    }
    // normales aux points
    mod[n].norm[0]=malloc(mod[n].nbpts[0]*sizeof(vector));
    for (i=0; i<mod[n].nbpts[0]; i++) {
        copyv(&u,&vec_zero);
        for (j=0; j<mod[n].nbfaces[0]; j++) {
            if (mod[n].fac[0][j].p[0]==i || mod[n].fac[0][j].p[1]==i || mod[n].fac[0][j].p[2]==i)
                addv(&u, &mod[n].fac[0][j].norm);
        }
        renorme(&u);
        copyv(&mod[n].norm[0][i],&u);
    }
    fclose(in);
    // load colors
    static pixel const black = { 0, 0, 0 };
    sprintf(fncol,"%s.col",fn);
    in=file_open_try(fncol, DATADIR, "r");
    if (in) {
        for (i=0; i<mod[n].nbfaces[0]; i++) file_read(&mod[n].fac[0][i].color,sizeof(pixel), in);
        fclose(in);
    } else {
        for (i=0; i<mod[n].nbfaces[0]; i++) mod[n].fac[0][i].color = black;
    }
    // IDEM AVEC LA VERSION LIGHT
    if (fnlight==NULL) {
        mod[n].pts[1]=NULL;
        mod[n].norm[1]=NULL;
        mod[n].fac[1]=NULL;
        mod[n].nbpts[1]=0;
        mod[n].nbfaces[1]=0;
    } else {
        in = file_open(fnlight, DATADIR, "r");
        file_read(&u, sizeof(vector), in);
        file_read(&mod[n].nbpts[1], sizeof(int), in);
        file_read(&mod[n].nbfaces[1], sizeof(int), in);
        mod[n].pts[1]=malloc(mod[n].nbpts[1]*sizeof(vector));
        mod[n].fac[1]=malloc(mod[n].nbfaces[1]*sizeof(face));
        file_read(mod[n].pts[1], sizeof(vector) * mod[n].nbpts[1], in);
        for (i=0; i<mod[n].nbfaces[1]; i++) {
            file_read(&ftmp, sizeof(facelight), in);
            for (j=0; j<3; j++) mod[n].fac[1][i].p[j]=ftmp.p[j];
            if (plat2) copyv(&mod[n].fac[1][i].norm,&vec_zero);
            else {
                subv3(&mod[n].pts[1][mod[n].fac[1][i].p[1]],&mod[n].pts[1][mod[n].fac[1][i].p[0]],&u);
                subv3(&mod[n].pts[1][mod[n].fac[1][i].p[2]],&mod[n].pts[1][mod[n].fac[1][i].p[0]],&v);
                prodvect(&v,&u,&mod[n].fac[1][i].norm);
                renorme(&mod[n].fac[1][i].norm);
            }
        }
        // normales aux points
        mod[n].norm[1]=malloc(mod[n].nbpts[1]*sizeof(vector));
        for (i=0; i<mod[n].nbpts[1]; i++) {
            copyv(&u,&vec_zero);
            for (j=0; j<mod[n].nbfaces[1]; j++) {
                if (mod[n].fac[1][j].p[0]==i || mod[n].fac[1][j].p[1]==i || mod[n].fac[1][j].p[2]==i)
                    addv(&u, &mod[n].fac[1][j].norm);
            }
            renorme(&u);
            copyv(&mod[n].norm[1][i],&u);
        }
        fclose(in);
        // load les couleures
        sprintf(fncol,"%s.col",fnlight);
        in=file_open_try(fncol, DATADIR, "r");
        if (in) {
            for (i=0; i<mod[n].nbfaces[1]; i++) file_read(&mod[n].fac[1][i].color, sizeof(pixel), in);
            fclose(in);
        } else {
            // use detailed model colors
            for (i=0; i<mod[n].nbfaces[1]; i++) {
                mod[n].fac[1][i].color = i < mod[n].nbfaces[0] ?
                    mod[n].fac[0][i].color :
                    black;
            }
        }
    }
    mod[n].type=type;
    mod[n].pere=pere;
    mod[n].nobjet=no;
    mod[n].fixe=fixe;
    mod[n].rayon=0;
    for (i=0; i<mod[n].nbpts[0]; i++) if (norme(&mod[n].pts[0][i])>mod[n].rayon) mod[n].rayon=norme(&mod[n].pts[0][i]);
    if (mod[n].type==BOMB) mod[n].rayoncollision=Easy?400:250;
    else {
        mod[n].rayoncollision=mod[n].rayon;
        if (mod[n].type==AVION && n==pere) mod[n].rayoncollision*=1.5;
    }
}

void LoadModeles() {
    int i, s, nbmod=0, n;
//  printf("Loading objects...\n");
    for (i=0; i<NBNAVIONS+NBBASES+NBMAISONS+NBVEHICS+NBDECOS+NBZEPPELINS; i++) nbmod+=nobjet[i].nbpieces;
    mod=(modele*)malloc((1+nbmod+1)*sizeof(modele));
    // MODELE 0 : TIR
    copyv(&mod[0].offset,&vec_zero);
    for (i=0; i<NBNIVOLISS; i++) {
        static vector ptstir[]={{-40,0,0},{40,0,0}};
        mod[0].nbpts[i]=2; mod[0].nbfaces[i]=0;
        mod[0].pts[i]=mod[0].norm[i]=ptstir;
        mod[0].fac[i]=NULL;
    }
    mod[0].rayoncarac=0; mod[0].rayoncollision=Easy?10:3; mod[0].rayon=60; mod[0].type=TIR; mod[0].fixe=0; mod[0].pere=0;
    nbtir=0;
    s=1;
    // LOAD NAVIONS
    for (n=0; n<NBNAVIONS; n++) {
        nobjet[n].firstpiece=s;
        for (i=0; i<nobjet[n].nbpieces; i++) {
            int t=AVION;
            if (nobjet[n].piece[i].bomb==1) t=BOMB;
            else if (nobjet[n].piece[i].bomb==2) t=PHARE;
            else if (i==viondesc[n].tabbord) t=TABBORD;
            loadmodele(s++, nobjet[n].piece[i].fn,nobjet[n].piece[i].fnlight, t, nobjet[n].firstpiece+nobjet[n].piece[i].pere, nobjet[n].piece[i].plat,nobjet[n].piece[i].platlight,n,nobjet[n].piece[i].mobil);
            affjauge(.25/nbmod);
        }
    }
    // LOAD BABASES
    for (; n<NBNAVIONS+NBBASES; n++) {
        nobjet[n].firstpiece=s;
        for (i=0; i<nobjet[n].nbpieces; i++) {
            int t=DECO;
            if (nobjet[n].piece[i].bomb==2) t=PHARE;
            else if (nobjet[n].piece[i].bomb==3) t=CIBGRAT;
            loadmodele(s++, nobjet[n].piece[i].fn,nobjet[n].piece[i].fnlight, t, nobjet[n].firstpiece+nobjet[n].piece[i].pere, nobjet[n].piece[i].plat,nobjet[n].piece[i].platlight,n,nobjet[n].piece[i].mobil);
            affjauge(.25/nbmod);
        }
    }
    // LOAD MAISONS
    for (; n<NBNAVIONS+NBBASES+NBMAISONS; n++) {
        nobjet[n].firstpiece=s;
        for (i=0; i<nobjet[n].nbpieces; i++) {
            int t=DECO;
            if (nobjet[n].piece[i].bomb==2) t=PHARE;
            else if (nobjet[n].piece[i].bomb==3) t=CIBGRAT;
            loadmodele(s++, nobjet[n].piece[i].fn,nobjet[n].piece[i].fnlight, t, nobjet[n].firstpiece+nobjet[n].piece[i].pere, nobjet[n].piece[i].plat,nobjet[n].piece[i].platlight,n,nobjet[n].piece[i].mobil);
            affjauge(.25/nbmod);
        }
    }
    // LOAD VEHICS
    for (; n<NBNAVIONS+NBBASES+NBMAISONS+NBVEHICS; n++) {
        nobjet[n].firstpiece=s;
        for (i=0; i<nobjet[n].nbpieces; i++) {
            int t=VEHIC;
            if (nobjet[n].piece[i].bomb==2) t=PHARE;
            loadmodele(s++, nobjet[n].piece[i].fn,nobjet[n].piece[i].fnlight, t, nobjet[n].firstpiece+nobjet[n].piece[i].pere, nobjet[n].piece[i].plat,nobjet[n].piece[i].platlight,n,nobjet[n].piece[i].mobil);
            affjauge(.25/nbmod);
        }
    }
    // LOAD DECOS
    for (; n<NBNAVIONS+NBBASES+NBMAISONS+NBVEHICS+NBDECOS; n++) {
        nobjet[n].firstpiece=s;
        for (i=0; i<nobjet[n].nbpieces; i++) {
            int t;
            if (n==NBNAVIONS+NBBASES+NBMAISONS+NBVEHICS) t=NUAGE;
            else if (n==NBNAVIONS+NBBASES+NBMAISONS+NBVEHICS+1) t=FUMEE;
            else t=DECO;
            loadmodele(s++, nobjet[n].piece[i].fn,nobjet[n].piece[i].fnlight, t, nobjet[n].firstpiece+nobjet[n].piece[i].pere, nobjet[n].piece[i].plat,nobjet[n].piece[i].platlight,n,nobjet[n].piece[i].mobil);
            affjauge(.25/nbmod);
        }
    }
    // LOAD ZEPPELINS
    for (; n<NBNAVIONS+NBBASES+NBMAISONS+NBVEHICS+NBDECOS+NBZEPPELINS; n++) {
        nobjet[n].firstpiece=s;
        for (i=0; i<nobjet[n].nbpieces; i++) {
            loadmodele(s++, nobjet[n].piece[i].fn,nobjet[n].piece[i].fnlight, ZEPPELIN, nobjet[n].firstpiece+nobjet[n].piece[i].pere, nobjet[n].piece[i].plat,nobjet[n].piece[i].platlight,n,nobjet[n].piece[i].mobil);
            affjauge(.25/nbmod);
        }
    }
/*  // LOAD GRAVS
    for (; n<NBNAVIONS+NBBASES+NBMAISONS+NBVEHICS+NBDECOS+NBGRAVS; n++) {
        nobjet[n].firstpiece=s;
        for (i=0; i<nobjet[n].nbpieces; i++) {
            loadmodele(s++, nobjet[n].piece[i].fn,nobjet[n].piece[i].fnlight, GRAV, nobjet[n].firstpiece+nobjet[n].piece[i].pere, nobjet[n].piece[i].plat,nobjet[n].piece[i].platlight,n,nobjet[n].piece[i].mobil);
        }
    }*/
    for (s--; s>=0; s--) subv(&mod[s].offset,&mod[mod[s].pere].offset);
}
int addnobjet(int na, vector *p, matrix *m, uchar sol) {
    int i;
    int firstobj=nbobj;
    for (i=nobjet[na].firstpiece; i<nobjet[na].firstpiece+nobjet[na].nbpieces; i++) {
        vector pp;
        if (mod[i].pere==i)
            addobjet(i, p, m, -1, sol);
        else {
            mulmv(m,&mod[i].offset,&pp);
            addv(&pp,&obj[firstobj+mod[i].pere-nobjet[na].firstpiece].pos);
            addobjet(i, &pp, m, mod[i].fixe<1?-1:firstobj+mod[i].pere-nobjet[na].firstpiece, 0);
        }
    }
    if (na<NBNAVIONS) { // ne pas afficher les poscam
        obj[nbobj-1].aff=0;
    }
    return firstobj;
}
