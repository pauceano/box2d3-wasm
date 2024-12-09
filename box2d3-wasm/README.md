# Box2d3-wasm

Let's try and use box2d v3 on the web.

```bash
git clone --recurse-submodules https://github.com/Birch-san/box2d3-wasm.git
```

## Building

Install [Emscripten](https://emscripten.org/docs/getting_started/downloads.html).

```bash
# emscripten seems to go bang if you PATH a local typescript, so I had to make it global
npm i -g typescript
FLAVOUR=simd TARGET_TYPE=Debug ./shell/0_build_makefile.sh
emmake make -j8 -C cmake-build
FLAVOUR=simd TARGET_TYPE=Debug ./shell/1_build_wasm.sh
node integration-test/index.mjs
```