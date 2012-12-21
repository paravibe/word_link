<?php

/**
 * @file
 * Administrative pages for the Word Link module.
 */

/**
 * Form builder for a settings page.
 */
function word_link_admin_settings() {
  $form['#tree'] = TRUE;
  $form['word_link_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Settings'),
    '#collapsible' => FALSE,
  );
  $form['word_link_settings']['word_link_node_types'] = array(
    '#type' => 'fieldset',
    '#title' => t('Node types'),
    '#description' => t('Choose the node types and fields into which Word Link will automatically insert links.'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  $node_types = node_type_get_names();
  foreach ($node_types as $type => $name) {
    $fields = word_link_get_node_fields($type);
    $type_defaults = variable_get('word_link_node_types', array());
    if (!empty($fields)) {
      $form['word_link_settings']['word_link_node_types'][$type]['status'] = array(
        '#type' => 'checkbox',
        '#title' => $name,
        '#default_value' => isset($type_defaults[$type]['status']) ? $type_defaults[$type]['status'] : 0,
      );
      $form['word_link_settings']['word_link_node_types'][$type]['fields'] = array(
        '#type' => 'select',
        '#title' => t('@type fields', array('@type' => $name)),
        '#options' => $fields,
        '#multiple' => TRUE,
        '#default_value' => isset($type_defaults[$type]['fields']) ? $type_defaults[$type]['fields'] : NULL,
      );
    }
  }
  $form['word_link_settings']['word_link_css'] = array(
    '#type' => 'checkbox',
    '#title' => t('Add default CSS'),
    '#description' => t('If turned on default CSS from module will be added.'),
    '#default_value' => variable_get('word_link_css', 1),
  );
  $form['word_link_settings']['word_link_limit'] = array(
    '#type' => 'textfield',
    '#size' => 5,
    '#maxlenghth' => 3,
    '#title' => t('Convert limit'),
    '#description' => t('Set the maxium instances that will be converted. Use 0 if you want all instances in the node to be converted.'),
    '#default_value' => variable_get('word_link_limit', 0),
  );
  $form['word_link_settings']['word_link_tags_except'] = array(
    '#type' => 'textfield',
    '#title' => t('Disallowed HTML tags'),
    '#description' => t('A list of HTML tags that will be ignored. Never enter here tags that are not text. E.g. @tags.', array('@tags' => '<img>')),
    '#default_value' => variable_get('word_link_tags_except', '<h1> <h2> <h3> <h4> <h5> <h6>'),
  );
  $form['word_link_add_form']['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Form validation for a settings page.
 */
function word_link_admin_settings_validate($form, &$form_state) {
  if (!is_numeric($form_state['values']['word_link_settings']['word_link_limit'])) {
    form_set_error('word_link_settings][word_link_limit', t('Replace limit must be numeric.'));
  }
}

/**
 * Submit form for a settings page.
 */
function word_link_admin_settings_submit($form, &$form_state) {
  variable_set('word_link_limit', $form_state['values']['word_link_settings']['word_link_limit']);
  variable_set('word_link_tags_except', $form_state['values']['word_link_settings']['word_link_tags_except']);
  variable_set('word_link_node_types', $form_state['values']['word_link_settings']['word_link_node_types']);
  variable_set('word_link_css', $form_state['values']['word_link_settings']['word_link_css']);
  drupal_set_message(t('Saved'));
}

/**
 * Buid a Word Link list page.
 */
function word_link_list_page_form($form, $form_state) {
  // Return confirm page when need to delete words.
  if (!empty($form_state['values']['words'])) {
    return word_link_list_page_form_delete_confirm($form, $form_state, array_filter($form_state['values']['words']));
  }
  // Build the sortable table header.
  $header = array(
    'text' => array('data' => t('Text'), 'field' => 'wl.text'),
    'case_sensitive' => array('data' => t('Case sensitive'), 'field' => 'wl.case_sensitive'),
    'url' => array('data' => t('URL'), 'field' => 'wl.url'),
    'url_title' => array('data' => t('URL Title'), 'field' => 'wl.url_title'),
    'class' => array('data' => t('Class'), 'field' => 'wl.class'),
    'except' => array('data' => t('Except'), 'field' => 'wl.except'),
    'operations' => array('data' => t('Operations')),
  );
  // Get words from DB.
  $words = word_link_get_link(NULL, $header, 50);

  $destination = drupal_get_destination();
  $options = array();
  foreach ($words as $delta => $word) {
    $options[$word->id] = array(
      'text' => check_plain($word->text),
      'case_sensitive' => $word->case_sensitive ? t('Yes') : t('No'),
      'url' => array(
        'data' => array(
          '#type' => 'link',
          '#title' => $word->url,
          '#href' => $word->url,
          '#options' => array('attributes' => array('target' => '_blank')),
        ),
      ),
      'url_title' => check_plain($word->url_title),
      'class' => check_plain($word->class),
      'except' => str_replace(array("\r\n", "\n", "\r"), " | ", check_plain(trim($word->except))),
    );

    // Build a list of all the accessible operations for the current word.
    $operations = array();
    if (user_access('edit word link')) {
      $operations['edit'] = array(
        'title' => t('edit'),
        'href' => 'admin/config/content/word-link/edit/' . $word->id,
        'query' => $destination,
      );
    }
    if (user_access('delete word link')) {
      $operations['delete'] = array(
        'title' => t('delete'),
        'href' => 'admin/config/content/word-link/delete/' . $word->id,
        'query' => $destination,
      );
    }
    $options[$word->id]['operations'] = array();
    if (count($operations) > 1) {
      // Render an unordered list of operations links.
      $options[$word->id]['operations'] = array(
        'data' => array(
          '#theme' => 'links__node_operations',
          '#links' => $operations,
          '#attributes' => array('class' => array('links', 'inline')),
        ),
      );
    }
    elseif (!empty($operations)) {
      // Render the first and only operation as a link.
      $link = reset($operations);
      $options[$word->id]['operations'] = array(
        'data' => array(
          '#type' => 'link',
          '#title' => $link['title'],
          '#href' => $link['href'],
          '#options' => array('query' => $link['query']),
        ),
      );
    }
  }
  // Only use a tableselect when the current user is able to perform any
  // operations.
  if (user_access('view word link')) {
    $form['words'] = array(
      '#type' => 'tableselect',
      '#header' => $header,
      '#options' => $options,
      '#empty' => t('No words available.'),
    );
  }
  // Otherwise, use a simple table.
  else {
    $form['words'] = array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $options,
      '#empty' => t('No words available.'),
    );
  }
  if (!empty($options)) {
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Delete selected'),
    );
  }
  $form['pager'] = array('#markup' => theme('pager'));

  return $form;
}

/**
 * Submit for a Word Link list page.
 */
function word_link_list_page_form_submit($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
}

/**
 * Confirm form for delete operation.
 */
function word_link_list_page_form_delete_confirm($form, &$form_state, $words) {
  $form['words']['#tree'] = TRUE;
  foreach ($words as $wid) {
    $form['words'][$wid] = array(
      '#type' => 'hidden',
      '#value' => $wid,
    );
  }
  $form['#submit'][] = 'word_link_list_page_form_delete_confirm_submit';
  $confirm_question = format_plural(count($words), 'Are you sure you want to delete this item?', 'Are you sure you want to delete these items?');
  return confirm_form($form, $confirm_question, 'admin/config/content/word-link/list', t('This action cannot be undone.'), t('Delete'), t('Cancel'));
}

/**
 * Submit for delete operation.
 */
function word_link_list_page_form_delete_confirm_submit($form, &$form_state) {
  foreach ($form_state['values']['words'] as $wid) {
    if (!empty($wid) && is_numeric($wid)) {
      word_link_del_link($wid);
    }
  }
}

/**
 * Form builder for add or edit page.
 */
function word_link_add_form($form, &$form_state, $id = NULL) {
  if (!empty($id)) {
    $defaults = word_link_get_link($id);
    $form['word_link_add_form']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
    );
  }
  $form['word_link_add_form'] = array(
    '#type' => 'fieldset',
    '#title' => t('Add word'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  $form['word_link_add_form']['text'] = array(
    '#type' => 'textfield',
    '#title' => t('Word/Phrase'),
    '#size' => 30,
    '#maxlength' => 255,
    '#description' => t('The word or phrase you wish to convert to a link. This field is case sensitive.'),
    '#required' => TRUE,
    '#default_value' => !empty($defaults[$id]->text) ? $defaults[$id]->text : '',
  );
  $form['word_link_add_form']['case_sensitive'] = array(
    '#type' => 'checkbox',
    '#title' => t('Case Sensitivity'),
    '#description' => t('By default Word Link are case sensitive. Uncheck this checkbox if you want this particular Word Link to be case insensitive.'),
    '#default_value' => isset($defaults[$id]->case_sensitive) ? (int) $defaults[$id]->case_sensitive : 1,
  );
  $form['word_link_add_form']['url'] = array(
    '#type' => 'textfield',
    '#title' => t('URL'),
    '#size' => 30,
    '#maxlength' => 255,
    '#description' => t('The URL of the page to link to. External links must include "http://"'),
    '#required' => TRUE,
    '#default_value' => isset($defaults[$id]->url) ? $defaults[$id]->url : '',
  );
  $form['word_link_add_form']['url_title'] = array(
    '#type' => 'textfield',
    '#title' => t('URL Title'),
    '#size' => 30,
    '#maxlength' => 255,
    '#description' => t('Title for the above URL. It will be embedded in the created link and appear as a tooltip when hovering the mouse over the link.'),
    '#default_value' => isset($defaults[$id]->url_title) ? $defaults[$id]->url_title : '',
  );
  $form['word_link_add_form']['class'] = array(
    '#type' => 'textfield',
    '#title' => t('Class'),
    '#size' => 30,
    '#maxlength' => 255,
    '#description' => t('Use this to add a class for the link. Default value is "word-link".'),
    '#default_value' => isset($defaults[$id]->class) ? $defaults[$id]->class : 'word-link',
  );
  $form['word_link_add_form']['except'] = array(
    '#type' => 'textarea',
    '#title' => t('Do not show links on specific pages'),
    '#description' => t('Specify pages by using their paths. Enter one path per line. E.g. node/1.'),
    '#default_value' => isset($defaults[$id]->except) ? $defaults[$id]->except : '',
  );
  $form['word_link_add_form']['id'] = array(
    '#type' => 'hidden',
    '#value' => !empty($id) ? $id : NULL,
  );
  $form['word_link_add_form']['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Validate form for add or edit page.
 */
function word_link_add_form_validate($form, &$form_state) {
  $args = arg();
  $arg = empty($args[5]) ? 0 : $args[5];
  $exist = word_link_get_link_text(trim($form_state['values']['text']), $arg);
  if ($exist) {
    form_set_error('text', t('This word already exists.'));
  }
}

/**
 * Submit form for add or edit page.
 */
function word_link_add_form_submit($form, &$form_state) {
  $values = array(
    'text' => trim($form_state['values']['text']),
    'case_sensitive' => $form_state['values']['case_sensitive'],
    'url' => trim($form_state['values']['url']),
    'url_title' => trim($form_state['values']['url_title']),
    'class' => trim($form_state['values']['class']),
    'except' => trim($form_state['values']['except']),
  );
  if (isset($form_state['values']['id'])) {
    if ($form_state['values']['op'] == 'Delete') {
      word_link_del_link($form_state['values']['id']);
    }
    elseif ($form_state['values']['op'] == 'Save') {
      word_link_add_update_link('update', $values, $form_state['values']['id']);
    }
    drupal_set_message(t('Done'));
  }
  else {
    word_link_add_update_link('insert', $values);
    drupal_set_message(t('Link succesfuly added'));
  }
  $form_state['redirect'] = 'admin/config/content/word-link/list';
}

/**
 * Form builder for delete confirm page.
 */
function word_link_del_form($form, &$form_state, $id) {
  $form['word_link_del_form']['message'] = array(
    '#markup' => t('This action cannot be undone.'),
    '#prefix' => '<div class="form-item">',
    '#suffix' => '</div>',
  );
  $form['word_link_del_form']['delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
  );
  $form['word_link_del_form']['cancel'] = array(
    '#markup' => l(t('Cancel'), 'admin/config/content/word-link/list'),
  );
  $form['word_link_del_form']['id'] = array(
    '#type' => 'hidden',
    '#value' => $id,
  );

  return $form;
}

/**
 * Submit form for delete confirm page.
 */
function word_link_del_form_submit($form, &$form_state) {
  word_link_del_link($form_state['values']['id']);
  $form_state['redirect'] = 'admin/config/content/word-link/list';
}
