<?php
// $Id$

/**
 * @file
 * GMap Views support.
 */

function theme_gmap_views_view_gmapextended(&$vars) {
  $markers = array();
  foreach ($vars['markers'] as $offset => $data) {
    if (empty($data['latitude']) || empty($data['longitude'])) {
      continue;
    }
    $markers[] = array(
      'text' => $vars['result'][$offset], // @@@
      'latitude' => (float)$data['latitude'],
      'longitude' => (float)$data['longitude'],
      'markername' => $data['markername'],
    );
  }
  $map = gmap_parse_macro($vars['options']['macro']);
  $map['markers'] = $markers;
  $elem = array(
    '#type' => 'gmap',
    '#gmap_settings' => $map,
  );
  return drupal_render($elem);
}

function template_preprocess_gmap_views_view_gmapextended(&$vars) {
  $view     = $vars['view'];

  // We need the raw data for this grouping, which is passed in as $vars['rows'].
  // However, the template also needs to use for the rendered fields.  We
  // therefore swap the raw data out to a new variable and reset $vars['rows']
  // so that it can get rebuilt.
  // Store rows so that they may be used by further preprocess functions.
  $result   = $vars['result'] = $vars['rows'];
  $vars['rows'] = array();

  $options  = $view->style_plugin->options;
  $handler  = $view->style_plugin;

  $fields   = &$view->field;

  // Render fields.
  $renders = $handler->render_fields($result);

  $markers = array();
  // File away fields into the marker data.
  foreach ($renders as $i => $row) {
    $markers[$i] = $options['fallback_values'];
    foreach ($options['field_purposes'] as $field => $purpose) {
      $markers[$i][$purpose] .= $row[$field];
    }
  }
  /*
  foreach ($options['field_purposes'] as $field => $purpose) {
    foreach ($renders as $i => $row) {
      if (!isset($markers[$i][$purpose])) {
        $markers[$i][$purpose] = '';
      }
      $markers[$i][$purpose] .= $row[$field];
    }
  }
  */

  // @@@ When to enable this?
  // Marker click text.
  foreach ($markers as $offset => $marker) {
    if (!isset($marker['text']) && !empty($vars['result'][$offset])) {
      // Copy prerendered marker text in.
      $marker['text'] = $vars['result'][$offset];
    }
  }

  // @@@ add rest of handling
  /*
  if ($options['markers'] == 'static') {
    foreach ($markers as $offset => $marker) {
      $markers[$offset]['markername'] = $options['markertype'];
    }
  }
  */

  // Marker cleanup
  foreach ($markers as $offset => $marker) {
    $marker['latitude'] = isset($marker['latitude']) ? (float)$marker['latitude'] : NULL;
    $marker['longitude'] = isset($marker['longitude']) ? (float)$marker['longitude'] : NULL;
  }

  $vars['markers'] = $markers;
}
