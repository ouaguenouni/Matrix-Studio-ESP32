Legacy Pokémon cries #1–251 from the PokéAPI cries archive:
https://github.com/PokeAPI/cries
Pinned revision: ef687b18f0ce17169b4b4c09175819f7ade92f0f

Original downloads are in legacy/*.ogg. The catalogue maps Pokédex numbers to
their original and converted SHA-256 hashes. Normal/shiny versions and every
Unown letter share their species cry. These are legacy game cries, not anime
voices. Pokémon audio belongs to Nintendo / Game Freak; this archive does not
grant ownership of those recordings.

adpcm/*.ima stores 8 kHz mono WAV IMA ADPCM data blocks (256 bytes/block),
converted using ffmpeg. Playback interpolates to 44.1 kHz stereo for A2DP.
tools/prepare_cries.py packs converted files into cries.bin for firmware flash;
normal PlatformIO builds are offline and do not require ffmpeg or downloading.
To download/reconvert explicitly: python3 tools/prepare_cries.py --download --convert
