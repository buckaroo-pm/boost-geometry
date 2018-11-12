prebuilt_cxx_library(
  name = 'geometry', 
  header_namespace = 'boost', 
  header_only = True, 
  exported_headers = subdir_glob([
    ('include/boost', '**/*.hpp'), 
  ]), 
  deps = [
    'buckaroo.github.buckaroo-pm.boost-mpl//:mpl', 
  ], 
  visibility = [
    'PUBLIC', 
  ], 
)
