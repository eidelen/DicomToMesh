#!/bin/bash
set -euo pipefail

# Variables
PPA="ppa:eidelen/d2m"
PACKAGE="dicom2mesh"
OUTPUT_FILE="torusgen.stl"  # <-- Replace with your actual expected output filename
INPUT_FILE="lib/test/data/torus.obj"          # <-- Provide a suitable test input file path here

# Step 1: Add PPA
echo "Adding PPA: $PPA"
sudo apt-get update -qq
sudo apt-get install -y --no-install-recommends software-properties-common
sudo add-apt-repository -y "$PPA"

# sudo apt-get install -y build-essential cmake libglvnd-dev libproj-dev libvtk9-dev libvtk9-qt-dev qt5-qmake qtbase5-dev-tools

# Step 2: Install dicom2mesh
echo "Installing $PACKAGE"
sudo apt-get update -qq
sudo apt-get install -y "$PACKAGE"

# Step 3: Run the tool
echo "Running dicom2mesh..."
dicom2mesh -i "$INPUT_FILE" -o "$OUTPUT_FILE"  # <-- You will adjust this command line as needed

# Step 4: Check output
if [ -f "$OUTPUT_FILE" ]; then
    echo "✅ Output file '$OUTPUT_FILE' was successfully generated."
    exit 0
else
    echo "❌ Output file '$OUTPUT_FILE' was not found."
    exit 1
fi

