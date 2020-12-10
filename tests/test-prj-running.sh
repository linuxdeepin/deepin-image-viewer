rm -r build-ut
rm -r ../build-ut
mkdir ../build-ut
cd ../build-ut
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j8

#workdir=$(cd ../$(dirname $0)/tests/build-ut; pwd)
#executable=deepin-image-viewer-test #可执行程序的文件名
#build_dir=$workdir
#result_coverage_dir=$build_dir/html
#result_report_dir=$build_dir/report/report_ut_test_time.xml
#$build_dir/tests/$executable --gtest_output=xml:$result_report_dir

#lcov -d $build_dir -c -o $build_dir/coverage.info

#lcov --extract $build_dir/coverage.info $extract_info --output-file  $build_dir/coverage.info
#lcov --remove $build_dir/coverage.info $remove_info --output-file $build_dir/coverage.info

#lcov --list-full-path -e $build_dir/coverage.info –o $build_dir/coverage-stripped.info

#genhtml -o $result_coverage_dir $build_dir/coverage.info

#cp $build_dir/report/report_ut_record_time.xml ./build-ut/report/report_ut_test_time.xml
#cp $build_dir/html/index.html ./html/cov_ut_test_time.html
#cp $build_dir/asan_ut_record_time.log.* ./build-ut/asan_ut_test_time.log  

workdir=$(cd ../$(dirname $0)/build-ut; pwd)
PROJECT_NAME=deepin-image-viewer-test #可执行程序的文件名
PROJECT_REALNAME=deepin-image-viewer  #项目名称
mkdir -p html
mkdir -p coverageResult
mkdir -p tests/test
     echo " ===================CREAT LCOV REPROT==================== "
     mkdir ./CMakeFiles/${PROJECT_REALNAME}.dir
     lcov --directory ./CMakeFiles/${PROJECT_REALNAME}.dir --zerocounters
     ./tests/${PROJECT_NAME}
     lcov --directory . --capture --output-file ./html/${PROJECT_REALNAME}_Coverage.info

     echo " =================== do filter begin ==================== "
     lcov --remove ./html/${PROJECT_REALNAME}_Coverage.info 'CMakeFiles/${PROJECT_NAME}.dir/deepin-image-viewer-test_autogen/*/*' '${PROJECT_NAME}_autogen/*/*' 'googletest/*/*' '*/usr/include/*' '*/tests/*' '/usr/local/*' -o ./html/${PROJECT_REALNAME}_Coverage_fileter.info
     echo " =================== do filter end ==================== "

     genhtml -o ./html ./html/${PROJECT_REALNAME}_Coverage_fileter.info
     echo " -- Coverage files have been output to ${CMAKE_BINARY_DIR}/html "
     mv ./html/index.html ./html/cov_${PROJECT_REALNAME}.html
     mv asan.log* asan_${PROJECT_REALNAME}.log
#    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
exit 0