docker build -f jenkins/Dockerfile.dev -t pdnsim .
docker run -v -u $(id -u ${USER}):$(id -g ${USER}) -it $(pwd):/PDNSim pdnsim bash -c "./PDNSim/jenkins/install.sh"
