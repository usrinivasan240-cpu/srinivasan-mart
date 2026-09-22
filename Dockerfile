# ============================================================
# Sri Mart backend — C++20 + Drogon + libpq (PostgreSQL).
# Used by Render/Fly/etc. Vercel cannot run this server, so the
# static UI (vercel.json) goes to Vercel and this image runs the API.
# Drogan is built via vcpkg; PORT and PG* come from the environment.
# ============================================================

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
# CPATH lets the compiler find libpq-fe.h from libpq-dev
# (/usr/include/postgresql) without touching CMakeLists.txt.
ENV CPATH=/usr/include/postgresql

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake git curl zip unzip tar pkg-config \
    libssl-dev libpq-dev zlib1g-dev libjsoncpp-dev uuid-dev \
    && rm -rf /var/lib/apt/lists

# vcpkg provides Drogon + its dependency tree.
RUN git clone --depth 1 https://github.com/microsoft/vcpkg.git /opt/vcpkg \
    && /opt/vcpkg/bootstrap-vcpkg.sh
RUN /opt/vcpkg/vcpkg install drogon

WORKDIR /app
COPY . .

RUN cmake -B build -S . \
    -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
    && cmake --build build -j"$(nproc)"

ENV PORT=8080
EXPOSE 8080

CMD ["./build/sri_mart"]
