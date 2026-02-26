#!/usr/bin/env bash
set -euo pipefail

if [ $# -eq 0 ] ; then
    >&2 echo 'No arguments supplied'
    echo "  Usage:"
    echo "    batch_syntheticScarVolume.sh [options] /path/to/data/*.txt"
    echo " "
    echo "  Each .txt file is treated as a seed coordinate file."
    echo "  The corresponding mesh is expected at the same path with a .vtk"
    echo "  extension (e.g. case1.txt -> case1.vtk)."
    echo "  Outputs are written next to the input mesh."
    echo " "
    echo "  Options (all optional, must come before file arguments):"
    echo "    -b <path>   Path to syntheticScarVolume binary (default: ./syntheticScarVolume)"
    echo "    -n <list>   Comma-separated neighbourhood sizes  (default: 10,15,20)"
    echo "    -f <list>   Comma-separated falloff modes 1=Gaussian 2=Linear (default: 1,2)"
    echo "    -s <list>   Comma-separated sigma values          (default: 3.0,7.0,15.0)"
    echo "    -i <path>    Input mesh file path                  (default: <input_file>.vtk)"
    exit 1
fi

# -----------------------------------------------------------------------
# -----------------------------------------------------------------------

BINARY="./syntheticScarVolume"
IFS=',' read -ra NEIGHBOURHOODS <<< "10,15,20"
IFS=',' read -ra FALLOFFS       <<< "1,2"
IFS=',' read -ra SIGMAS         <<< "3.0,7.0,15.0"
MESH_EXT="vtk"
INPUT_MESH="vtk"

# --- Parse options -----------------------------------------------------
while getopts ":b:n:f:s:e:i:" opt; do
    case $opt in
        b) BINARY="$OPTARG" ;;
        n) IFS=',' read -ra NEIGHBOURHOODS <<< "$OPTARG" ;;
        f) IFS=',' read -ra FALLOFFS       <<< "$OPTARG" ;;
        s) IFS=',' read -ra SIGMAS         <<< "$OPTARG" ;;
        i) INPUT_MESH="$OPTARG" ;;
        \?) >&2 echo "Unknown option: -$OPTARG"; exit 1 ;;
        :)  >&2 echo "Option -$OPTARG requires an argument"; exit 1 ;;
    esac
done
shift $((OPTIND - 1))

if [ $# -eq 0 ] ; then
    >&2 echo "No point files supplied after options."
    exit 1
fi

if [ ! -x "$BINARY" ]; then
    >&2 echo "Binary not found or not executable: $BINARY"
    exit 1
fi

if [ -z "$INPUT_MESH" ]; then
    >&2 echo "Input mesh required: -i <mesh>"
    exit 1
fi

if [ ! -f "$INPUT_MESH" ]; then
    >&2 echo "Input mesh not found: $INPUT_MESH"
    exit 1
fi

# --- Main loop ---------------------------------------------------------
total=$(( $# * ${#NEIGHBOURHOODS[@]} * ${#FALLOFFS[@]} * ${#SIGMAS[@]} ))
count=0

for pts_file in "$@"; do
    base="${pts_file%.*}"

    for n in "${NEIGHBOURHOODS[@]}"; do
        for falloff in "${FALLOFFS[@]}"; do
            for sigma in "${SIGMAS[@]}"; do
                count=$(( count + 1 ))
                output="${base}_n${n}_f${falloff}_s${sigma}.${MESH_EXT}"

                echo "[${count}/${total}] n=${n} falloff=${falloff} sigma=${sigma}"
                echo "  pts:  $pts_file"
                echo "  mesh: $INPUT_MESH"
                echo "  out:  $output"

                "$BINARY" \
                    -i "$INPUT_MESH" \
                    -pts "$pts_file" \
                    -o "$output" \
                    -n "$n" \
                    -falloff "$falloff" \
                    -sigma "$sigma"
            done
        done
    done
done

echo "Done. ${count} meshes generated."