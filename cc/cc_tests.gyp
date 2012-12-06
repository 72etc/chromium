# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'chromium_code': 0,
    'cc_unit_tests_source_files': [
      'active_animation_unittest.cc',
      'content_layer_unittest.cc',
      'contents_scaling_layer_unittest.cc',
      'damage_tracker_unittest.cc',
      'delay_based_time_source_unittest.cc',
      'delegated_renderer_layer_impl_unittest.cc',
      'draw_quad_unittest.cc',
      'float_quad_unittest.cc',
      'frame_rate_controller_unittest.cc',
      'gl_renderer_unittest.cc',
      'gl_renderer_pixeltest.cc',
      'hash_pair_unittest.cc',
      'heads_up_display_unittest.cc',
      'keyframed_animation_curve_unittest.cc',
      'layer_animation_controller_unittest.cc',
      'layer_impl_unittest.cc',
      'layer_iterator_unittest.cc',
      'layer_quad_unittest.cc',
      'layer_sorter_unittest.cc',
      'layer_tree_host_common_unittest.cc',
      'layer_tree_host_impl_unittest.cc',
      'layer_tree_host_unittest.cc',
      'layer_unittest.cc',
      'math_util_unittest.cc',
      'nine_patch_layer_impl_unittest.cc',
      'nine_patch_layer_unittest.cc',
      'occlusion_tracker_unittest.cc',
      'picture_layer_tiling_set_unittest.cc',
      'picture_layer_tiling_unittest.cc',
      'prioritized_resource_unittest.cc',
      'quad_culler_unittest.cc',
      'region_unittest.cc',
      'render_pass_unittest.cc',
      'render_surface_filters_unittest.cc',
      'render_surface_unittest.cc',
      'resource_provider_unittest.cc',
      'resource_update_controller_unittest.cc',
      'scheduler_state_machine_unittest.cc',
      'scheduler_unittest.cc',
      'scoped_resource_unittest.cc',
      'scrollbar_animation_controller_linear_fade_unittest.cc',
      'scrollbar_layer_unittest.cc',
      'software_renderer_unittest.cc',
      'solid_color_layer_impl_unittest.cc',
      'texture_copier_unittest.cc',
      'texture_layer_unittest.cc',
      'texture_uploader_unittest.cc',
      'tile_priority_unittest.cc',
      'tiled_layer_impl_unittest.cc',
      'tiled_layer_unittest.cc',
      'tree_synchronizer_unittest.cc',
      'timing_function_unittest.cc',
      'test/fake_web_graphics_context_3d_unittest.cc',
    ],
    'cc_tests_support_files': [
      'test/animation_test_common.cc',
      'test/animation_test_common.h',
      'test/compositor_fake_web_graphics_context_3d.h',
      'test/fake_content_layer_client.cc',
      'test/fake_content_layer_client.h',
      'test/fake_output_surface.h',
      'test/fake_layer_tree_host_client.cc',
      'test/fake_layer_tree_host_client.h',
      'test/fake_picture_layer_tiling_client.cc',
      'test/fake_picture_layer_tiling_client.h',
      'test/fake_proxy.cc',
      'test/fake_proxy.h',
      'test/fake_tile_manager_client.h',
      'test/fake_web_compositor_output_surface.h',
      'test/fake_web_compositor_software_output_device.h',
      'test/fake_web_graphics_context_3d.h',
      'test/fake_web_scrollbar_theme_geometry.h',
      'test/geometry_test_utils.cc',
      'test/geometry_test_utils.h',
      'test/layer_test_common.cc',
      'test/layer_test_common.h',
      'test/layer_tree_test_common.cc',
      'test/layer_tree_test_common.h',
      'test/mock_quad_culler.cc',
      'test/mock_quad_culler.h',
      'test/occlusion_tracker_test_common.h',
      'test/paths.cc',
      'test/paths.h',
      'test/pixel_test_output_surface.cc',
      'test/pixel_test_output_surface.h',
      'test/render_pass_test_common.cc',
      'test/render_pass_test_common.h',
      'test/scheduler_test_common.cc',
      'test/scheduler_test_common.h',
      'test/tiled_layer_test_common.cc',
      'test/tiled_layer_test_common.h',
    ],
  },
  'targets': [
    {
      'target_name': 'cc_unittests',
      'type': '<(gtest_target_type)',
      'dependencies': [
        '../base/base.gyp:test_support_base',
        '../media/media.gyp:media',
        '../skia/skia.gyp:skia',
        '../testing/gmock.gyp:gmock',
        '../testing/gtest.gyp:gtest',
        '../ui/ui.gyp:ui',
        '../webkit/support/webkit_support.gyp:webkit_gpu',
        'cc.gyp:cc',
        'cc_test_support',
        'cc_test_utils',
      ],
      'sources': [
        'test/run_all_unittests.cc',
        '<@(cc_unit_tests_source_files)',
      ],
      'include_dirs': [
        'test',
        '.',
        '../third_party/WebKit/Source/Platform/chromium',
      ],
      'conditions': [
        ['OS == "android" and gtest_target_type == "shared_library"', {
          'dependencies': [
            '../testing/android/native_test.gyp:native_test_native_code',
          ],
        }],
      ],
    },
    {
      'target_name': 'cc_perftests',
      'type': '<(gtest_target_type)',
      'dependencies': [
        '../base/base.gyp:test_support_base',
        '../media/media.gyp:media',
        '../skia/skia.gyp:skia',
        '../testing/gmock.gyp:gmock',
        '../testing/gtest.gyp:gtest',
        '../ui/ui.gyp:ui',
        'cc.gyp:cc',
        'cc_test_support',
      ],
      'sources': [
        'layer_tree_host_perftest.cc',
        'test/run_all_unittests.cc',
      ],
      'include_dirs': [
        'test',
        '.',
        '../third_party/WebKit/Source/Platform/chromium',
      ],
      'conditions': [
        ['OS == "android" and gtest_target_type == "shared_library"', {
          'dependencies': [
            '../testing/android/native_test.gyp:native_test_native_code',
          ],
        }],
      ],
    },
    {
      'target_name': 'cc_test_support',
      'type': 'static_library',
      'include_dirs': [
        'test',
        '.',
        '..',
        '../third_party/WebKit/Source/Platform/chromium',
      ],
      'dependencies': [
        '../skia/skia.gyp:skia',
        '../testing/gmock.gyp:gmock',
        '../testing/gtest.gyp:gtest',
        '../third_party/WebKit/Source/WebKit/chromium/WebKit.gyp:webkit',
        '../third_party/mesa/mesa.gyp:osmesa',
        '../ui/gl/gl.gyp:gl',
      ],
      'sources': [
        '<@(cc_tests_support_files)',
      ],
    },
    {
      'target_name': 'cc_test_utils',
      'type': 'static_library',
      'include_dirs': [
        '..'
      ],
      'sources': [
        'test/pixel_test_utils.cc',
        'test/pixel_test_utils.h',
      ],
      'dependencies': [
        '../skia/skia.gyp:skia',
        '../ui/ui.gyp:ui',  # for png_codec
      ],
    },
  ],
  'conditions': [
    # Special target to wrap a gtest_target_type==shared_library
    # cc_unittests into an android apk for execution.
    ['OS == "android" and gtest_target_type == "shared_library"', {
      'targets': [
        {
          'target_name': 'cc_unittests_apk',
          'type': 'none',
          'dependencies': [
            'cc_unittests',
          ],
          'variables': {
            'test_suite_name': 'cc_unittests',
            'input_shlib_path': '<(SHARED_LIB_DIR)/<(SHARED_LIB_PREFIX)cc_unittests<(SHARED_LIB_SUFFIX)',
          },
          'includes': [ '../build/apk_test.gypi' ],
        },
      ],
    }],
    ['OS == "android" and gtest_target_type == "shared_library"', {
      'targets': [
        {
          'target_name': 'cc_perftests_apk',
          'type': 'none',
          'dependencies': [
            'cc_perftests',
          ],
          'variables': {
            'test_suite_name': 'cc_perftests',
            'input_shlib_path': '<(SHARED_LIB_DIR)/<(SHARED_LIB_PREFIX)cc_perftests<(SHARED_LIB_SUFFIX)',
          },
          'includes': [ '../build/apk_test.gypi' ],
        },
      ],
    }]
  ],
}
