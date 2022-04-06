#!/bin/bash

TEMPDIR="../LocalTimeRK-Temp"
DIRS=("automated-test" "more-examples")

mkdir $TEMPDIR || set status 0

for d in "${DIRS[@]}"; do
    echo $d
    mv $d $TEMPDIR
done

particle library upload

mv $TEMPDIR/* .
rmdir $TEMPDIR