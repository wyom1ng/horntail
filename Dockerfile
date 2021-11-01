FROM conanio/gcc11 as builder

WORKDIR /usr/app

COPY . .

USER root

RUN apt update
RUN apt -y install git cmake uuid-dev libbrotli-dev libmariadb-dev libsqlite3-dev libssl-dev zlib1g-dev

WORKDIR /usr/app/third_party/jsoncpp
RUN git clone https://github.com/open-source-parsers/jsoncpp .

WORKDIR /usr/app/third_party/jsoncpp/build
RUN cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF ..
RUN cmake --build . --target install

WORKDIR /usr/app/build

RUN cmake -E env CXXFLAGS="-static-libstdc++" cmake -DCMAKE_BUILD_TYPE=Release ..
RUN make

WORKDIR /usr/app

ENTRYPOINT /usr/app/build/horntail /usr/app/config.json

FROM debian

WORKDIR /usr/app

COPY --from=builder /usr/app/build/horntail /usr/app/horntail
COPY --from=builder /usr/app/resources /usr/app/resources

RUN apt update
RUN apt -y install uuid-dev libbrotli-dev libmariadb-dev libsqlite3-dev libssl-dev zlib1g-dev
RUN apt clean

ENTRYPOINT /usr/app/horntail