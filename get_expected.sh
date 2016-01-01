#!/bin/bash
for n in test?; do cd $n; ../mycrond -o -f ./testcron 2>&1 | cat > test.exp; cd ..; done

