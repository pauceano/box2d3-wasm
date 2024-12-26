# box2d3-wasm

Let's try and use [Box2D v3][] on the web via [wasm][]!

```bash
git clone --recurse-submodules https://github.com/Birch-san/box2d3-wasm.git
```

## Install from npm

Install [Node.js and npm][].  
Install [npm package](https://www.npmjs.com/package/box2d3-wasm) like so:

```bash
npm i --save box2d3-wasm
```

## Usage

**Web:**  
See the [demo](demo/modern/index.html) for an example of how to use box2d3-wasm via the Web platform. You'll need to serve the assets correctly in order to access performance features such as threading. See [Run Demos](#run-demos) for more on the serving requirements.

**NodeJS:**  
See the [integration test](integration-test/index.mjs) for an example of how to use box2d3-wasm via NodeJS. There'll be no graphics; you probably only want the NodeJS approach if you're building server-side physics or you intend to build your own native GUI.


## Build-From-Source Pre-Requisites

Install [Node.js and npm][].

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

These policies are required for the Box2D3 WebAssembly module to run with threading, because it uses the SharedArrayBuffer feature which requires an isolated context for security reasons.

Regardless of whether you use threading: you may need web server customizations (or good defaults) to serve WASM assets with the correct MIME type (`application/wasm`).

## Run Tests

```bash
node integration-test/index.mjs
```

## Release

From `box2d3-wasm` package's directory,

```bash
git clean -dfx build cmake-build
FLAVOUR=simd TARGET_TYPE=Release ./shell/0_build_makefile.sh
emmake make -j8 -C cmake-build
FLAVOUR=simd TARGET_TYPE=Release ./shell/1_build_wasm.sh
# e.g. major | minor | patch, see https://docs.npmjs.com/cli/v10/commands/npm-version
npm version minor
npm publish
```

[Box2D v3]: https://github.com/erincatto/box2d
[wasm]: https://webassembly.org/
[Node.js and npm]: https://nodejs.org/en/download/