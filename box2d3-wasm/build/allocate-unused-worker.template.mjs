  allocateUnusedWorkerViaBlob: async function() {
    if (!globalThis._workerPromise) {
      globalThis._workerPromise = (async function() {
        const response = await fetch(new URL("Box2D.deluxe.mjs", import.meta.url));
        const text = await response.text();
        const blob = new Blob([text], { type: 'application/javascript' });
        const workerUrl = URL.createObjectURL(blob);

        setTimeout(() => {
          URL.revokeObjectURL(workerUrl);
          delete globalThis._workerPromise;
        }, 0);

        return workerUrl;
      })();
    }

    const workerUrl = await globalThis._workerPromise;

    var workerOptions = {
      "type": "module",
      "workerData": "em-pthread",
      "name": "em-pthread-" + PThread.nextWorkerID
    };

    const worker = new Worker(workerUrl, workerOptions);
    PThread.unusedWorkers.push(worker);
    return worker;
  },
  allocateUnusedWorker: async function() {
    if (loadWorkerViaBlob) {
      await PThread.allocateUnusedWorkerViaBlob();
    } else {
      PThread.allocateUnusedWorkerDirect();
    }
  },