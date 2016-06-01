<?php
/**
 * @file
 * Point chart visualisation.
 */

$plugin = array(
  'title' => t('Scatter Plot chart'),
  'settings' => array(
    'rotated' => 'false',
    'show_labels' => 0,
    'point_size' => 2.5,
    'grid' => NULL,
    'palette_override' => NULL,
    'show_title' => 1,
    'x_label' => NULL,
    'y_label' => NULL,
    'column_overrides' => array(),
    'label_settings' => array(
      'overrides' => NULL,
    ),
    'axis_settings' => array(
      'x_tick_rotate' => 0,
      'x_tick_count' => NULL,
      'y_tick_count' => NULL,
      'x_tick_cull' => NULL,
    ),
    'ckan_filters' => array(
      'search' => NULL,
      'filters' => NULL,
    ),
  ),
);

/**
 * Returns a renderable array that represents the block content.
 */
function govcms_ckan_display_scatter_plot_chart_view($file, $display, $config) {
  $element = array();
  $chart_class = 'ckan-point-chart';
  $response = govcms_ckan_client_request_records($file->resource_id, $config['ckan_filters']['search'], $config['ckan_filters']['filters']);

  // If failure, provide error message.
  if ($response->valid === FALSE) {
    $element['#theme'] = 'ckan_empty_visualisation';
  }
  else {

    // Setup our configuration.
    $keys = array_filter($config['keys']);
    $column_overrides = govcms_ckan_display_parse_column_overrides($config['column_overrides']);
    $label_replacements = govcms_ckan_string_to_array($config['label_settings']['overrides']);

    // Attributes for the table.
    $attributes = array(
      'class' => array('ckan-chart', $chart_class),

      // Entity settings.
      'data-type' => 'scatter',
      'data-rotated' => $config['rotated'],
      'data-labels' => ($config['show_labels'] == 1 ? 'true' : 'false'),
      'data-pointSize' => $config['point_size'],
      'data-grid' => $config['grid'],
      'data-showTitle' => ($config['show_title'] == 1 ? 'true' : 'false'),
      'data-title' => $file->filename,
      'data-xLabel' => $config['x_label'],
      'data-yLabel' => $config['y_label'],
      'data-xTickRotate' => ($config['axis_settings']['x_tick_rotate'] == 1 ? '90' : '0'),
      'data-xTickCount' => $config['axis_settings']['x_tick_count'],
      'data-yTickCount' => $config['axis_settings']['y_tick_count'],
      'data-xTickCull' => $config['axis_settings']['x_tick_cull'],

      // Display settings.
      'data-palette' => (!empty($config['palette_override']) ? $config['palette_override'] : $config['palette']),
      'data-exportWidth' => $config['export_width'],
      'data-exportHeight' => $config['export_height'],
    );

    // Parse the data.
    $parser = govcms_ckan_dataset_parser($response->data);
    $parser
      ->setKeys($keys)
      ->setLabelKey($config['labels'])
      ->setHeaderSource($config['x_axis_grouping'])
      ->setLabelReplacements($label_replacements)
      ->setTableAttributes($attributes)
      ->setColumnAttributes($column_overrides);

    // If splitting on the value of a key.
    if ($config['split']) {
      $parser->setGroupKey($config['split']);
    }

    // Return the parsed tables in a wrapper.
    $element = array(
      '#theme' => 'ckan_display_table_wrapper',
      '#tables' => $parser->parse(),
      '#show_titles' => !empty($config['split']),
    );

    // Add the JS to the tables.
    govcms_ckan_display_attach_charts($element, '.' . $chart_class);
  }

  return $element;
}


/**
 * Configure form callback.
 */
function govcms_ckan_display_scatter_plot_chart_configure($plugin, $form, $form_state, $config) {
  // Get default key elements.
  $config_form = govcms_ckan_media_visualisation_default_key_config($form, $form_state, $config);

  // Point chart specific settings.
  $config_form['rotated'] = array(
    '#type' => 'select',
    '#title' => t('Orientation'),
    '#default_value' => $config['rotated'],
    '#options' => array(
      'false' => t('Vertical'),
      'true' => t('Horizontal'),
    ),
  );

  $config_form['show_labels'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable data labels'),
    '#default_value' => $config['show_labels'],
  );

  $config_form['point_size'] = array(
    '#type' => 'textfield',
    '#title' => t('Point size'),
    '#default_value' => $config['point_size'],
    '#description' => t('Default point size is 2.5'),
  );

  $config_form['palette_override'] = array(
    '#title' => t('Palette override'),
    '#type' => 'textfield',
    '#default_value' => $config['palette_override'],
    '#description' => t('Palette is a comma separated list of hex values. If not set, default palette is applied.'),
  );

  $config_form['grid'] = array(
    '#type' => 'select',
    '#title' => t('Enable grid'),
    '#default_value' => $config['grid'],
    '#empty_option' => t('None'),
    '#options' => array(
      'x' => t('X Lines'),
      'y' => t('Y Lines'),
      'xy' => t('Both X and Y lines'),
    ),
  );

  // Axis settings.
  $axis_config_form = govcms_ckan_media_visualisation_default_axis_config($form, $form_state, $config);
  $config_form = array_merge($config_form, $axis_config_form);

  return $config_form;
}