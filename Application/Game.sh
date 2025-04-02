#!/bin/bash
konsole -e bash -c "
echo 'Testing arg passing into main...'
./bin/Engine foo bar foo bar a

echo ''
echo 'Testing foo call'
./bin/Engine foo

echo ''
echo 'Testing main entry'
./bin/Engine

echo ''
echo 'Done!'
bash
"


