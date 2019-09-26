FROM gitpod/workspace-full

USER root

RUN apt-get update \
    && apt-get install -y libopenmpi-dev openmpi-bin openmpi-doc

ENV OMPI_MCA_btl_base_warn_component_unused 0

WoRKDIR /work/

RUN git clone --depth 1 https://github.com/wichtounet/mnist.git mnist



RUN wget http://bitbucket.org/eigen/eigen/get/3.3.7.zip \
    && unzip 3.3.7.zip \
    && rm 3.3.7.zip \
    && mv eigen-eigen-323c052e1731/ eigen


# Install custom tools, runtime, etc. using apt-get
# For example, the command below would install "bastet" - a command line tetris clone:
#
# RUN apt-get update \
#    && apt-get install -y bastet \
#    && apt-get clean && rm -rf /var/cache/apt/* && rm -rf /var/lib/apt/lists/* && rm -rf /tmp/*
#
# More information: https://www.gitpod.io/docs/42_config_docker/
