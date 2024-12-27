# GitHub pages demo

We'll write a demo into [`static/index.html`](static/index.html). 

## Local usage

Fetch libraries:

```bash
npm i
```

Copy library assets into static/lib:

```bash
git clean -dfx static/box2d3-wasm static/coi-serviceworker.min.js
mkdir -p static/box2d3-wasm
cp -r node_modules/box2d3-wasm/build/dist/es/. static/box2d3-wasm/
cp node_modules/coi-serviceworker/coi-serviceworker.min.js static/
```

Serve static directory in a similar, limited way to how GitHub pages would (i.e. doesn't set COOP/COEP headers for us):

```bash
npm run serve
```