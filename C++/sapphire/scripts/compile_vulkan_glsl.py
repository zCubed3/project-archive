# Assistant script for compiling GLSL vert+frag files for Vulkan

# The first argument is the output
# The rest are input files (in order of inclusion)

import os
import sys
import subprocess

out_path = sys.argv[1]
stage = sys.argv[2]

temp_merge_path = out_path + ".merge_temp.glsl"

source = ""
for i in range(3, len(sys.argv)):
    with open(sys.argv[i], 'r') as src_file:
        source += src_file.read()

with open(temp_merge_path, "w") as merge_file:
    merge_file.write(source)
    merge_file.write("\n\n")

subprocess.run(["glslc", f"-fshader-stage={stage}", "-o", out_path, temp_merge_path])
os.remove(temp_merge_path)