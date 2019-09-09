FROM gitpod/workspace-full

USER root

RUN apt-get update \
    && apt-get install -y libopenmpi-dev openmpi-bin openmpi-doc
RUN OMPI_MCA_btl_base_warn_component_unused=0 \
    && export OMPI_MCA_btl_base_warn_component_unused
# Install custom tools, runtime, etc. using apt-get
# For example, the command below would install "bastet" - a command line tetris clone:
#
# RUN apt-get update \
#    && apt-get install -y bastet \
#    && apt-get clean && rm -rf /var/cache/apt/* && rm -rf /var/lib/apt/lists/* && rm -rf /tmp/*
#
# More information: https://www.gitpod.io/docs/42_config_docker/
