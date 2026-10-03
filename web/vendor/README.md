`omggif.mjs` vendors omggif 1.0.10 by Dean McNamee, from
https://github.com/deanm/omggif/blob/master/omggif.js, downloaded 2026-10-03.
The MIT license and copyright are retained at the top of the source.

Local changes: ES module exports and tolerance for NUL padding between GIF
blocks. The Crystal shiny Magmar GIF contains this padding; browsers and Pillow
also accept it. All downloaded sprite frames are checked against Pillow output.
