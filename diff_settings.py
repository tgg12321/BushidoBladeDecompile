#!/usr/bin/env python3
# asm-differ configuration for Bushido Blade 2 decomp

def apply(config, args):
    config["baseimg"] = "disc/SLUS_006.63"
    config["myimg"] = "build/bb2.exe"
    config["mapfile"] = "build/bb2.map"
    config["source_directories"] = ["src"]
    config["arch"] = "mipsel"
    config["map_format"] = "gnu"
    config["build_command"] = [
        "make",
    ]
    config["objdump_executable"] = "mipsel-linux-gnu-objdump"
