import os
import re

HEADER_FILES = [
    "include/controlly/arena.h",
    "include/controlly/core.h",
    "include/controlly/matrix.h",
    "include/controlly/tf.h",
    "include/controlly/statespace.h",
    "include/controlly/controllers/pid.h",
    "src/internal_common.h",
    "src/statespace/ss_internal.h"
]

SOURCE_FILES = [
    "src/core.c",
    "src/arena.c",
    "src/matrix.c",
    "src/statespace/router.c",
    "src/statespace/mimo.c",
    "src/statespace/siso.c",
    "src/tf.c",
    "src/controllers/pid.c",
]

OUTPUT_FILE = "dist/controlly.h"


def strip_includes(content):
    content = re.sub(r'#include\s+[<"]controlly/[^>"]+[>"]', "", content)
    content = re.sub(r'#include\s+"[^"]+"', "", content)
    return content


with open(OUTPUT_FILE, "w") as out:
    out.write("/* Controlly - Automated Header Library Conversion */\n\n")

    for header in HEADER_FILES:
        with open(header, "r") as f:
            out.write(strip_includes(f.read()) + "\n")

    out.write("\n#ifdef CONTROLLY_IMPLEMENTATION\n\n")
    for source in SOURCE_FILES:
        with open(source, "r") as f:
            out.write(f"/* -- {os.path.basename(source)} -- */\n")
            out.write(strip_includes(f.read()) + "\n")

    out.write("\n#endif /* CONTROLLY_IMPLEMENTATION */\n")


print(f"Generated {OUTPUT_FILE}")
