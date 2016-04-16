#!/usr/bin/env python

import os
import sys
import subprocess
import shutil
from fwoptions import *

# Error codes
FIRMWARE_SIZE_TOO_BIG = 1
COMPILATION_ERROR = 4
INVALID_FIRMWARE = 5
INVALID_BOARD = 6
INVALID_LANGUAGE = 7

# Board types
BOARD_9X = 0
BOARD_GRUVIN9X = 1
BOARD_SKY9X = 2
BOARD_TARANIS = 3

# Board families
BOARD_FAMILY_AVR = 0
BOARD_FAMILY_ARM = 1

if len(sys.argv) != 3:
    exit(INVALID_FIRMWARE)

what = sys.argv[1]
directory, filename = os.path.split(sys.argv[2])
root, ext = os.path.splitext(filename)
options = root.split("-")

if len(options) < 2 or options[0] != "opentx":
    exit(INVALID_FIRMWARE)

optcount = 1
command_options = {}

if options[optcount] == "9x":
    command_options["PCB"] = "9X"
    firmware_options = options_9x
    maxsize = 65536
    board = BOARD_9X
    board_family = BOARD_FAMILY_AVR
elif options[optcount] == "9xr":
    command_options["PCB"] = "9XR"
    firmware_options = options_9x
    maxsize = 65536
    board = BOARD_9X
    board_family = BOARD_FAMILY_AVR
elif options[optcount] == "9x128":
    command_options["PCB"] = "9X128"
    command_options["FRSKY_STICKS"] = "YES"
    firmware_options = options_9x128
    maxsize = 65536 * 2
    board = BOARD_9X
    board_family = BOARD_FAMILY_AVR
elif options[optcount] == "9xr128":
    command_options["PCB"] = "9XR128"
    command_options["FRSKY_STICKS"] = "YES"
    firmware_options = options_9x128
    maxsize = 65536 * 2
    board = BOARD_9X
    board_family = BOARD_FAMILY_AVR
elif options[optcount] == "gruvin9x":
    command_options["PCB"] = "GRUVIN9X"
    command_options["EXT"] = "FRSKY"
    firmware_options = options_gruvin9x
    maxsize = 65536 * 4
    board = BOARD_GRUVIN9X
    board_family = BOARD_FAMILY_AVR
elif options[optcount] == "mega2560":
    command_options["PCB"] = "MEGA2560"
    command_options["EXT"] = "FRSKY"
    firmware_options = options_mega2560
    maxsize = 65536 * 4
    board = BOARD_GRUVIN9X
    board_family = BOARD_FAMILY_AVR
elif options[optcount] == "sky9x":
    command_options["PCB"] = "SKY9X"
    firmware_options = options_sky9x
    maxsize = 65536 * 4
    board = BOARD_SKY9X
    board_family = BOARD_FAMILY_ARM
elif options[optcount] == "9xrpro":
    command_options["PCB"] = "9XRPRO"
    command_options["SDCARD"] = "YES"
    firmware_options = options_sky9x
    maxsize = 65536 * 4
    board = BOARD_SKY9X
    board_family = BOARD_FAMILY_ARM
elif options[optcount] == "ar9x":
    command_options["PCB"] = "AR9X"
    command_options["SDCARD"] = "YES"
    command_options["HARDWARE_VOLUME"] = "NO"
    firmware_options = options_ar9x
    maxsize = 65536 * 4
    board = BOARD_SKY9X
    board_family = BOARD_FAMILY_ARM
elif options[optcount] == "taranis":
    command_options["PCB"] = "TARANIS"
    firmware_options = options_taranis
    maxsize = 65536 * 8
    board = BOARD_TARANIS
    board_family = BOARD_FAMILY_ARM
elif options[optcount] == "taranisplus":
    command_options["PCB"] = "TARANIS"
    command_options["PCBREV"] = "REVPLUS"
    firmware_options = options_taranisplus
    maxsize = 65536 * 8
    board = BOARD_TARANIS
    board_family = BOARD_FAMILY_ARM
elif options[optcount] == "taranisx9e":
    command_options["PCB"] = "TARANIS"
    command_options["PCBREV"] = "REV9E"
    firmware_options = options_taranisx9e
    maxsize = 65536 * 8
    board = BOARD_TARANIS
    board_family = BOARD_FAMILY_ARM
else:
    exit(INVALID_BOARD)

if what == "firmware":
    if board_family == BOARD_FAMILY_ARM:
        ext = ".bin"
    else:
        ext = ".hex"
    target = "opentx" + ext
    filename = "opentx"
else:
    exit(INVALID_BOARD)

filename += "-" + options[optcount]
optcount += 1

# The firmware options
for opt, value in firmware_options.items():
    found = False
    for i in range(optcount, len(options)):
        if options[i] == opt:
            found = True
            break

    if found:
        optvalue = value[1];
        filename += "-" + opt;
    else:
        optvalue = value[2]

    if optvalue is not None:
        command_options[value[0]] = optvalue

# The firmware display language
language = ""
for key in languages:
    if key == options[-1]:
        language = key
if not language:
    exit(INVALID_LANGUAGE)
command_options["TRANSLATIONS"] = language.upper()

filename += "-" + language + ext
srcdir = os.path.dirname(os.path.realpath(__file__)) + "/../.."
path = os.path.join(directory, filename)
outpath = path + ".out"
errpath = path + ".err"

if os.path.isfile(errpath):
    exit(COMPILATION_ERROR)

if not os.path.isfile(path):
    # Copy the radio directory locally
    shutil.copytree(srcdir, "code")

    # Launch make
    os.chdir("code/radio/src")
    cmd = ["make"]
    for opt, value in command_options.items():
        cmd.append("%s=%s" % (opt, value))
    if "OPENTX_VERSION_SUFFIX" in os.environ:
        cmd.append('VERSION_SUFFIX="%s"' % os.environ["OPENTX_VERSION_SUFFIX"])
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    output, error = proc.communicate()
    if proc.returncode == 0:
        file(outpath, "a").write(output + error)
    else:
        file(errpath, "w").write(output + error)
        exit(COMPILATION_ERROR)

    # Check binary size
    if board_family == BOARD_FAMILY_ARM:
        size = os.stat(target).st_size
    else:
        size = subprocess.check_output('avr-size -A %s | grep Total | cut -f2- -d " "' % target, shell=True)
        size = int(size.strip())
    if size > maxsize:
        exit(FIRMWARE_SIZE_TOO_BIG)

    # Copy binary to the binaries directory
    shutil.copyfile(target, path)

print filename
exit(0)
