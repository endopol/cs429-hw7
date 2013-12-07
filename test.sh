#!/bin/bash

# Make the directory to hold all the testing items
mkdir -p testfiles
cd testfiles

# Download all the testing files
files=( "full" "jan" "prime" "spover" )

for i in "${files[@]}"
do
    if [ ! -f $i.log ]; then
        wget -q http://www.cs.utexas.edu/users/peterson/prog7/$i.log
    fi
    if [ ! -f $i.obj ]; then
        wget -q http://www.cs.utexas.edu/users/peterson/prog7/$i.obj
    fi
    if [ ! -f $i.out ]; then
        wget -q http://www.cs.utexas.edu/users/peterson/prog7/$i.out
    fi
    if [ ! -f $i.in ]; then
        wget -q http://www.cs.utexas.edu/users/peterson/prog7/$i.in
    fi
done

cd ..

mkdir -p testoutput
make pdp429

errors=0

# Run the tests
for i in "${files[@]}"
do
    ./pdp429 -v ./testfiles/$i.obj < ./testfiles/$i.in > ./testoutput/$i.out 2> ./testoutput/$i.log

    diff ./testfiles/$i.out ./testoutput/$i.out > ./testoutput/$i.out.diff
    diff ./testfiles/$i.log ./testoutput/$i.log > ./testoutput/$i.log.diff

    if [[ -s ./testoutput/$i.out.diff ]]; then
        echo -e "\e[00;31m$i OUTPUT FAILED\e[00m"
        errors=$[$errors+1]
    fi
    if [[ -s ./testoutput/$i.log.diff ]]; then
        echo -e "\e[00;31m$i LOG OUTPUT FAILED\e[00m"
        errors=$[$errors+1]
    fi
done

if [[ "$errors" -gt 0 ]]; then
    echo -e "\e[00;31mTEST FAILED WITH $errors ERRORS\e[00m"
else
    echo -e "\e[00;32mALL TESTS PASSED\e[00m"
fi
