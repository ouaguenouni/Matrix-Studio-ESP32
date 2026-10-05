"""Regenerate the embedded browser interface and the local config before PlatformIO builds."""
from pathlib import Path
import subprocess

Import("env")

root = Path(env.subst("$PROJECT_DIR"))
subprocess.check_call([env.subst("$PYTHONEXE"), str(root / "tools" / "build_ui.py")])
subprocess.check_call([env.subst("$PYTHONEXE"), str(root / "tools" / "build_config.py")])
