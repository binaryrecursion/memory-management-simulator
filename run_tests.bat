@echo off
echo Creating output folder...
if not exist output mkdir output

echo Clearing combined file...
echo. > all_tests_output.txt

echo Running Buddy test...
echo ===== BUDDY TEST ===== >> all_tests_output.txt
memsim.exe < test\buddy_test.txt > output\buddy_log.txt
type output\buddy_log.txt >> all_tests_output.txt

echo Running Linear allocation test...
echo ===== LINEAR ALLOCATION TEST ===== >> all_tests_output.txt
memsim.exe < test\linear_allocation_test.txt > output\linear_log.txt
type output\linear_log.txt >> all_tests_output.txt

echo Running Compare allocation test...
echo ===== COMPARE ALLOCATION TEST ===== >> all_tests_output.txt
memsim.exe < test\compare_allocation_test.txt > output\compare_log.txt
type output\compare_log.txt >> all_tests_output.txt

echo Running Cache access test...
echo ===== CACHE ACCESS TEST ===== >> all_tests_output.txt
memsim.exe < test\cache_access_test.txt > output\cache_log.txt
type output\cache_log.txt >> all_tests_output.txt

echo Running VM access test...
echo ===== VM ACCESS TEST ===== >> all_tests_output.txt
memsim.exe < test\vm_access_test.txt > output\vm_log.txt
type output\vm_log.txt >> all_tests_output.txt

echo All test done
echo combined output with logs is in all_test_output.txt file
echo individual test output with logs is in output folder.
pause
