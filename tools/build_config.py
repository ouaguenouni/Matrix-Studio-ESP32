#!/usr/bin/env python3
"""Turn the gitignored config.conf into a header the firmware can compile in."""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
source = root / "config.conf"
values = {}
if source.exists():
    for raw in source.read_text().splitlines():
        line = raw.split("#", 1)[0].strip()
        if not line or "=" not in line:
            continue
        key, value = line.split("=", 1)
        values[key.strip()] = value.strip().strip('"').strip("'")

key = values.get("youtube_api_key", "")
escaped = key.replace("\\", "\\\\").replace('"', '\\"')
header = root / "include" / "local_config.h"
header.write_text("#pragma once\n#define YOUTUBE_API_KEY \"%s\"\n" % escaped)
print("Loaded local config." if key else "No youtube_api_key in config.conf; YouTube stays unset.")
