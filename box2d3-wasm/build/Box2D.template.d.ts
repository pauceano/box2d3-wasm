// Emscripten Module properties that looked interesting enough to write documentation for
export interface EmscriptenModuleKnownOptions {
  // default: 16MiB, which you can express as 16 * 1024**2
  // initial heap size.
  // must be a multiple of 65536 (it's 16KiB pages, 16 * 1024 bytes each)
  INITIAL_MEMORY: number;
}

// Module properties we ourselves patched into the Emscripten output
export interface OurModuleOptions {
  // default: globalThis.navigator?.hardwareConcurrency ?? 4
  // set to 0 to disable threading. default value is to use all your threads,
  // but this is probably a bad default, as it may incur memory syncs across
  // caches. it's better to keep the work within CPU cores which share a cache.
  // 4 is a safe value and probably good enough. Some Apple and Intel chips may
  // share caches between more than 8 cores.
  pthreadCount: number;
  // default: true
  // required for threading, but if you're not using threading you could turn this off.
  // perhaps turning this off will reduce the need to synchronize reads & writes.
  // some browsers (e.g. Safari <16.4) cannot grow shared memory, so turning this off
  // could improve compatibility in that situation.
  sharedMemEnabled: boolean;
}

// Combine known options with arbitrary properties
export type ModuleOptions = Partial<EmscriptenModuleKnownOptions & OurModuleOptions> & Record<string, any>;

export default function MainModuleFactory(
  options?: ModuleOptions
): Promise<MainModule>;