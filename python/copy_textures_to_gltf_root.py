#!/usr/bin/env python3
import os
import shutil
import argparse

def collect_dds(root_dir):
    root_dir = os.path.abspath(root_dir)

    for current_root, _, files in os.walk(root_dir):
        for name in files:
            if name.lower().endswith(".dds"):
                src = os.path.join(current_root, name)
                dst = os.path.join(root_dir, name)

                # Skip if source is already in root
                if os.path.abspath(src) == dst:
                    continue

                shutil.copy2(src, dst)

def main():
    parser = argparse.ArgumentParser(description="Copy all .dds files from subdirectories into the root directory")
    parser.add_argument("directory", help="Root directory path")
    args = parser.parse_args()

    collect_dds(args.directory)

if __name__ == "__main__":
    main()
