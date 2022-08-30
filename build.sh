#!/usr/bin/env bash
#
# Copyright (C) 2021-2022 Intel Corporation
# SPDX-License-Identifier: MIT
# @file build.sh
#


WORK=`dirname "$0"`
WORK_DIR=`cd ${WORK} && pwd`

echo "build distribution package"
cd ${WORK_DIR}
if [ -d build ]; then
    for f in $( ls -a build )
    do
        if [ x"$f" != x"." ] && [ x"$f" != x".." ] \
            && [ x"$f" != x"hwloc" ] \
            && [ x"$f" != x"third_party" ] \
            && [ x"$f" != x"lib" ]; then
            rm -rf build/$f
        fi
    done
    echo "build folder exist."
else
    mkdir build
fi
cd build
cmake .. -DDAEMONLESS=True $@
make -j

echo "---------Create installation package-----------"
cpack   

if [ -f ~/password.sys_dcm ]; then
    PackageName=$(cat package_file_name)
    CSUser="ccr\\sys_dcm"
    CSPwd=$(cat ~/password.sys_dcm)
    echo "SignFile:${PackageName}" 
    "${WORK_DIR}"/install/tools/signfile/SignFile -vv -u "${CSUser}" -p "${CSPwd}" ${PackageName}
fi
