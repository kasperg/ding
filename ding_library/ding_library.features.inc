<?php

/**
 * Implementation of hook_content_default_fields().
 */
function ding_library_content_default_fields() {
  module_load_include('inc', 'ding_library', 'ding_library.defaults');
  $args = func_get_args();
  return call_user_func_array('_ding_library_content_default_fields', $args);
}

/**
 * Implementation of hook_imagecache_default_presets().
 */
function ding_library_imagecache_default_presets() {
  module_load_include('inc', 'ding_library', 'ding_library.defaults');
  $args = func_get_args();
  return call_user_func_array('_ding_library_imagecache_default_presets', $args);
}

/**
 * Implementation of hook_strongarm().
 */
function ding_library_strongarm() {
  module_load_include('inc', 'ding_library', 'ding_library.defaults');
  $args = func_get_args();
  return call_user_func_array('_ding_library_strongarm', $args);
}
