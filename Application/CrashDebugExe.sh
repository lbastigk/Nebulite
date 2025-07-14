#!/bin/bash
WINEDEBUG=+seh,+tid,+loaddll,+warn+all wine run_nebulite.bat &> wine_crash.log
