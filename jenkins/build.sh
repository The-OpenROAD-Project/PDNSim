docker build -t pdnsim .
docker run -it -v $(pwd):/PDNSim pdnsim bash
