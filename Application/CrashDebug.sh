#!/bin/bash


taskname=TaskFiles/gravity_classic.txt
#taskname=TaskFiles/obj_oob.txt

valgrind \
  --tool=memcheck \
  --track-origins=yes \
  --leak-check=no \
  --error-limit=no \
  ./bin/Nebulite task $taskname
