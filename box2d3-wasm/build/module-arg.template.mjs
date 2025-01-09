  moduleArg = {
    pthreadCount: globalThis.navigator?.hardwareConcurrency ?? 4,
    sharedMemEnabled: true,
    loadWorkerViaBlob: false,
    ...moduleArg,
  };
  const {pthreadCount, sharedMemEnabled, loadWorkerViaBlob} = moduleArg;