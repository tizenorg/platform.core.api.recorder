#!/bin/sh

. ./_export_target_env.sh                    # setting environment variables

export TET_SUITE_ROOT=`pwd`
FILE_NAME_EXTENSION=`date +%s`

RESULT_DIR=results
HTML_RESULT=$RESULT_DIR/exec-tar-result-$FILE_NAME_EXTENSION.html
JOURNAL_RESULT=$RESULT_DIR/exec-tar-result-$FILE_NAME_EXTENSION.journal

mkdir -p $RESULT_DIR

tcc -e -j $JOURNAL_RESULT -p ./
grw -c 3 -f chtml -o $HTML_RESULT $JOURNAL_RESULT
