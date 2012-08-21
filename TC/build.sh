#!/bin/sh

. ./_export_env.sh                              # setting environment variables

export TET_SUITE_ROOT=`pwd`
FILE_NAME_EXTENSION=`date +%s`

RESULT_DIR=results
HTML_RESULT=$RESULT_DIR/build-tar-result-$FILE_NAME_EXTENSION.html
JOURNAL_RESULT=$RESULT_DIR/build-tar-result-$FILE_NAME_EXTENSION.journal

mkdir -p $RESULT_DIR

tcc -c -p ./
tcc -b -j $JOURNAL_RESULT -p ./
grw -c 7 -f chtml -o $HTML_RESULT $JOURNAL_RESULT
