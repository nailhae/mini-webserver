# mini-webserver

mini-webserver는 HTTP1.1을 지원하는 웹서버입니다.
웹서버와 멀티플렉싱, HTTP프로토콜을 이해하기 위한 목적으로 진행한 프로젝트입니다.

## 실행 요구사항

- macOS or BSD
- C++98
- make

## 특징

- HTTP1.1 지원
- I/O Multiplexing(`kqueue()` 활용)
- `config` 파일을 통한 서버 설정 지원
- `CGI` 지원

## 역할

- jeongyunnim: mini-webserver 설계, WebServer class 구현, HTTP 메소드 함수 구현

- nailhae: `conf` 파일 설계, HTTP 메소드 함수 구현

- parksangmin1543: `conf` 파일 파싱, CGI 구현

- hyemch: HTTP 메소드 함수 구현

## 실행

```sh
git clone git@github.com:nailhae/mini-webserver.git
```

```sh
cd ./mini-webserver.git
```

```sh
make
```

```sh
./mini-webserver config/default.conf
```

## 서버 설정

- 설정 파일 위치

  - `mini-webserver/config/default.conf`

- 설정 파일 예시

```sh
# MIME 기본 지원 text/plain, png, html, css
default_type text/plain;

# Setup default error pages.
error_page 400 ./assets/html/error/400.html;
error_page 404 ./assets/html/error/404.html;
error_page 405 ./assets/html/error/405.html;
error_page 500 502 503 504  ./assets/html/error/50x.html;

server {
  listen       8888;
  # Setup the server_names or not.
  server_name  hello_world;

  client_max_body_size 1000000;
  root         ./assets/html;

  location / {
    # Define a list of accepted HTTP methods for the route.
    limit_except GET POST;

    location /cgi-bin {
      root         ./assets;
      location /tmp {
        limit_except GET POST DELETE;
      }
    }
    # Turn on or off directory listing.
    autoindex on;
    # Set a default file to answer if the request is a directory.
    index index.html;
    location /hello/ {
    }
  }
}

# server block 추가 가능
```
