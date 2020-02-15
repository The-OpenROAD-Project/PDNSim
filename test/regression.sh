echo "Running tests .."
echo "========================="

echo "======== TEST 1: ========"
./pdnsim --input-lef ../test/nangate45/Nangate45.lef --input-def \
../test/gcd/gcd.def --input-lib \
../test/nangate45/NangateOpenCellLibrary_typical.lib --input-verilog \
../test/gcd/gcd.v --input-sdc ../test/gcd/gcd.sdc --input-vsrc \
../test/gcd/Vsrc.loc --top-module gcd > ../test/gcd/gcd_test.check 

diff --brief ../test/gcd/gcd_test.check ../test/gcd/gcd_test.ok >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "FAIL!"
    echo "===== TEST 1: FAIL ======"
else
    echo "SUCCESS!"
    echo "===== TEST 1: PASS ======"
fi

echo "========================="

echo "======== TEST 2: ========"
./pdnsim --input-lef ../test/nangate45/Nangate45.lef --input-def \
../test/aes/aes.def --input-lib \
../test/nangate45/NangateOpenCellLibrary_typical.lib --input-verilog \
../test/aes/aes.v --input-sdc ../test/aes/aes.sdc --input-vsrc \
../test/aes/Vsrc.loc --top-module aes_cipher_top > ../test/aes/aes_test.check 

diff --brief ../test/aes/aes_test.check ../test/aes/aes_test.ok >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "FAIL!"
    echo "===== TEST 2: FAIL ======"
else
    echo "SUCCESS!"
    echo "===== TEST 2: PASS ======"
fi
