#!/usr/bin/env bash
set -euo pipefail

if [ $# -eq 0 ] ; then
    >&2 echo 'No arguments supplied'
    echo "  Usage:"
    echo "    parallel_syntheticScarVolume.sh [options] /path/to/data/*.txt"
    echo " "
    echo "  Parallel version of batch_syntheticScarVolume.sh using GNU parallel."
    echo "  Each .txt file is treated as a seed coordinate file."
    echo "  The corresponding mesh is expected at the same path with a .vtk"
    echo "  extension (e.g. case1.txt -> case1.vtk)."
    echo "  Outputs are written next to the input mesh."
    echo " "
    echo "  Requires: GNU parallel (brew install parallel / apt install parallel)"
    echo " "
    echo "  Options (all optional, must come before file arguments):"
    echo "    -b <path>   Path to syntheticScarVolume binary (default: ./syntheticScarVolume)"
    echo "    -n <list>   Comma-separated neighbourhood sizes  (default: 10,15,20)"
    echo "    -f <list>   Comma-separated falloff modes 1=Gaussian 2=Linear (default: 1,2)"
    echo "    -s <list>   Comma-separated sigma values          (default: 3.0,7.0,15.0)"
    echo "    -i <ext>    Input volumetric mesh                 (default: vtk)"
    echo "    -j <int>    Number of parallel jobs               (default: 8)"
    
    exit 1
fi

# -----------------------------------------------------------------------
# -----------------------------------------------------------------------

BINARY="./syntheticScarVolume"
IFS=',' read -ra NEIGHBOURHOODS <<< "10,15,20"
IFS=',' read -ra FALLOFFS       <<< "1,2"
IFS=',' read -ra SIGMAS         <<< "3.0,7.0,15.0"
INPUT_MESH=""
MESH_EXT="vtk"
JOBS=$(( $(nproc) - 1 ))  # Use all but one CPU core by default

# --- Parse options -----------------------------------------------------
while getopts ":b:n:f:s:e:j:i:" opt; do
    case $opt in
        b) BINARY="$OPTARG" ;;
        n) IFS=',' read -ra NEIGHBOURHOODS <<< "$OPTARG" ;;
        f) IFS=',' read -ra FALLOFFS       <<< "$OPTARG" ;;
        s) IFS=',' read -ra SIGMAS         <<< "$OPTARG" ;;
        i) INPUT_MESH="$OPTARG" ;;
        j) JOBS="$OPTARG" ;;
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

if ! command -v parallel &>/dev/null; then
    >&2 echo "GNU parallel not found. Install with: brew install parallel"
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

# --- Build job list ----------------------------------------------------
# Write all combinations to a temp file so parallel can consume them.
# Format: pts_file|mesh|output|n|falloff|sigma
JOBLIST=$(mktemp)
trap 'rm -f "$JOBLIST"' EXIT

for pts_file in "$@"; do
    base="${pts_file%.*}"
    
    for n in "${NEIGHBOURHOODS[@]}"; do
        for falloff in "${FALLOFFS[@]}"; do
            for sigma in "${SIGMAS[@]}"; do
                output="${base}_n${n}_f${falloff}_s${sigma}.${MESH_EXT}"
                echo "${pts_file}|${INPUT_MESH}|${output}|${n}|${falloff}|${sigma}"
            done
        done
    done
done > "$JOBLIST"

total=$(wc -l < "$JOBLIST")
echo "Submitting ${total} jobs across ${JOBS} workers."

# --- Run ---------------------------------------------------------------
run_one() {
    local binary="$1"
    local line="$2"

    IFS='|' read -r pts_file input_mesh output n falloff sigma <<< "$line"

    echo "n=${n} falloff=${falloff} sigma=${sigma} -> $(basename "$output")"
    "$binary" \
        -i "$input_mesh" \
        -pts "$pts_file" \
        -o "$output" \
        -n "$n" \
        -falloff "$falloff" \
        -sigma "$sigma"
}

export -f run_one

parallel -j "$JOBS"  \
    run_one "$BINARY" {} \
    :::: "$JOBLIST"

echo "Done. ${total} meshes generated."