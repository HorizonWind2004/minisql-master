file(REMOVE_RECURSE
  ".1"
  "libglog.0.6.0.dylib"
  "libglog.1.dylib"
  "libglog.dylib"
  "libglog.pdb"
)

# Per-language clean rules from dependency scanning.
foreach(lang CXX)
  include(CMakeFiles/glog.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
