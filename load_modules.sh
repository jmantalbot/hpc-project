#!/bin/bash

if [ "$(which "module")" != "" ];
then
  echo "loading modules..."
  module load gcc/13.1.0
  module load openmpi
  module load cmake
  echo "loaded."
else
  echo "'module' command not found -- not on CHPC."
fi