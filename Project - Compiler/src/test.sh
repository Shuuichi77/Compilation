#!/bin/bash
make
echo -n > test/tests_results.txt

test_directory() {
    files=$1*.tpc
    total_tests=$(ls $files | wc -l)
    test_succeed=0
    current_test_number=0

    for f in $files; 
    do
        let current_test_number++
        echo -e "- ($current_test_number/$total_tests) Testing \"$f\" :" | tee -a test/tests_results.txt
        ./bin/tpcc < $f 2>&1 | tee -a test/tests_results.txt

        if [ ${PIPESTATUS[0]} = 0 ]; then
            let test_succeed++
            echo -e "Result : Test succeed \n" | tee -a test/tests_results.txt
        else
            echo -e "Result : Test failed \n" | tee -a test/tests_results.txt
        fi
    done

    echo -e "TOTAL : ${test_succeed}/${total_tests} tests succeed in directory \"$1\"" | tee -a test/tests_results.txt
}

echo -e "\n______________________________________________________________\n"

echo -e "\n|--------------------- Good Tests ---------------------|\n" | tee -a test/tests_results.txt
test_directory "test/good/"

echo -e "\n|--------------------- Syntaxique Errors Tests ---------------------|\n" | tee -a test/tests_results.txt
test_directory "test/syn-err/"

echo -e "\n|--------------------- Semantique Errors Tests ---------------------|\n" | tee -a test/tests_results.txt
test_directory "test/sem-err/"

echo -e "\n|--------------------- Warnings Tests ---------------------|\n" | tee -a test/tests_results.txt
test_directory "test/warn/"

echo -e "\n______________________________________________________________\n"