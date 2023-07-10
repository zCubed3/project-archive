# Merges a SPIR-V vert-frag shader into a single file

import sys
import os

in_vert_path = sys.argv[1]
in_frag_path = sys.argv[2]
out_path = sys.argv[3]

vert_source = ""
frag_source = ""

with open(in_vert_path, "rb") as vert_src_file:
    vert_source = vert_src_file.read()

with open(in_frag_path, "rb") as frag_src_file:
    frag_source = frag_src_file.read()

with open(out_path, "wb") as merge_file:
    vert_len = len(vert_source)
    frag_len = len(frag_source)

    merge_file.write("MSPV".encode('ascii'))

    # The MSPV format expects descriptor for the stage type before reading it
    merge_file.write("VERT".encode('ascii'))
    merge_file.write(vert_len.to_bytes(4, byteorder="little"))
    merge_file.write(vert_source)

    merge_file.write("FRAG".encode('ascii'))
    merge_file.write(frag_len.to_bytes(4, byteorder="little"))
    merge_file.write(frag_source)

