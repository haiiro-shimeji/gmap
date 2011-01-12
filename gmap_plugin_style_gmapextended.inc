<?php
// $Id$

/**
 * @file
 * GMap style plugin.
 */

/**
 * Style plugin to render a map.
 *
 * @ingroup views_style_plugins
 */
class gmap_plugin_style_gmapextended extends views_plugin_style {
  /**
   * Set default options
   */
  function option_definition() {
    $options = parent::option_definition();
    
    $options['fallback_values'] = array('default' => array());
    $options['field_purposes'] = array('default' => array());

    $options['macro'] = array(
      'default' => '[gmap ]',
    );

    $options['datasource'] = array(
      'default' => 'location',
    );

    $options['markers'] = array('default' => 'static');
    $options['markertype'] = array('default' => 'drupal');

    $options['latfield'] = array('default' => '');
    $options['lonfield'] = array('default' => '');
    $options['markerfield'] = array('default' => '');

    $options['center_on_nodearg'] = array('default' => 0);
    $options['center_on_nodearg_arg'] = array('default' => '');

    $options['highlight_nodearg'] = array('default' => 0);
    $options['highlight_nodearg_arg'] = array('default' => '');
    $options['highlight_nodearg_color'] = array('default' => '#FF0000');

    $options['tooltipenabled'] = array('default' => 0);
    $options['tooltipfield'] = array('default' => '');

    return $options;
  }

  function purpose_list() {
    return array(
      '' => t('Unused'),
      'text' => t('Marker bubble contents'),
      'lat' => t('Marker latitude'),
      'lon' => t('Marker longitude'),
      'dyn' => t('Address for dynamic geocoding'),
      'title' => t('Marker title (tooltip)'),
      'marker' => t('Marker type'),
      'offset' => t('Marker offset'),
    );
  }

  function query() {
    parent::query();

    if ($this->options['datasource'] == 'location') {
      $table = $this->view->query->ensure_table('location');
      $this->view->query->add_field($table, 'latitude', 'gmap_lat');
      $this->view->query->add_field($table, 'longitude', 'gmap_lon');
    }

    if ($this->options['markers'] == 'nodetype') {
      $this->view->query->add_field('node', 'type', 'gmap_node_type');
    }
    else if ($this->options['markers'] == 'taxonomy') {
      $this->view->query->add_field('gmap_taxonomy_node', 'marker', 'gmap_node_marker');
    }
    else if ($this->options['markers'] == 'userrole') {
      $this->view->query->add_field('users_roles', 'rid', 'gmap_role_marker');
    }

    if (isset($this->row_plugin)) {
      $this->row_plugin->query();
    }
  }

  /**
   * Normalize a list of columns based upon the fields that are
   * available. This compares the fields stored in the style handler
   * to the list of fields actually in the view, removing fields that
   * have been removed and adding new fields in their own column.
   *
   * - Each field must be in a column.
   * - Each column must be based upon a field, and that field
   *   is somewhere in the column.
   * - Any fields not currently represented must be added.
   * - Columns must be re-ordered to match the fields.
   *
   * @param $columns
   *   An array of all fields; the key is the id of the field and the
   *   value is the id of the column the field should be in.
   * @param $fields
   *   The fields to use for the columns. If not provided, they will
   *   be requested from the current display. The running render should
   *   send the fields through, as they may be different than what the
   *   display has listed due to access control or other changes.
   */
  function sanitize_columns($columns, $fields = NULL) {
    $sanitized = array();
    if ($fields === NULL) {
      $fields = $this->display->handler->get_option('fields');
    }

    // Preconfigure the sanitized array so that the order is retained.
    foreach ($fields as $field => $info) {
      // Set to itself so that if it isn't touched, it gets column
      // status automatically.
      $sanitized[$field] = $field;
    }

    foreach ($columns as $field => $column) {
      // first, make sure the field still exists.
      if (!isset($sanitized[$field])) {
        continue;
      }

      // If the field is the column, mark it so, or the column
      // it's set to is a column, that's ok
      if ($field == $column || (isset($columns[$column]) && $columns[$column] == $column) && !empty($sanitized[$column])) {
        $sanitized[$field] = $column;
      }
      // Since we set the field to itself initially, ignoring
      // the condition is ok; the field will get its column
      // status back.
    }

    return $sanitized;
  }

  /**
   * Render the given style.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $handlers = $this->display->handler->get_handlers('field');
    if (empty($handlers)) {
      $form['error_markup'] = array(
        '#markup' => '<div class="error form-item description">' . t('You need at least one field before you can configure your GMap settings') . '</div>',
      );
      return;
    }

    $form['#theme'] = 'gmap_views_ui_gmapextended';

    $purposes = $this->purpose_list();
    $columns = $this->sanitize_columns($this->options['field_purposes']);

    // Create an array of allowed columns from the data we know:
    $field_names = $this->display->handler->get_field_labels();

    foreach ($columns as $field => $column) {
      $safe = str_replace(array('][', '_', ' '), '-', $field);
      // the $id of the column for dependency checking.
      $id = 'edit-style-options-columns-' . $safe;

      $form['field_purposes'][$field] = array(
        '#type' => 'select',
        '#options' => $purposes,//$field_names,
        '#default_value' => $column,
      );
      $form['info'][$field]['separator'] = array(
        '#type' => 'textfield',
        '#size' => 10,
        '#default_value' => isset($this->options['info'][$field]['separator']) ? $this->options['info'][$field]['separator'] : '',
        '#dependency' => array($id => array($field)),
      );

      // markup for the field name
      $form['info'][$field]['name'] = array(
        '#markup' => $field_names[$field],
      );
    }

    $form['description_markup'] = array(
      '#markup' => '<div class="description form-item">' . t('Place fields into columns; you may combine multiple fields into the same column. If you do, the separator in the column specified will be used to separate the fields. Check the sortable box to make that column click sortable, and check the default sort radio to determine which column will be sorted by default, if any. You may control column order and field labels in the fields section.') . '</div>',
    );

    $form['fallback_values'] = array(
      '#type' => 'fieldset',
      '#tree' => TRUE,
      '#title' => 'Default values',
      '#description' => t('You can set the value to use for marker parameters not controlled by fields here.'),
    );
    // @@@ a bit hacky...
    /*
    foreach ($purposes as $key => $title) {
      $form['fallback_values'][$key] = array(
        '#title' => $title,
        '#type' => 'textarea', //@@@
        '#default_value' => isset($this->options['fallback_values'][$key]) ? $this->options['fallback_values'][$key] : '',
      );
    }
    */
    $form['fallback_values']['text'] = array( // @@@ Make this #text_format compatible so people can do neat tricks.
      '#type' => 'textarea',
      '#title' => t('Marker bubble contents'),
      '#default_value' => isset($this->options['fallback_values']['text']) ? $this->options['fallback_values']['text'] : '',
    );
    $form['fallback_values']['title'] = array(
      '#type' => 'textfield',
      '#title' => t('Marker title (tooltip)'),
      '#description' => t('Set a fallback value for the marker tooltip if you want to use a fixed tooltip instead of assigning a field, or leave blank.'),
      '#default_value' => isset($this->options['fallback_values']['title']) ? $this->options['fallback_values']['title'] : '',
    );
    $markernames = array('' => t('Use site default'));
    $markernames += gmap_get_marker_titles();
    $form['fallback_values']['marker'] = array(
      '#type' => 'select',
      '#title' => t('Marker type'),
      '#options' => $markernames,
      '#description' => t('Set a fallback value for the marker type if you want to use a fixed type.'),
      '#default_value' => isset($this->options['fallback_values']['marker']) ? $this->options['fallback_values']['marker'] : '',
    );
    $form['fallback_values']['offset'] = array(
      '#type' => 'textfield',
      '#title' => t('Marker offset'),
      '#description' => t('Set a fallback value if you want to use a static offset. Leave blank if you want automatic assignment.'),
      '#default_value' => isset($this->options['fallback_values']['offset']) ? $this->options['fallback_values']['offset'] : '',
    );


//////////////////////////

    $form['macro'] = array(
      '#type' => 'textarea',
      '#title' => t('Macro'),
      '#rows' => 3,
      '#default_value' => $this->options['macro'],
    );

    $form['markers'] = array(
      '#type' => 'select',
      '#title' => t('Marker handling'),
      // @@@ Detect view type automatically?
      '#options' => array(
        'nodetype' => t('By content type (for node views)'),
        'taxonomy' => t('By term (for node views)'),
        'userrole' => t('By user role (for user views)'),
        'field' => t('Use marker field'),
        'static' => t('Use single marker type'),
      ),
      '#default_value' => $this->options['markers'],
    );

    // Hide the taxonomy handling if gmap_taxonomy.module isn't installed.
    if (!module_exists('gmap_taxonomy')) {
      unset($form['markers']['#options']['taxonomy']);
    }

    $form['markertype'] = array(
      '#type' => 'gmap_markerchooser',
      '#title' => t('Marker / fallback marker to use'),
      '#default_value' => $this->options['markertype'],
    );

    $form['center_on_nodearg'] = array(
      '#type' => 'checkbox',
      '#title' => t('Center on node argument'),
      '#default_value' => $this->options['center_on_nodearg'],
      '#description' => ($this->view->base_table == 'node') ? t('Note: The view must contain an argument whose value is a node ID.') : t('Note: The view must contain an argument whose value is a node ID.') . '<br />' . t("The view must contain 'Node: nid' as one of its fields because the view type is not 'Node'."),
    );
    $form['center_on_nodearg_arg'] = array(
      '#title' => t('Argument'),
      '#description' => empty($argument_options) ? t("The value of the selected argument must be a number that matches a node ID.  Use the 'Global: Null' argument if you don't want to also restrict results to that node ID.  You must have added arguments to the view to use this option.") : t("The selected argument must be a number that matches a node ID.  Use the 'Global: Null' argument if you don't want to also restrict results to that node ID."),
      '#type' => 'select',
      '#options' => $argument_options,
      '#default_value' => $this->options['center_on_nodearg_arg'],
//      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-style-options-center-on-nodearg' => array(TRUE)),
    );

    $form['highlight_nodearg'] = array(
      '#type' => 'checkbox',
      '#title' => t('Highlight marker for node argument'),
      '#default_value' => $this->options['highlight_nodearg'],
      '#description' => ($this->view->base_table == 'node') ? t('Note: The view must contain an argument whose value is a node ID.') : t('Note: The view must contain an argument whose value is a node ID.') . '<br />' . t("The view must contain 'Node: nid' as one of its fields because the view type is not 'Node'."),
    );
    $form['highlight_nodearg_arg'] = array(
      '#title' => t('Argument'),
      '#description' => empty($argument_options) ? t("The value of the selected argument must be a number that matches a node ID.  Use the 'Global: Null' argument if you don't want to also restrict results to that node ID.  You must have added arguments to the view to use this option.") : t("The value of the selected argument must be a number that matches a node ID.  Use the 'Global: Null' argument if you don't want to also restrict results to that node ID."),
      '#type' => 'select',
      '#options' => $argument_options,
      '#default_value' => $this->options['highlight_nodearg_arg'],
//      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-style-options-highlight-nodearg' => array(TRUE)),
    );
    $form['highlight_nodearg_color'] = array(
      '#title' => t('Highlight color'),
      '#description' => t("A 6 digit hex color value to use for the highlight. Include preceding hash. Example #FF0000"),
      '#type' => 'textfield',
      '#size' => 7,
      '#maxlength' => 7,
      '#default_value' => $this->options['highlight_nodearg_color'],
//      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-style-options-highlight-nodearg' => array(TRUE)),
    );

  }

  /**
   * Validate the options form.
   */
  function options_validate(&$form, &$form_state) {
    // Check if highlight color is a valid hex color
    if (!preg_match('/^#[a-f0-9]{6}$/i', $form_state['values']['style_options']['highlight_nodearg_color'])) {
      form_error($form['highlight_nodearg_color'], t('Highlight colour must be a valid hex code in the form #FF0000.'));
    }
  }
}
