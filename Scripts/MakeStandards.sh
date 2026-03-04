#!/bin/bash

./bin/Nebulite standard-file render-object || { echo "Failed to create RenderObject.standard"; exit 1; }
./bin/Nebulite settings save-standards || { echo "Failed to create Settings.standard"; exit 1; }
