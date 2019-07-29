How to collect code coverage?
==============
-- code coverage in Linux
Prepare LCOV
1)	http://ltp.sourceforge.net/coverage/lcov.php
2)	sudo apt-get install lcov

Build EDKII test case
1)	Add below to INF to enable GCOV
  GCC:*_GCC5_*_CC_FLAGS = --coverage
  GCC:*_GCC5_*_DLINK_FLAGS = --coverage

Run LCOV/GCOV
1)	run the test app
e.g. ./Build/UefiHostFuzzTestCasePkg/DEBUG_GCC5/IA32/TestPartition

2)	Collect coverage data 
lcov --capture --directory project-dir --output-file coverage.info
e.g. lcov --capture --directory  ./Build/UefiHostFuzzTestCasePkg/DEBUG_GCC5/IA32 --output-file coverage.info

3)	Generate HTML output
genhtml coverage.info --output-directory out
e.g. genhtml coverage.info --output-directory out

4)	View html
out/index.html

A simple python script to run all seeds before collect coverage data
python run_all_seeds.py <tcs-relative-path> <findings_dir>
e.g "python Build/UefiHostFuzzTestCasePkg/DEBUG_GCC5/IA32/TestPatition /dev/shm/findings_dir"

-- code coverage in Windows
1) download DynamoRIO from https://github.com/DynamoRIO/dynamorio/wiki/Downloads
2) install Perl
3) Read http://dynamorio.org/docs/page_drcov.html
4) execute below:
   DynamoRIO-Windows-6.2.0-2\bin32>drrun.exe -c ..\tools\lib32\release\drcov.dll -- XXX.exe XXX.seed
      // this will generate drcov.XXX.exe.yyy.0000.proc.log
   DynamoRIO-Windows-6.2.0-2\bin32>..\tools\bin32\drcov2lcov.exe -input drcov.XXX.exe.yyy.0000.proc.log -src_filter c:\zzz\edkii\xxxpkg
      // NOTE: please use *all low cases* in -src_fileter.
      // NOTE: please put your *target code* in -src_filter. e.g. if code is in EDKII, use c:\edkii\...; if code is in HBFA, use c:\zzz\dfst\hbfa\...
      // this will generate coverage.info
   DynamoRIO-Windows-6.2.0-2\bin32>perl ..\tools\bin32\genhtml coverage_new.info
      // this will generate index.html
5) View html
