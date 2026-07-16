import json
import os
import pathlib
import psutil
import subprocess
import sys

ROOT_RESOLVED = pathlib.Path(__file__).resolve().parent.parent
ROOT = pathlib.Path(__file__).parent.parent

CONTAINER_DIR = ROOT / "Container"
PRESETS = ROOT / "CMakePresets.json"


def determine_process_count() -> int:
    # Reservere a set amount of RAM per process to avoid running out of memory
    ram_per_process = 4 # GiB

    # Get the total amount of RAM and the number of CPU threads available on the system
    total_ram = psutil.virtual_memory().total / (1024 ** 3) # GiB
    total_threads = os.cpu_count()

    # Determine process count
    max_processes = min(int(total_ram / ram_per_process), total_threads)
    if not isinstance(max_processes, int):
        error_msg = f"Expected max_processes to be an integer, but got {type(max_processes).__name__} instead."
        raise RuntimeError(error_msg)
    if max_processes < 1:
        return 1
    return max_processes

def load_preset(name: str):
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


def get_container_preset_name(preset_name: str):
    return f"container-{preset_name}"

def run(cmd: str):
    print("+", " ".join(cmd))
    subprocess.run(cmd, check=True)

def image_name(container: str) -> str:
    return f"localhost/nebulite-build-{container}:latest"

def rebuild_image(image: str):
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

def run_container(container, preset, rebuild=False, type="build"):
    image = ensure_image(container, rebuild=rebuild)

    if type == "test":
        # Find the singular binary in ./bin/
        if not (ROOT / "bin").exists():
            raise RuntimeError(
                f"Cannot test container {image} because the ./bin/ directory does not exist. Please build the project first."
            )
        binaries = list((ROOT / "bin").glob("*"))
        if len(binaries) != 1:
            raise RuntimeError(
                f"Cannot test container {image} because there is not exactly one binary in ./bin/. Found: {binaries}"
            )
        binary = binaries[0]

        print(f"Testing container {image}...")
        run(
            [
                "podman",
                "run",
                "--rm",
                "-it",
                "-e",
                "SDL_AUDIODRIVER=dummy",
                "-e",
                "SDL_VIDEODRIVER=dummy",
                "-v",
                f"{ROOT}:{ROOT}:Z",
                "-w",
                f"{ROOT}",
                image,
                "bash",
                "-c",
                f"{binary} help"
            ]
        )
        run(
            [
                "podman",
                "run",
                "--rm",
                "-it",
                "-e",
                "SDL_AUDIODRIVER=dummy",
                "-e",
                "SDL_VIDEODRIVER=dummy",
                "-v",
                f"{ROOT}:{ROOT}:Z",
                "-w",
                f"{ROOT}",
                image,
                "bash",
                "-c",
                "make test"
            ]
        )
    elif type == "build":
        process_count = determine_process_count()
        print(f"Running build with {process_count} processes")
        cmd = [
            "podman",
            "run",
            "--rm",
            "-it",
            "-v",
            f"{ROOT}:{ROOT}:Z",
        ]
        # If ROOT is a symlink, also mount the resolved path.
        if ROOT != ROOT_RESOLVED:
            cmd.extend([
                "-v",
                f"{ROOT_RESOLVED}:{ROOT_RESOLVED}:Z",
            ])
        cmd.extend([
            "-w",
            f"{ROOT}",
            image,
            "bash",
            "-c",
            f"cmake --preset {preset} && cmake --build --preset {preset} -j{process_count}"
        ])
        run(cmd)
    elif type == "lint":
        cmd = [
            "podman",
            "run",
            "--rm",
            "-it",
            "-v",
            f"{ROOT}:{ROOT}:Z",
        ]
        # If ROOT is a symlink, also mount the resolved path.
        if ROOT != ROOT_RESOLVED:
            cmd.extend([
                "-v",
                f"{ROOT_RESOLVED}:{ROOT_RESOLVED}:Z",
            ])
        cmd.extend([
            "-w",
            f"{ROOT}",
            image,
            "bash",
            "-c",
            "./Scripts/Validation/clangCheck.sh --main-diff"
        ])
        run(cmd)

def main():
    test = False
    rebuild = False
    args = sys.argv[1:]

    # Rebuild container
    if "--rebuild" in args:
        rebuild = True
        args = [arg for arg in args if arg != "--rebuild"]

    # Determine type
    if "--test" in args:
        type = "test"
        args = [arg for arg in args if arg != "--test"]
    elif "--lint" in args:
        type = "lint"
        args = [arg for arg in args if arg != "--lint"]
    else:
        type = "build"


    if len(args) != 1:
        print("Usage: build.py [--rebuild] [--test] [--lint] <cmake-preset>")
        print("Using test will not build the container but just test it.")
        sys.exit(1)

    preset_name = args[0]
    container_preset = load_preset(get_container_preset_name(preset_name))
    container = get_container(container_preset)
    run_container(container, get_container_preset_name(preset_name), rebuild=rebuild, type=type)

if __name__ == "__main__":
    print(f"Root is {ROOT}")
    main()