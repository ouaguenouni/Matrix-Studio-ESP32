"""Regenerate the embedded browser interface before PlatformIO builds."""
from pathlib import Path
import subprocess

Import("env")

subprocess.check_call([
    env.subst("$PYTHONEXE"),
    str(Path(env.subst("$PROJECT_DIR")) / "tools" / "build_ui.py"),
])
