#!/bin/bash

ulimit -n 1024  # Set max open files to 1024

taskname=TaskFiles/crash_test.txt
#taskname=TaskFiles/obj_oob.txt

valgrind \
  --tool=memcheck \
  --track-origins=yes \
  --leak-check=no \
  --error-limit=no \
  ./bin/Nebulite_Debug task $taskname
