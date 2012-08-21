#!/bin/sh

. ./_export_env.sh                              # setting environment variables

export TET_SUITE_ROOT=`pwd`
RESULT_DIR=results

tcc -c -p ./                                # executing tcc, with clean option (-c)
rm -r $RESULT_DIR
rm -r tet_tmp_dir
rm testcase/tet_captured
