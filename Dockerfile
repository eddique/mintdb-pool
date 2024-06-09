FROM alpine:latest AS build

WORKDIR /app

RUN apk update && apk add --no-cache make gcc musl-dev

COPY src ./src

COPY include ./include

COPY makefile .

RUN mkdir -p bin obj

RUN make

RUN chmod +x /app/bin/mintdb-pool

FROM scratch

COPY --from=build /app/bin/mintdb-pool /mintdb-pool

EXPOSE 4321

ENTRYPOINT [ "./mintdb-pool" ]