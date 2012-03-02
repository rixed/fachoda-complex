Fachoda Complex
===============

Fachoda Complex is a lightweight airplane arcade simulator, free to
use/copy/modify under the terms of the GPLv3.

For further details, see [homepage](http://rixed.github.com/fachoda-complex/).

Build
-----

To compile fachoda from source, run:

    cd src && make

should do it, provided you have SDL, libjpeg and openAL installed (on
Debian distributions, I mean the `-dev` packages).

For better performances you can try:

    cd src && CFLAGS=-O4 make -j6

Objective
---------

The objective is to make as much money as possible by destroying as many
enemies as you can. From time to time rewards are offered for destruction of a
given target (be it friend or foe), flagged in the map with a small magenta
skull. As you have to pay for every bullet you'd better go for these before a
computer controlled plane or tank does it.

You need to return to your home base and park to refuel and repair your plane.
There you can also buy a better plane by parking your plane close to an unused
one you can afford and press F10.

Each time you earn or lose money the new balance is written in front of the
screen (blue for credits or red for debits, then white for the actual
balance).

Your final score is the maximal return on investment achieved during the whole
play.

Whenever your plane get shot down you will take the control of another plane
that's flying on your side and which value is less or equal to the one you
just left, until there's no more and the game is over. When respawning,
though, the Lord takes all your money but $55.


The planes
----------

You can fly six different warbirds:

- Snophill, a red biplane with no bombing facilities
- Dewoitine, a green good old plane
- Bogoplane, an ugly white war bird
- Spitflame, a raced red acrobat
- Corsair, escaped from the pacific
- Moshito, an orange bi-propulsed gunner

Each have various abilities and capacity.
You start the game in a Dewoitine.


Price list
----------

		Goods:
    Buller ............ $1
    Bomb ............ $200
    Fiul tank ....... $400 (approx)
    Repairs ....... depend

		Planes:
    Spotwill ........ $300
    Dewoitine ...... $1000
    Bogoplane ...... $2000
    Corsair ........ $5000
    Spitflame ...... $6000
    Moshito ........ $7000


Controls
========

Mouse
-----

The mouse controls the flight stick. Left button fires the selected weapon,
right button switches between guns and bombs.

You can play with keyboard instead ('-nomouse' option). The numeric keypad's
`8`, `4`, `6` and `2` then works for up, left, right and down, while `5`
centers the stick. You can also set keyboard sensitiveness in the option menu,
and tune the neutral point of the flight stick during the play by the
combinations `5`+`2` or `5`+`8` (that is : center+up and center+down) on the
keypad.

In -nomouse mode, `space` fires and `control` switches between weapons.

Keys
----

Here are a key list. You can redefine all these in the option menu.

`ESC`: Quit

`+`/`-`: throttle

`F7`/`F8`: zoom in map mode and external views

`F4`: swap between internal views (normal/dogfight)

`F6`: external view (traveling)

`F5`: swap between external views (around, satellite, pursuit, bomb)

`F2`/`F3`: cycle through other planes of your side, or through close planes in
dogfight view

`F1`: view your own plane, or select the closest enemy plane in dogfight view

`F9`: map view

`P`: autopilot

`G`: gears

`F`: flaps

`B`: wheel brakes

`X`: accelerated mode (make sure to set autopilot first!)

`S`: set a flag at plane's location on the map

`N`: set navpoint to home base

Arrows: watch in every direction in internal view ; move the map in map view

`Home`: back to front view (must be hold in dogfight view)

`End`: rear view (same)

`Delete`: left view

`PgDown`: right view

`PgUp`: up view

`Insert` (hold): focus at the instrument panel

`Pause`: guess what

`F10`: sell your plane and by the closest one amongst those parked at your home
base.

`Enter` (hold): display the highs-cores

Mouse: move the command controls

Left click: fire with the active weapon

Right click: select the other weapon (guns/bombs)

`F12`: suicide


Instrument panel
----------------

                         +--+
     ---------  -------- |a | -------   +-------+
    |elevating|| speed  ||na|| alti  |  |bullets|
    |  speed  || meter  ||gt|| meter |  |  bombs|
     >-------<  >------< |lt| >-----<   +-------+
    |  slope  ||throttle||ea||       || Gear |Flaps|
    |indicator||--------|| c||compass| >----< >---<
     --------- |  fiul  ||ok||       ||Brakes|Auto |
                -------- |f | -------        |pilot|
                         +--+



Development
===========

Restoring vintage software
--------------------------

Some restore antique furnitures or old cars for a hobby.  I am restoring a
restoring a vintage piece of code, a small game that I quickly put together at
the end of the former century and have not touched since.

I've always wanted to come up with a follow up but even if I started on
several occasions I never achieved anything, nor did I maintained this
original version. Being not interested in games nor 3D rendering any more, the
only second version of it I can seriously envision now is to bring back to
life this rusty piece of software.

I addition to bringing peace to my conscience I will also probably gain a few
good laughs rediscovering the old bugs and non portable monstrosities I
committed back then.

Roadmap
-------

So, after a quick diagnosis, here is a short list of what need be done:

- Replace the former README (done!)

- Replace the old MMX x86 code by slow but portable C equivalent (done)

- Fix the bugs related to framerate issues (hosts are much faster now and I
  suspect this will reveal some problems) (done)

- Upgrade build process to modern (??) autotools

- Fix compilation warnings, move around the code, reorganize the files
  somewhat to comply with my present taste

- Replace the whole software renderer by a mere OpenGL 2.1 renderer, saying
  goodbye to the original (but somewhat bugged) shading of the game

- Replace the aforementioned OpenGL 2.1 renderer by a modern OpenGL-ES
  renderer with simple shaders in order to revive some of the original feeling
  of the game (but not the related bugs).

And do all this by small steps, without imposing on myself any deadline or
interfering with my many other projects.

After all, the game was already released 12 years ago wasn't it?

