FROM k0kubun/llvm35
MAINTAINER Takashi Kokubun <takashikkbn@gmail.com>

RUN apt-get install -qq flex bison libc6-dev-i386

COPY . /app
WORKDIR /app
RUN make
