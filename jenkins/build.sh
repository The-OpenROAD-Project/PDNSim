docker build -f jenkins/Dockerfile.dev -t pdnsim .
docker run -v -it $(pwd):/PDNSim pdnsim bash -c "./PDNSim/jenkins/install.sh"
