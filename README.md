# License to Illuse

Requires [Egg v2](https://github.com/aksommerville/egg2) to build.

Uplifting Game Jam #7, theme "ILLUSION".

Note: This repo, and internal symbols, are "license" with an S but the game's true title is "licence" with a C.
I'm American and I forgot that the game's inspiration, _Licence to Kill_, is British. Oops.

## TODO

- [x] Physics
- [x] Attacks
- [x] Foes
- [x] Level end
- [x] Death
- [x] Rabbit and Bird should be metered, you have to collect and spend them.
- [x] Victory
- [x] Three levels
- [x] Hello
- [x] Sound effects
- [x] Music
- [x] sprite_soldier, we have walking tiles, but didn't implement idle walking.
- [x] Should soldier defunct after some time, when spooked and horizontally blocked?
- [x] Soldiers tend to accidentally fall off platforms. I guess we do need to be exact in the path selection.
- [ ] Final pass: Ensure all powerup index are unique, and powerups come before active sprites in the command list.
- [x] Can step off goal and cause it to reload current map. Oops. Should be that once you touch the goal, you will win, even if you leave it.
- - ...tightened up clocks and drop some inputs when we're fading out. It's still technically possible to un-win, but shouldn't happen by accident.
- [x] Sometimes I fall thru a oneway when dropping on to it.
- - Might take a hundred tries but yes, a simple non-walking jump can do it at the first oneway in level 1. Short jumps, needn't go all the way up.
- - ...failed to account for SMIDGE in the previous position. Corrected, and ran >100 reps with no fall-thru.
- [ ] Prettier scenery.
- [ ] Itch page.
- [x] Need some hard reset option! Esp at the very end, if you scare the last guard, you can't win and maybe can't die either.
- - AUX1 to pause, with "Restart Level" and "Main Menu" options.
