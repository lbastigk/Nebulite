# Build script for Nebulite
#
# Call from content root directory with:
#  python ./Container/build.py <cmake-preset>

# Take name of the preset, parse CMakePresets.json, extract the compiler to use and launch the correct podman container

import json
import pathlib
import subprocess
import sys


ROOT = pathlib.Path(__file__).resolve().parent.parent


CONTAINER_DIR = ROOT / "Container"
PRESETS = ROOT / "CMakePresets.json"

def load_preset(name):
    with PRESETS.open() as f:
        data = json.load(f)

    for preset in data.get("configurePresets", []):
        if preset["name"] == name:
            return preset

    raise RuntimeError(f"Unknown preset: {name}")


def get_container(preset):
    try:
        return preset["vendor"]["nebulite"]["container"]
    except KeyError:
        raise RuntimeError(
            f"Preset '{preset['name']}' has no nebulite container defined! Please add key vendor.nebulite.container to the preset."
        )


def get_container_preset_name(preset_name):
    return f"container-{preset_name}"


def run(cmd):
    print("+", " ".join(cmd))
    subprocess.run(cmd, check=True)


def image_name(container):
    return f"localhost/nebulite-build-{container}:latest"


def rebuild_image(image):
    run(["podman", "rmi", "-f", image])


def ensure_image(container, rebuild=False):
    image = image_name(container)

    result = subprocess.run(
        ["podman", "image", "exists", image],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )

    if result.returncode == 0:
        if rebuild:
            print(f"Rebuilding image {image}...")
            rebuild_image(image)
        else:
            print(
                f"Image {image} already exists. If you want to rebuild it, run this script with the --rebuild option."
            )
            return image
    else:
        print(f"Image {image} not found, building...")

    containerfile_dir = CONTAINER_DIR / container

    if not containerfile_dir.exists():
        raise RuntimeError(
            f"No Containerfile directory found for '{container}'"
        )

    run(
        [
            "podman",
            "build",
            "-t",
            image,
            str(containerfile_dir),
        ]
    )

    return image


def run_container(container, preset, rebuild=False):
    image = ensure_image(container, rebuild=rebuild)

    # TODO: using nproc used to cause issues on low memory systems, add a guard for that

    # TODO: running this script in a clion terminal with the procided task shows escape sequences instead of coloring

    run(
        [
            "podman",
            "run",
            "--rm",
            "-it",
            "-v",
            f"{ROOT}:{ROOT}:Z",
            "-w",
            f"{ROOT}",
            image,
            "bash",
            "-c",
            f"cmake --preset {preset} && cmake --build --preset {preset} -j$(nproc)",
        ]
    )


def main():
    rebuild = False
    args = sys.argv[1:]

    if "--rebuild" in args:
        rebuild = True
        args = [arg for arg in args if arg != "--rebuild"]

    if len(args) != 1:
        print("Usage: build.py [--rebuild] <cmake-preset>")
        sys.exit(1)

    preset_name = args[0]
    container_preset = load_preset(get_container_preset_name(preset_name))
    container = get_container(container_preset)
    run_container(container, get_container_preset_name(preset_name), rebuild=rebuild)


if __name__ == "__main__":
    main()