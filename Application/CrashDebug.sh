#!/bin/bash
valgrind \
  --tool=memcheck \
  --track-origins=yes \
  --leak-check=no \
  --error-limit=no \
  ./bin/Nebulite task TaskFiles/obj_oob.txt
