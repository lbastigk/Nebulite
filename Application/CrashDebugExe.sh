#!/bin/bash

WINEDEBUG=+seh,+tid,+loaddll,+warn+all wine ./bin/Nebulite_Debug.exe serialize &> wine_crash.log

