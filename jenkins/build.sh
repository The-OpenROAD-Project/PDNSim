docker build -f jenkins/Dockerfile.dev -t pdnsim .
docker run -u $(id -u ${USER}):$(id -g ${USER}) -v $(pwd):/PDNSim pdnsim bash -c "./PDNSim/jenkins/install.sh"
