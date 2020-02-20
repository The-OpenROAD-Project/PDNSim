docker run -u $(id -u ${USER}):$(id -g ${USER}) -v $(pwd):/PDNSim pdnsim bash -c "./PDNSim/test/regression.sh"
