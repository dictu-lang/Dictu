## Dictu Docker

### Building the Docker image

Make sure you have Docker installed on your system. Refer to [Docker installation](https://docs.docker.com/engine/install/). 

To build the Docker image of Dictu, clone the repository and change directory into `Dictu`. 

Run the following command from the **root of the project** i.e, the `Dictu` folder by default. 

To build the *Alpine* version of Dictu - 

```bash 
$ docker build -t dictu:alpine -f Docker/DictuAlpineDockerfile . 
``` 

To build the *Ubuntu* version of Dictu -

```bash 
$ docker build -t dictu:ubuntu -f Docker/DictuUbuntuDockerfile . 
``` 

To start a REPL from this image, run - (Replace the tag with the appropriate version, i.e, alpine or ubuntu)

```bash 
$ docker run -it dictu:ubuntu 
```

> The image built can be used in other docker images as a base image for running dictu files. Make sure you have the image built locally.

```Dockerfile 
FROM dictu:ubuntu 
COPY example.du . 
RUN dictu example.du
```
