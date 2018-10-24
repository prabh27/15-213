FROM ubuntu:trusty
MAINTAINER Prabh Simran Singh Baweja

RUN \
  apt-get update && \
  apt-get -y install \
          vim \
          curl \
          valgrind \
          make \
          gcc \
          git-core && \
  rm -rf /var/lib/apt/lists/*

ENV HOME /root

WORKDIR /root

ADD shlab-handout.tar /lab
ADD tsh.c /lab


CMD ["bash"]

