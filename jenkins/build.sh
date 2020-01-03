docker build --target base-dependencies -t openira .
docker run -it -v $(pwd):/OpenIRA openira bash
