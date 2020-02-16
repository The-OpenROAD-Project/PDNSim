docker build -f jenkins/Dockerfile -t pdnsim .
docker run -it -v $(pwd):/PDNSim pdnsim bash -c "./PDNSim/jenkins/install.sh"
