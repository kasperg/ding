<?php

/**
 * @file ding_library.admin.inc
 * Admin forms, page callbacks and related functions.
 *
 * Much of this stuff is a taken from the scaffolding example module.
 * http://cvs.drupal.org/viewvc.py/drupal/contributions/docs/developer/examples/scaffolding_example/
 */

/**
 * The main settings form.
 */
function ding_library_admin_settings_main_form(&$form_state) {
  $form = array();

  $form['placeholder'] = array(
    '#value' => 'This page intentionally left blank.',
  );

  return system_settings_form($form);
}

/**
 * The library ordering page.
 *
 * Provides a tabledrag sorting view of all the libraries, so they can 
 * be ordered as per the users preference.
 */
function ding_library_admin_ordering_form(&$form_state) {
  // Grab all the library nodes with their weight.
  $query = db_query("
    SELECT n.nid, n.vid, n.title, d.library_weight FROM {node} AS n
    LEFT JOIN {ding_library} AS d ON (d.vid = n.vid)
    WHERE n.type = 'library'
    ORDER BY d.library_weight, n.title;
  ");

  $libraries = array();
  $form = array();
  $form['records']['#tree'] = TRUE;

  while ($row = db_fetch_object($query)) {
    $form['records'][$row->nid] = _ding_library_admin_ordering_field($row);
    $libraries[$row->nid] = $row;
  }

  // Store the original library data, so we have it available when
  // saving our data, so we can figure out whether to do INSERT or UPDATE.
  $form['libraries'] = array(
    '#type' => 'value',
    '#value' => $libraries,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save changes'),
  );

  return $form;
}

/**
 * General submit handler for the drag-and-drop overview form.
 *
 * Updates the weights of all libraries on the form.
 *
 * @ingroup formapi
 * @see scaffolding_example_overview_form()
 */
function ding_library_admin_ordering_form_submit($form, &$form_state) {
  foreach ($form_state['values']['records'] as $nid => $val) {
    $orig_data = $form_state['values']['libraries'][$nid];

    // Since the library weight is NOT NULL in the database, it
    // being NULL here means that the node does not have a row in the
    // {ding_library} table.
    if (is_null($orig_data->library_weight)) {
      db_query("INSERT INTO {ding_library} (nid, vid, library_weight) VALUES (%d, %d, %d);", $nid, $orig_data->vid, $val['library_weight']);
    }
    else {
      db_query("UPDATE {ding_library} SET library_weight = %d WHERE vid = %d", $val['library_weight'], $orig_data->vid);
    }
  }
}

/**
 * Build the fields for a single record on the drag-and-drop overview form.
 */
function _ding_library_admin_ordering_field($row) {
  $field = array();

  $field['nid'] = array(
    '#type' => 'hidden',
    '#value' => $row->nid,
  );

  $field['title'] = array(
    '#value' => check_plain($row->title),
  );

  $field['library_weight'] = array(
    '#type' => 'weight',
    '#delta' => 50,
    '#default_value' => (is_null($row->library_weight)) ? 0 : $row->library_weight,
  );

  return $field;
}

/**
 * Theme the drag-and-drop overview form.
 *
 * Arranges records in a table, and adds the css and js for draggable sorting.
 *
 * @ingroup themeable
 * @ingroup forms
 */
function theme_ding_library_admin_ordering_form($form) {
  // Each record has a 'weight' that can be used to arrange it in relation to
  // other records. Drupal's tabledrag.js library allows users to control these
  // weights by dragging and dropping the records in a list -- we just need to
  // add identifying CSS classes to key elements in the table.

  $rows = array();
  foreach (element_children($form['records']) as $key) {
    $row = array();

    // Render the hidden 'node id' field and the title of the record into the
    // same column of the row.
    $row[] = drupal_render($form['records'][$key]['nid']) . drupal_render($form['records'][$key]['title']);

    // Add an identifying CSS class to our weight field, as it's the one
    // the tabledrag.js will be controlling. This can be anything we want it to
    // be, we'll just tell the tabledrag.js library what it should look for.
    $form['records'][$key]['library_weight']['#attributes']['class'] = 'library-weight';
    $row[] = drupal_render($form['records'][$key]['library_weight']);

    // Add the new row to our collection of rows, and give it the 'draggable'
    // class, indicating that it should be... well, draggable.
    $rows[] = array(
      'data' => $row,
      'class' => 'draggable',
    );
  }

  // If there were no records found, note the fact so users don't get confused
  // by a completely empty table.
  if (count($rows) == 0) {
    $rows[] = array(t('No records have been added.'), '<span class="scaffolding-example-weight"></span>', '');
  }

  // Render a list of header titles, and our array of rows, into a table. Even
  // we've already rendered all of our records, we always call drupal_render()
  // on the form itself after we're done, so hidden security fields and other
  // elements (like buttons) will appear properly at the bottom of the form.
  $header = array(t('Title'), t('Weight'));
  $output = theme('table', $header, $rows, array('id' => 'library-ordering-form'));
  $output .= drupal_render($form);

  // Now that we've built our output, tell Drupal to add the tabledrag.js library.
  // We'll pass in the ID of the table, the behavior we want it to use, and the
  // class that appears on each 'weight' form element it should be controlling.
  drupal_add_tabledrag('library-ordering-form', 'order', 'self', 'library-weight');

  return $output;
}

/**
 * Slug editing form.
 */
function ding_library_admin_slugs_form(&$form_state) {
  // Confirmation form for changing existing slugs. The storage array
  // is set by the submit handler, and as long as it is non-empty, we're
  // returned to the form.
  if (!empty($form_state['storage']['slug_changes'])) {
    $changes_text = array();
    foreach ($form_state['storage']['slug_changes'] as $change) {
      $changes_text[] = t('Change slug for %library from !old to !new', array(
        '%library' => $change['title'],
        '!old' => '<tt>' . check_plain($change['old_slug']) . '</tt>',
        '!new' => '<tt>' . check_plain($change['new_slug']) . '</tt>',
      ));
    }

    return confirm_form(array(), t('Please confirm these slug changes'), $_GET['q'], theme('item_list', $changes_text), t('Confirm'), t('Cancel'));
  }

  // Grab all the library nodes with their weight.
  $query = db_query("
    SELECT n.nid, n.title, p.value AS slug FROM {node} AS n
    LEFT JOIN {purl} AS p ON (p.id = n.nid AND provider = 'ding_library')
    WHERE n.type = 'library'
    ORDER BY n.title;
  ");

  $existing = array();
  $form = array();

  $form['warning'] = array(
    '#prefix' => '<div class="warning">',
    '#value' => t("Here you can edit the identifying part of a library's URL, called “slugs”. Be aware that changing the existing slugs will URLs to change, resulting in dead links."),
    '#suffix' => '</div>',
  );

  $form['records']['#tree'] = TRUE;
  while ($row = db_fetch_object($query)) {
    $form['records'][$row->nid] = array(
      '#type' => 'textfield',
      '#title' => $row->title,
      '#default_value' => $row->slug,
    );
    // Add the existing slugs to an array, so we'll be able to detect
    // overwriting in the validation.
    if (!empty($row->slug)) {
      $existing[$row->nid] = $row->slug;
    }
  }

  $form['existing_slugs'] = array(
    '#type' => 'value',
    '#value' => $existing,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save changes'),
  );

  return $form;
}

/**
 * Submit handler for the slug editing form.
 */
function ding_library_admin_slugs_form_submit($form, &$form_state) {
  // If the there's slug changes in the storage array, it means that
  // we've been through the confirmation form, and should now proceeed
  // to change the slugs in the database.
  if (isset($form_state['storage']['slug_changes'])) {
    foreach ($form_state['storage']['slug_changes'] as $change) {
      purl_save(array(
        'id' => $change['nid'],
        'provider' => 'ding_library',
        'value' => $change['new_slug'],
      ));

      // Unset the storage, so we'll won't get the confirmation page again.
      unset($form_state['storage']);
      return;
    }
  }

  $slug_changes = array();

  foreach ($form_state['values']['records'] as $nid => $slug) {
    if (!empty($slug)) {
      if (isset($form_state['values']['existing_slugs'][$nid]) && $form_state['values']['existing_slugs'][$nid] != $slug) {
        // Existing slug is being changed, store the changes so we can
        // create a confirmation form with it.
        $slug_changes[$nid] = array(
          'nid' => $nid,
          'title' => $form['records'][$nid]['#title'],
          'old_slug' => $form_state['values']['existing_slugs'][$nid],
          'new_slug' => ding_library_slugify($slug),
        );
      }
      else {
        // Save the newly added slug via PURL.
        purl_save(array(
          'id' => $nid,
          'provider' => 'ding_library',
          'value' => ding_library_slugify($slug),
        ));
      }
    }
  }
  $form_state['storage']['slug_changes'] = $slug_changes;
}

