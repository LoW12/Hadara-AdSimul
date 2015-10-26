#!/bin/bash

echo "Generating png from dot files..."
for f in *.dot
do
  echo "Processing $f file..."
  dot -Tpng -o "${f%%.*}.png" "$f"
done
echo "Done."

