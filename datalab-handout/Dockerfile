FROM ubuntu:16.04

MAINTAINER PrabhSimran Singh Baweja

RUN apt-get update && apt-get -y install \
    make \
;

WORKDIR /home/auser/workspace

ADD datalab-handout /home/auser/workspace

CMD ["bash"]