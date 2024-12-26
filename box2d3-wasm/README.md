# box2d3-wasm

Let's try and use [Box2D v3][] on the web via [wasm][]!

```bash
git clone --recurse-submodules https://github.com/Birch-san/box2d3-wasm.git
```

## Pre-Requisites

Install [Node.js and npm](https://nodejs.org/en/download/).

emscripten seems to go bang if you PATH a local typescript, so I had to make it global.

Install typescript and yarn.

```bash
npm i -g typescript
npm i -g yarn
```

Next, install [Emscripten](https://emscripten.org/docs/getting_started/downloads.html).

On macOS, you can use [Homebrew](https://brew.sh/) to install it.

```bash
brew install emscripten
```

## Build

Run the following commands:

```bash
FLAVOUR=simd TARGET_TYPE=Debug ./shell/0_build_makefile.sh
emmake make -j8 -C cmake-build
FLAVOUR=simd TARGET_TYPE=Debug ./shell/1_build_wasm.sh
```

## Run Demos

Inside the `demo` directory, run `yarn` to install `local-web-server` locally.

Then, to start the web server run `yarn serve`.

Note that the server runs with the following CORS headers:

```
Cross-Origin-Embedder-Policy: require-corp
Cross-Origin-Opener-Policy: same-origin
```

These are required for the Box2D3 WebAssembly module to run, because it uses the SharedArrayBuffer feature which requires an isolated context for security reasons.

## Run Tests

```bash
node integration-test/index.mjs
```

[Box2D v3]: https://github.com/erincatto/box2d
[wasm]: https://webassembly.org/
