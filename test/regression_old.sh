echo "Running tests .."
echo "========================="

APP=./PDNSim/build/pdnsim

echo "======== TEST 1: ========"
$APP ./PDNSim/test/gcd/gcd_test.tcl | tee ./PDNSim/test/gcd/gcd_test.check 

diff --brief ./PDNSim/test/gcd/gcd_test.check ./PDNSim/test/gcd/gcd_test.ok >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "FAIL!"
    echo "===== TEST 1: FAIL ======"
    exit 1
else
    echo "SUCCESS!"
    echo "===== TEST 1: PASS ======"
fi

echo "========================="

echo "======== TEST 2: ========"
$APP ./PDNSim/test/aes/aes_test.tcl | tee ./PDNSim/test/aes/aes_test.check 

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
