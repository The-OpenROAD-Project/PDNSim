echo "Running tests .."
echo "========================="

APP=./PDNSim/build/pdnsim

echo "======== TEST 1: ========"
$APP --input-lef ./PDNSim/test/nangate45/Nangate45.lef --input-def \
./PDNSim/test/gcd/gcd.def --input-lib \
./PDNSim/test/nangate45/NangateOpenCellLibrary_typical.lib --input-verilog \
./PDNSim/test/gcd/gcd.v --input-sdc ./PDNSim/test/gcd/gcd.sdc --input-vsrc \
./PDNSim/test/gcd/Vsrc.loc --top-module gcd > ./PDNSim/test/gcd/gcd_test.check 

diff --brief ./PDNSim/test/gcd/gcd_test.check ./PDNSim/test/gcd/gcd_test.ok >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "FAIL!"
    exit 1
    echo "===== TEST 1: FAIL ======"
else
    echo "SUCCESS!"
    echo "===== TEST 1: PASS ======"
fi

echo "========================="

echo "======== TEST 2: ========"
$APP --input-lef ../test/nangate45/Nangate45.lef --input-def \
./PDNSim/test/aes/aes.def --input-lib \
./PDNSim/test/nangate45/NangateOpenCellLibrary_typical.lib --input-verilog \
./PDNSim/test/aes/aes.v --input-sdc ./PDNSim/test/aes/aes.sdc --input-vsrc \
./PDNSim/test/aes/Vsrc.loc --top-module aes_cipher_top > ./PDNSim/test/aes/aes_test.check 

diff --brief ./PDNSim/test/aes/aes_test.check ./PDNSim/test/aes/aes_test.ok >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "FAIL!"
    echo "===== TEST 2: FAIL ======"
    exit 1
else
    echo "SUCCESS!"
    echo "===== TEST 2: PASS ======"
fi
