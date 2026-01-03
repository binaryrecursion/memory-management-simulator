#!/bin/bash

echo "Creating output folder..."
mkdir -p output

echo "Clearing combined file..."
: > all_tests_output.txt

echo "Running Buddy test..."
echo "===== BUDDY TEST =====" >> all_tests_output.txt
./memsim < test/buddy_test.txt > output/buddy_log.txt
cat output/buddy_log.txt >> all_tests_output.txt

echo "Running Linear allocation test..."
echo "===== LINEAR ALLOCATION TEST =====" >> all_tests_output.txt
./memsim < test/linear_allocation_test.txt > output/linear_log.txt
cat output/linear_log.txt >> all_tests_output.txt

echo "Running Compare allocation test..."
echo "===== COMPARE ALLOCATION TEST =====" >> all_tests_output.txt
./memsim < test/compare_allocation_test.txt > output/compare_log.txt
cat output/compare_log.txt >> all_tests_output.txt

echo "Running Cache access test..."
echo "===== CACHE ACCESS TEST =====" >> all_tests_output.txt
./memsim < test/cache_access_test.txt > output/cache_log.txt
cat output/cache_log.txt >> all_tests_output.txt

echo "Running VM access test..."
echo "===== VM ACCESS TEST =====" >> all_tests_output.txt
./memsim < test/vm_access_test.txt > output/vm_log.txt
cat output/vm_log.txt >> all_tests_output.txt

echo "Done! All logs saved — nothing printed to screen."
