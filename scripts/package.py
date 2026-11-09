"""Merge each built Robot into one browser-flashable binary plus an ESP Web Tools manifest.

Usage (after `pio run`):  python scripts/package.py <version> [env ...]
Writes dist/<env>/<env>-<version>.bin and dist/<env>/manifest.json (offset 0, ESP32-S3).
"""
import json
import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
PIO_HOME = pathlib.Path.home() / ".platformio"
NAMES = {"rover": "Rover", "biped": "Biped"}


def boot_app0() -> pathlib.Path:
    path = PIO_HOME / "packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin"
    if not path.exists():
        sys.exit(f"boot_app0.bin not found at {path}; run `pio run` first")
    return path


def package(env: str, version: str) -> None:
    build = ROOT / ".pio/build" / env
    out_dir = ROOT / "dist" / env
    out_dir.mkdir(parents=True, exist_ok=True)
    merged = out_dir / f"{env}-{version}.bin"
    subprocess.run(
        [
            sys.executable, "-m", "esptool", "--chip", "esp32s3", "merge_bin",
            "-o", str(merged), "--flash_mode", "dio", "--flash_freq", "80m", "--flash_size", "16MB",
            "0x0", str(build / "bootloader.bin"),
            "0x8000", str(build / "partitions.bin"),
            "0xe000", str(boot_app0()),
            "0x10000", str(build / "firmware.bin"),
        ],
        check=True,
    )
    manifest = {
        "name": f"Robot Kit Shop {NAMES.get(env, env)}",
        "version": version,
        # First installs ask before erasing; updates keep NVS (calibration, settings).
        "new_install_prompt_erase": True,
        "builds": [{"chipFamily": "ESP32-S3", "parts": [{"path": merged.name, "offset": 0}]}],
    }
    (out_dir / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
    print(f"packaged {env} {version}: {merged.relative_to(ROOT)}")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    version, envs = sys.argv[1], sys.argv[2:] or list(NAMES)
    for env in envs:
        package(env, version)
