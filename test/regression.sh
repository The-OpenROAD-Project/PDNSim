set -e

#BASE_DIR=$(dirname $0)
APP=./irsolver
echo $BASE_DIR
#echo $APP

echo "Running tests .."
echo "================"

echo "[1] I"
#$APP -c 1 ../test/gcd_test.tcl > ../test/gcd_test.check
./irsolver --input-lef ../test/nangate45/Nangate45.lef --input-def \
../test/gcd/gcd.def --input-lib \
../test/nangate45/NangateOpenCellLibrary_typical.lib --input-verilog \
../test/gcd/gcd.v --input-sdc ../test/gcd/gcd.sdc --input-vsrc \
../test/gcd/Vsrc.loc --top-module gcd > ../test/gcd_test.check 


diff --brief ../test/gcd_test.check ../test/gcd_test.ok >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "FAIL!"
else
    echo "SUCCESS!"
fi
