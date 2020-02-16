docker build -f jenkins/Dockerfile.dev -t pdnsim .
docker run -it -v $(pwd):/PDNSim pdnsim bash -c "./PDNSim/jenkins/install.sh"
