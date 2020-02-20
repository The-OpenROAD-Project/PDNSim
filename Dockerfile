FROM centos:centos7 AS base-dependencies

# Install dev and runtime dependencies
RUN yum group install -y "Development Tools" \
    && yum install -y https://centos7.iuscommunity.org/ius-release.rpm \
    && yum install -y wget git centos-release-scl devtoolset-8 \
    devtoolset-8-libatomic-devel boost-devel tcl-devel tcl tk libstdc++ tk-devel pcre-devel \
    python36u python36u-libs python36u-devel python36u-pip && \
    yum clean -y all && \
    rm -rf /var/lib/apt/lists/*

# Install CMake
RUN wget https://cmake.org/files/v3.14/cmake-3.14.0-Linux-x86_64.sh && \
    chmod +x cmake-3.14.0-Linux-x86_64.sh  && \
    ./cmake-3.14.0-Linux-x86_64.sh --skip-license --prefix=/usr/local && rm -rf cmake-3.14.0-Linux-x86_64.sh \
    && yum clean -y all

# Install epel repo
RUN wget https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm && \
    yum install -y epel-release-latest-7.noarch.rpm && rm -rf epel-release-latest-7.noarch.rpm  \
    && yum clean -y all

RUN yum install -y wget rh-mongodb32-boost-devel rh-mongodb32-boost-static

ENV PATH=/opt/rh/rh-mongodb32/root/usr/bin:$PATH \
    LD_LIBRARY_PATH=/opt/rh/rh-mongodb32/root/usr/lib64:/opt/rh/rh-mongodb32/root/usr/lib:/opt/rh/rh-mongodb32/root/usr/lib64/dyninst:/opt/rh/rh-mongodb32/root/usr/lib/dyninst:/opt/rh/rh-mongodb32/root/usr/lib64:/opt/rh/rh-mongodb32/root/usr/lib:$LD_LIBRARY_PATH \
    C_INCLUDE_PATH=/opt/rh/rh-mongodb32/root/usr/include \
    CPLUS_INCLUDE_PATH=/opt/rh/rh-mongodb32/root/usr/include

RUN yum install boost-program-options
# Install SWIG
RUN yum remove -y swig \
    && wget https://github.com/swig/swig/archive/rel-4.0.1.tar.gz \
    && tar xfz rel-4.0.1.tar.gz \
    && rm -rf rel-4.0.1.tar.gz \
    && cd swig-rel-4.0.1 \
    && ./autogen.sh && ./configure --prefix=/usr && make -j $(nproc) && make install \
    && cd .. \
    && rm -rf swig-rel-4.0.1

# Temporarily add boost till all dependent tools are updated..
RUN yum install -y boost-devel && \
    yum clean -y all && \
    rm -rf /var/lib/apt/lists/*



# Install dev and runtime dependencies
RUN yum install -y qt3-devel itcl-devel ksh qt3 \
    itcl iwidgets blt tcllib bwidget


FROM base-dependencies AS builder

COPY . /PDNSim
WORKDIR /PDNSim

# Build
RUN mkdir build
RUN cd build && cmake .. && make
