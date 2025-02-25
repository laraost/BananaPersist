project('persistence',
        'cpp',
        default_options: ['cpp_std=c++20',
                          'warning_level=3',
                          'werror=true',
                          'b_ndebug=if-release'],
        version: '0.0.1')

cpp = meson.get_compiler('cpp')

src_inc_dir = include_directories('src')
ext_inc_dir = include_directories('ext')

persistence_sources = [
     'src/datastructure/banana_tree.cpp',
     'src/datastructure/banana_tree_construction.cpp',
     'src/datastructure/banana_tree_iterators.cpp',
     'src/datastructure/banana_tree_local_operations.cpp',
     'src/datastructure/banana_tree_topological_operations.cpp',
     'src/datastructure/interval.cpp',
     'src/datastructure/list_item.cpp',
     'src/datastructure/persistence_context.cpp',
     'src/datastructure/persistence_diagram.cpp',
     'src/utility/errors.cpp',
     'src/utility/stats.cpp'
]

# Define the search tree used for storing items.
# Has to be one of
#  - 'AVL_SEARCH_TREE'
#  - 'SPLAY_SEARCH_TREE'
search_tree_type = 'SPLAY_SEARCH_TREE'

cpp_definitions = []
if get_option('fallback-operator') == true
  message('Using fallback operator<< for std::chrono::duration')
  cpp_definitions += '-DUSE_FALLBACK_CHRONO_OPERATOR'
endif

boost = dependency('boost')

executable('ex_construction',
           persistence_sources +
               'src/app/experiments/ex_construction.cpp',
           include_directories: [src_inc_dir, ext_inc_dir],
           cpp_args: cpp_definitions + '-DAVL_SEARCH_TREE',
           dependencies: [boost])

executable('ex_local_maintenance',
           persistence_sources +
               'src/app/experiments/ex_local_maintenance.cpp',
           include_directories: [src_inc_dir, ext_inc_dir],
           cpp_args: cpp_definitions + '-DAVL_SEARCH_TREE',
           dependencies: [boost])

executable('ex_topological_maintenance',
           persistence_sources +
               'src/app/experiments/ex_topological_maintenance.cpp',
           include_directories: [src_inc_dir, ext_inc_dir],
           cpp_args: cpp_definitions + '-DSPLAY_SEARCH_TREE',
           dependencies: [boost])

executable('ex_sliding_window_local',
           persistence_sources +
               'src/app/experiments/ex_sliding_window.cpp',
           include_directories: [src_inc_dir, ext_inc_dir],
           cpp_args: cpp_definitions + '-DAVL_SEARCH_TREE' + '-DSLIDING_WINDOW_LOCAL',
           dependencies: [boost])

executable('ex_sliding_window_topological',
           persistence_sources +
               'src/app/experiments/ex_sliding_window.cpp',
           include_directories: [src_inc_dir, ext_inc_dir],
           cpp_args: cpp_definitions + '-DSPLAY_SEARCH_TREE'+ '-DSLIDING_WINDOW_TOPOLOGICAL',
           dependencies: [boost])

executable('ex_time_series',
           persistence_sources +
               'src/app/experiments/ex_time_series.cpp',
           include_directories: [src_inc_dir, ext_inc_dir],
           cpp_args: cpp_definitions + '-DSPLAY_SEARCH_TREE',
           dependencies: [boost])

executable('generate_data',
           persistence_sources +
               'src/app/experiments/generate_data.cpp',
           include_directories: [src_inc_dir, ext_inc_dir],
           cpp_args: cpp_definitions + '-DAVL_SEARCH_TREE',
           dependencies: [boost])

#==============#
# Unit Testing #
#==============#

gtest = dependency('gtest', required: false)
gtest_main = dependency('gtest_main', required: false)

if gtest.found() and gtest_main.found()
  test_inc_dir = include_directories('test')
  test_definitions = ['-D' + search_tree_type]

  list_item_test_exe = executable('list_item_test',
                                  ['test/list_item_test.cpp'] +
                                       persistence_sources,
                                  include_directories: [src_inc_dir, test_inc_dir],
                                  dependencies: [boost, gtest, gtest_main],
                                  cpp_args: cpp_definitions + test_definitions)
  test('list_item', list_item_test_exe, protocol: 'gtest')

  interval_test_exe = executable('interval_test',
                                  ['test/interval_test.cpp'] +
                                       persistence_sources,
                                  include_directories: [src_inc_dir, test_inc_dir],
                                  dependencies: [boost, gtest, gtest_main],
                                  cpp_args: cpp_definitions + test_definitions)
  test('interval', interval_test_exe, protocol: 'gtest')

  banana_tree_construction_test_exe = executable('banana_tree_construction_test',
                                  ['test/banana_tree_construction_test.cpp'] +
                                       persistence_sources,
                                  include_directories: [src_inc_dir, test_inc_dir],
                                  dependencies: [boost, gtest, gtest_main],
                                  cpp_args: cpp_definitions + test_definitions)
  test('banana_tree_construction', banana_tree_construction_test_exe, protocol: 'gtest')

  banana_tree_iteration_test_exe = executable('banana_tree_iteration_test',
                                  ['test/banana_tree_iteration_test.cpp'] +
                                       persistence_sources,
                                  include_directories: [src_inc_dir, test_inc_dir],
                                  dependencies: [boost, gtest, gtest_main],
                                  cpp_args: cpp_definitions + test_definitions)
  test('banana_tree_iteration', banana_tree_iteration_test_exe, protocol: 'gtest')

  persistence_diagram_test_exe = executable('persistence_diagram_test',
                                  ['test/persistence_diagram_test.cpp'] +
                                       persistence_sources,
                                  include_directories: [src_inc_dir, test_inc_dir],
                                  dependencies: [boost, gtest, gtest_main],
                                  cpp_args: cpp_definitions + test_definitions)
  test('persistence_diagram', persistence_diagram_test_exe, protocol: 'gtest')

  local_operation_test_exe = executable('local_operation_test',
                                  ['test/local_operation_test.cpp'] +
                                       persistence_sources,
                                  include_directories: [src_inc_dir, test_inc_dir],
                                  dependencies: [boost, gtest, gtest_main],
                                  cpp_args: cpp_definitions + test_definitions)
  test('local_operation', local_operation_test_exe, protocol: 'gtest')

  topological_operation_test_exe = executable('topological_operation_test',
                                  ['test/topological_operation_test.cpp'] +
                                       persistence_sources,
                                  include_directories: [src_inc_dir, test_inc_dir],
                                  dependencies: [boost, gtest, gtest_main],
                                  cpp_args: cpp_definitions + test_definitions)
  test('topological_operation', topological_operation_test_exe, protocol: 'gtest')

  search_tree_test_exe = executable('search_tree_test',
                                  ['test/search_tree_test.cpp'] +
                                       persistence_sources,
                                  include_directories: [src_inc_dir, test_inc_dir],
                                  dependencies: [boost, gtest, gtest_main],
                                  cpp_args: cpp_definitions + test_definitions)
  test('search_tree', search_tree_test_exe, protocol: 'gtest')

  random_instance_test_exe = executable('random_instance_test',
                                  ['test/random_instance_test.cpp'] +
                                       persistence_sources,
                                  include_directories: [src_inc_dir, test_inc_dir],
                                  dependencies: [boost, gtest, gtest_main],
                                  cpp_args: cpp_definitions + test_definitions)
  test('random_instance', random_instance_test_exe, protocol: 'gtest')

  analysis_test_exe = executable('analysis_test',
                                  ['test/analysis_test.cpp'] +
                                       persistence_sources,
                                  include_directories: [src_inc_dir, test_inc_dir],
                                  dependencies: [boost, gtest, gtest_main],
                                  cpp_args: cpp_definitions + test_definitions)
  test('analysis', analysis_test_exe, protocol: 'gtest')
else
  message('gtest or gtest_main have not been found. Not building tests.')
endif
