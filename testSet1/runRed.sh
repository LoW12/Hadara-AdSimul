#!/bin/bash

echo "Starting workflow reduction.."

for f in *.xml
do
  echo "Processing $f file..."
  ./../Hadara_AdSimul_Red "$f"
done


echo "Done."


