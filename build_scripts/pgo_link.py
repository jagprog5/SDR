#!/usr/bin/env python3

import argparse
import subprocess
import shlex

def link(link_command_path: str):
    '''
    link_command_path: points to link.txt, generated by cmake
    '''
    with open(link_command_path, "r") as f:
        command = f.read()
    command = shlex.split(command)
    for i in reversed(range(len(command))):
        if command[i] == "-fprofile-generate":
            del command[i]
    # run it
    p = subprocess.run(command, stderr=subprocess.PIPE)
    print(p.stderr.decode(), end='')
    if p.returncode != 0:
        print("FAILED to link for pgo")
        print(command)
    return p.returncode

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="links a cmake target for a pgo second pass")
    parser.add_argument("link-command-path", type=str)
    args = parser.parse_args()
    exit(link(getattr(args, "link-command-path")))
