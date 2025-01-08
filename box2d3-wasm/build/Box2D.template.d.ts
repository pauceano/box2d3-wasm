// Emscripten Module properties that looked interesting enough to write documentation for
export interface EmscriptenModuleKnownOptions {
  // initial heap size. the default value is 16MiB, which you can express as 16 * 1024**2
  // must be a multiple of 65536 (it's 16KiB pages, 16 * 1024 bytes each)
  INITIAL_MEMORY: number;
}

// Module properties we ourselves patched into the Emscripten output
export interface OurModuleOptions {
  // set to 0 to disable threading. default value is to use all your threads,
  // but this is probably a bad default, as it may incur memory syncs across
  // caches. it's better to keep the work within CPU cores which share a cache.
  // 4 is a safe value and probably good enough. Some Apple and Intel chips may
  // share caches between more than 8 cores.
  pthreadCount: number;
}

// Combine known options with arbitrary properties
export type ModuleOptions = Partial<EmscriptenModuleKnownOptions & OurModuleOptions> & Record<string, any>;

export default function MainModuleFactory(
  options?: ModuleOptions
): Promise<MainModule>;