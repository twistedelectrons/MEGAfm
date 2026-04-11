FROM ubuntu:22.04

RUN sed -i -e "s/# \(deb.*universe.*\)/\1/g" /etc/apt/sources.list
RUN apt-get update --yes
RUN apt-get install --yes python2 python3 python3-pip clang-format
RUN pip3 install -U platformio

WORKDIR /app
