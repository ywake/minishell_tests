#!/bin/bash

test_echo() {
	./a.out $1 >>output
	echo $1 >>expect
}

test_res_print() {
	if [ $1 -eq 0 ]; then
		printf "\e[32m%s\n\e[m" "unit test OK!"
	else
		printf "\e[31m%s\n\e[m" "unit test KO!"
	fi
}

DIR=$(dirname "$0")

gcc $DIR/test.c \
	$(find $REPO_ROOT/srcs -type f -name echo.c) \
	$SHARED_LIB \
	$INCLUDES $LIBS -lft -lex

touch expect output
test_echo "test"
test_echo "test aaaaa rydydxgsfhsdf sdfhgsdth  dfshsrdh  dfgh werysreh fdsh sdht  dfsgdrsh se dsrrh srth "
test_echo "a"
test_echo "-n a"
test_echo "-n a"
test_echo "-n a"
test_echo "-n a"
test_echo "-n"
test_echo "-n a"
test_echo "-na a s df"
test_echo "-naa a sg"
test_echo "a -naa a sdfsd"
test_echo "a -n a sdfsd"
test_echo "a - a sdfsd"
# test_echo "- n a - a sdfsd"
# ダブルクオテーションで括って渡すと、-の解釈のされ方が変わるため、このテストケースをエスケープ
test_echo "-n a ljkjkl lkfjhioagjhio radggjriprehn  l;sjgkkl;sdgjladjrt hipj afdslghkeatl; "
test_echo "a ljkjkl lkfjhioagjhio radggjrip -nrehn   -n l;sjgkkl;sdgjladjrt hipj afdslghkeatl; "
test_echo "あいうえお"
test_echo "-n あいうえお"
test_echo "あいうえお -n かきくけこ"
test_echo "455555555555555555555555555555555555555555"
test_echo "-n               sdg                            f gdf                  f"
test_echo "'"
test_echo "'"
test_echo "'aaaaaaa    '  aaaaaaaaaaaaa' "
test_echo '    s  ffffffff f dg '
test_echo "'-n test'     test"

echo "========= test1 ========="
diff expect output
RES1=$?
test_res_print $RES1

gcc $DIR/test2.c \
	$(find $REPO_ROOT/srcs -type f -name echo.c) \
	$SHARED_LIB \
	$INCLUDES $LIBS -lft -lex

echo "========= test2 ========="
./a.out > output2
diff $DIR/expect2 output2
RES2=$?
test_res_print $RES2

echo "===================================="

rm expect output output2 a.out leaksout

if [ $RES1 -eq 1 ] || [ $RES2 -eq 1 ]; then
	exit 1
fi
