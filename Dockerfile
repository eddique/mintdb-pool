FROM alpine:latest AS build

WORKDIR /app

RUN apk update && apk add --no-cache make gcc musl-dev

COPY src ./src

COPY include ./include

COPY makefile .

RUN mkdir -p bin obj

RUN make

RUN chmod +x /app/bin/mint

FROM scratch

COPY --from=build /app/bin/mint /mint

EXPOSE 8080

ENTRYPOINT [ "./mint" ]