import os
import re

HEADER_FILES = [
    "include/controlly/arena.h",
    "include/controlly/core.h",
    "include/controlly/matrix.h",
    "include/controlly/tf.h",
    "include/controlly/statespace.h",
    "include/controlly/controllers/pid.h",
]

SOURCE_FILES = [
    "src/internal_common.h",
    "src/statespace/ss_internal.h",
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

INTERNAL_MACROS = [
    "CONTROL_UNUSED",
    "CONTROL_ALIGN_UP",
    "CONTROL_THROW",
    "CONTROL_REQUIRE",
    "CONTROL_CHECK_CTX",
    "CONTROL_CHECK_OUT",
    "CONTROL_CHECK_NOT_NULL",
    "CONTROL_TRY",
]


def strip_includes(content):
    content = re.sub(r'#include\s+[<"]controlly/[^>"]+[>"]', "", content)
    content = re.sub(r'#include\s+"[^"]+"', "", content)
    return content


with open(OUTPUT_FILE, "w") as out:
    out.write("/* Controlly - Automated Header Library Conversion */\n\n")

    for header in HEADER_FILES:
        with open(header, "r") as f:
            out.write("/*\n * ========================================\n")
            out.write(f" * {os.path.basename(header)}\n")
            out.write(" * ========================================\n*/\n\n")
            out.write(strip_includes(f.read()) + "\n")

    out.write("\n#ifdef CONTROLLY_IMPLEMENTATION\n\n")
    for source in SOURCE_FILES:
        with open(source, "r") as f:
            out.write("/*\n * ----------------------------------------\n")
            out.write(f" * {os.path.basename(source)}\n")
            out.write(" * ----------------------------------------\n*/\n\n")
            out.write(strip_includes(f.read()) + "\n")

    for macro in INTERNAL_MACROS:
        out.write(f"#undef {macro}\n")

    out.write("\n#endif /* CONTROLLY_IMPLEMENTATION */\n")


print(f"Generated {OUTPUT_FILE}")
