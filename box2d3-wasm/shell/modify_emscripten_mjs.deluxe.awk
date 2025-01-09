#!/usr/bin/env awk -f

BEGIN { found1=0; found2=0; found3=0; found4=0; found5=0 }
!found1 && $0 ~ /^async function\(moduleArg = \{\}\) \{$/ {
  print $0
  while ((getline line < module_arg_template) > 0) {
    print line
  }
  close(module_arg_template)
  found1=1
  next
}
!found2 && /^[[:space:]]*var pthreadPoolSize = _emscripten_num_logical_cores\(\);$/ {
  sub(/_emscripten_num_logical_cores\(\)/, "pthreadCount")
  print
  found2=1
  next
}
!found3 && /^[[:space:]]*"shared": true/ {
  sub(/"shared": true/, "\"shared\": sharedMemEnabled")
  print
  found3=1
  next
}
!found4 && /^[[:space:]]*PThread\.init\(\);$/ {
  print "  if(pthreadCount > 0) { PThread.init(); }"
  found4=1
  next
}
!found5 && /^[[:space:]]*allocateUnusedWorker\(\) \{$/ {
  while ((getline line < allocate_unused_worker_template) > 0) {
    print line
  }
  close(allocate_unused_worker_template)
  sub(/allocateUnusedWorker/, "allocateUnusedWorkerDirect")
  print
  found5=1
  next
}
{ print }
END { exit !(found1 && found2 && found3 && found4) }