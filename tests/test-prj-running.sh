#!/bin/bash

# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set -e

export builddir=build
export reportdir=build-ut
# 记录脚本所在目录的绝对路径（后续 cd 到 build 目录后仍可引用）
scriptdir=$(cd $(dirname $0); pwd)
export projectdir=$(cd "$scriptdir/.."; pwd)
rm -rf $builddir
rm -rf ../$builddir
rm -rf $reportdir
rm -rf ../$reportdir
mkdir ../$builddir
mkdir ../$reportdir
cd ../$builddir
#编译
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_SAFETYTEST_ARG="CMAKE_SAFETYTEST_ARG_ON" ..
make -j$(nproc)
#生成asan日志和ut测试xml结果
# 设置 ASAN/LSan 选项：抑制 Qt6 DBus 框架级的静态资源残留泄漏报告
# （详见 tests/lsan_suppressions.txt）
export ASAN_OPTIONS=${ASAN_OPTIONS:-detect_leaks=1}
export LSAN_OPTIONS=suppressions=$scriptdir/lsan_suppressions.txt

# 运行测试并生成 gtest XML 报告
GTEST_XML_DIR="$projectdir/$builddir/report"
mkdir -p "$GTEST_XML_DIR"
set +e
./tests/deepin-image-viewer-test --gtest_output="xml:$GTEST_XML_DIR/report_deepin-image-viewer.xml"
test_exit_code=$?
set -e

workdir=$(cd ../$(dirname $0)/$builddir; pwd)

mkdir -p report
#统计代码覆盖率并生成html报告
lcov -d $workdir -c -o ./coverage.info

lcov --extract ./coverage.info '*/src/*' -o ./coverage.info

lcov --remove ./coverage.info '*/tests/*' -o ./coverage.info

genhtml -o ./html ./coverage.info

mv ./html/index.html ./html/cov_deepin-image-viewer.html
#对asan、ut、代码覆盖率结果收集至指定文件夹
cp -r html ../$reportdir/
cp -r report ../$reportdir/
# 收集 ASAN 日志（若存在）；测试无崩溃时不会产生 asan.log，忽略缺失
cp -r asan*.log* ../$reportdir/asan_deepin-image-viewer.log 2>/dev/null || true

# 生成摘要 JSON
SUMMARY_JSON="$projectdir/$reportdir/ut-summary.json"
echo "==> Generating summary JSON: ${SUMMARY_JSON}"

python3 "$scriptdir/gen-ut-summary.py"

exit $test_exit_code
