<?php

/**
 * Implementation of hook_features_export_options().
 *
 * This hook tells features what items of this component are available for export.
 *
 * @return array
 *   A keyed array of items, suitable for use with a FormAPI select or
 *   checkboxes element.
 */
function nodequeue_export_features_export_options() {
  $options = array();

  // Bypass cache to get all the queues straight from the db.
  $queues = nodequeue_load_queues(nodequeue_get_all_qids(0), TRUE);
  foreach ($queues as $qid => $data) {
    $options[$data->name] = $data->title;
  }

  return $options;
}

/**
 * Implementation of hook_features_export().
 *
 * When one of these components is selected on the features page, this hook
 * includes the item (plus any dependencies) in the export array.
 *
 * @param array $data
 *   The machine name for this component.
 * @param array &$export
 *   An array of components to be exported.
 * @param string $module_name
 *   The name of the feature module that will be created.
 * @return array
 *   An array of functions for further processing (if desired).
 */
function nodequeue_export_features_export($data, &$export, $module_name) {
  // These are the dependencies for this module to function properly
  $export['dependencies']['nodequeue_export'] = 'nodequeue';
  $export['dependencies']['nodequeue_export'] = 'nodequeue_export';

  // This is a simple straight object export.
  foreach ($data as $component) {
    $export['features']['nodequeue_export'][$component] = $component;
  }

  return array();
}

/**
 * Implementation of hook_features_export_render().
 *
 * This hook is called to export the component(s) selected via the features UI.
 *
 * @param string $module_name
 *   The name of the feature module to be exported.
 * @param array $data
 *   An array of machine name identifiers for the rendered objects.
 * @param array $export
 *   An array with the full export for the feature (only called during update or recreate).
 * @return array
 *   The PHP code (an array) that will be rendered.
 */
function nodequeue_export_features_export_render($module_name, $data, $export = NULL) {
  $code = array();

  $code[] = ' $nodequeue_export = array();';
  $code[] = '';

  foreach ($data as $machine_name) {
    // Retrieve the nodequeue.
    $queue = nodequeue_load_queue_by_name($machine_name);
    // Format it in prep for export.
    $queue_data = _nodequeue_export_format_queue_data($queue);
    // Add the nodequeue to the feature.
    if (isset($queue_data)) {
      $code[] = ' $nodequeue_export[\'' .$queue_data['name'] .'\'] = ' .features_var_export($queue_data, ' ' ) .';';
    }
  }

  $code[] = ' return $nodequeue_export;';
  $code = implode("\n", $code);
  return array('nodequeue_export_features_default_settings' => $code);
}

/**
 * Implementation of hook_features_rebuild(). [component_hook]
 *
 * This is called whenever a new feature is enabled, or reverted.
 */
function nodequeue_export_features_rebuild($module) {
  // Look for exported nodequeues.
  $queues = module_invoke($module, 'nodequeue_export_features_default_settings');
  // Loop over the queues to find the ones we need to recreate.
  foreach ($queues as $id => $data) {
    // Check to see if the queue already exists.
    $exists = nodequeue_load_queue_by_name($data['name']);;
    // If it doesn't, store the queue info in the database.
    if (empty($exists)) {
      $saved = _nodequeue_export_store_queue_data($data);
    }
  }
}

/**
 * Implementation of hook_features_revert(). [component_hook]
 */
function nodequeue_export_features_revert($module) {
  nodequeue_export_features_rebuild($module);
}

/**
 * Take a nodequeue, loaded with the API, and format the data structure for export.
 */
function _nodequeue_export_format_queue_data($queue) {
  $queue_data = array(
    'name' => $queue->name,
    'title' => $queue->title,
    'subqueue_title' => $queue->subqueue_title,
    'size' => $queue->size,
    'reverse' => $queue->reverse,
    'link' => $queue->link,
    'link_remove' => $queue->link_remove,
    'owner' => $queue->owner,
    'roles' => $queue->roles,
    'types' => $queue->types,
    'reference' => $queue->reference,
    'i18n' => $queue->i18n,
    'show_in_links' => $queue->show_in_links,
    'show_in_tab' => $queue->show_in_tab,
    'show_in_ui' => $queue->show_in_ui,
    'new' => TRUE,
  );

  if (empty($queue->owner)) {
    $queue_data['add_subqueue'] = array($queue->title);
  }

  return $queue_data;
}

/**
 * Handle the actual creation of queue storage in the database.
 */
function _nodequeue_export_store_queue_data($data) {
  $data = (object) $data;
  if (isset($data->name) && $dataname !== '') {
    $queue = array(
      'name' => $data->name,
      'title' => $data->title,
      'subqueue_title' => $data->subqueue_title,
      'size' => $data->size,
      'reverse' => $data->reverse,
      'link' => $data->link,
      'link_remove' => $data->link_remove,
      'owner' => $data->owner,
      'roles' => $data->roles,
      'reference' => $data->reference,
      'types' => $data->types,
      'i18n' => $data->i18n,
      'show_in_links' => $data->show_in_links,
      'show_in_tab' => $data->show_in_tab,
      'show_in_ui' => $data->show_in_ui,
      'new' => TRUE,
    );

    if (!empty($data->add_subqueue)) {
      $queue['add_subqueue'] = $data->add_subqueue;
    }

    $queue = (object) $queue;
    nodequeue_save($queue);
  }
}
