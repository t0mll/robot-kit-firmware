# Bakes the Firmware version into the build: FW_VERSION from the environment (CI sets it
# from the git tag or short SHA), otherwise "dev".
import os
Import("env")  # noqa: F821  (provided by PlatformIO)
version = os.environ.get("FW_VERSION", "dev")
env.Append(CPPDEFINES=[("FW_VERSION", env.StringifyMacro(version))])  # noqa: F821
