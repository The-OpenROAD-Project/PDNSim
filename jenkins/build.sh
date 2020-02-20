docker build --target base-dependencies -t pdnsim .
docker run -u $(id -u ${USER}):$(id -g ${USER}) -v $(pwd):/PDNSim pdnsim bash -c "./PDNSim/jenkins/install.sh"
