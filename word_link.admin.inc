<?php

/**
 * @file
 * Administrative pages for the Word Link module.
 */

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
    'id' => array('data' => t('id'), 'field' => 'wl.id', 'sort' => 'desc'),
    'weight' => array('data' => t('weight'), 'field' => 'wl.weight'),
    'text' => array('data' => t('Text'), 'field' => 'wl.text'),
    'case_sensitive' => array('data' => t('Case sensitive'), 'field' => 'wl.case_sensitive'),
    'url' => array('data' => t('URL'), 'field' => 'wl.url'),
    'url_title' => array('data' => t('URL title'), 'field' => 'wl.url_title'),
    'class' => array('data' => t('Class'), 'field' => 'wl.class'),
    'except' => array('data' => t('Visibility'), 'field' => 'wl.except'),
    'operations' => array('data' => t('Operations')),
  );

  // Get words from DB.
  $words = word_link_load_table($header, 50);

  $destination = drupal_get_destination();
  $options = array();
  foreach ($words as $delta => $word) {
    $visibility = '';
    if (!empty($word->except)) {
      $visibility = $word->visibility ? t('Only on') . ': ' : t('Except') . ': ';
    }
    $options[$word->id] = array(
      'id' => $delta,
      'weight' => $word->weight,
      'text' => check_plain($word->text),
      'case_sensitive' => $word->case_sensitive ? t('Yes') : t('No'),
      'url' => array(
        'data' => array(
          '#type' => 'link',
          '#title' => check_plain($word->url),
          '#href' => $word->url,
          '#options' => array('attributes' => array('target' => '_blank')),
        ),
      ),
      'url_title' => check_plain($word->url_title),
      'class' => check_plain($word->class),
      'except' => $visibility . str_replace(array("\r\n", "\n", "\r"), " | ", check_plain(trim($word->except))),
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
          '#title' => check_plain($link['title']),
          '#href' => $link['href'],
          '#options' => array('query' => $link['query']),
        ),
      );
    }
  }
  // Only use a tableselect when the current user is able to perform any
  // operations.
  if (user_access('edit word link') || user_access('delete word link')) {
    $form['words'] = array(
      '#type' => 'tableselect',
      '#header' => $header,
      '#options' => $options,
      '#empty' => t('No words available.'),
    );
  }
  // Otherwise, use a simple table.
  else {
    unset($header['operations']);
    $form['words'] = array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $options,
      '#empty' => t('No words available.'),
    );
  }
  if (!empty($options) && user_access('delete word link')) {
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
      word_link_delete($wid);
      cache_clear_all('*', 'cache_filter', TRUE);
    }
  }
}

/**
 * Form builder for add or edit page.
 */
function word_link_add_form($form, &$form_state, $id = NULL) {
  $form['word_link_add_form'] = array(
    '#type' => 'fieldset',
    '#title' => t('Add word'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  if (!empty($id)) {
    $defaults = word_link_load($id);
    $form['word_link_add_form']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#weight' => 15,
      '#submit' => array('word_link_add_form_delete_submit'),
    );
  }
  $form['word_link_add_form']['text'] = array(
    '#type' => 'textfield',
    '#title' => t('Word/Phrase'),
    '#size' => 30,
    '#maxlength' => 255,
    '#description' => t('The word or phrase you wish to convert to a link. This field is case sensitive.'),
    '#required' => TRUE,
    '#default_value' => !empty($defaults->text) ? $defaults->text : '',
    '#autocomplete_path' => 'word-link/ajax/words',
  );
  $form['word_link_add_form']['weight'] = array(
    '#type' => 'textfield',
    '#title' => t('Weight'),
    '#size' => 3,
    '#maxlength' => 3,
    '#description' => t('Weight of the word. Lighter weights are higher up, heavier weights go down.'),
    '#required' => TRUE,
    '#default_value' => !empty($defaults->weight) ? $defaults->weight : 0,
  );
  $form['word_link_add_form']['case_sensitive'] = array(
    '#type' => 'checkbox',
    '#title' => t('Case Sensitivity'),
    '#description' => t('By default Word Link are case sensitive. Uncheck this checkbox if you want this particular Word Link to be case insensitive.'),
    '#default_value' => isset($defaults->case_sensitive) ? (int) $defaults->case_sensitive : 1,
  );
  $form['word_link_add_form']['url'] = array(
    '#type' => 'textfield',
    '#title' => t('URL'),
    '#size' => 30,
    '#maxlength' => 255,
    '#description' => t('The URL of the page to link to. External links must start with %http or %https and will be open in new window.', array('%http' => 'http://', '%https' => 'https://')),
    '#required' => TRUE,
    '#default_value' => isset($defaults->url) ? $defaults->url : '',
  );
  $form['word_link_add_form']['url_title'] = array(
    '#type' => 'textfield',
    '#title' => t('URL Title'),
    '#size' => 30,
    '#maxlength' => 255,
    '#description' => t('Title for the above URL. It will be embedded in the created link and appear as a tooltip when hovering the mouse over the link.'),
    '#default_value' => isset($defaults->url_title) ? $defaults->url_title : '',
  );
  $form['word_link_add_form']['class'] = array(
    '#type' => 'textfield',
    '#title' => t('Class'),
    '#size' => 30,
    '#maxlength' => 255,
    '#description' => t('Use this to add a class for the link. Default value is "word-link".'),
    '#default_value' => isset($defaults->class) ? $defaults->class : 'word-link',
  );
  $form['word_link_add_form']['visibility'] = array(
    '#type' => 'radios',
    '#title' => t('Show links on specific pages'),
    '#options' => array(
      0 => t('All pages except those listed'),
      1 => t('Only the listed pages'),
    ),
    '#default_value' => isset($defaults->visibility) ? $defaults->visibility : 0,
  );
  $form['word_link_add_form']['except'] = array(
    '#type' => 'textarea',
    '#description' => t('Specify pages by using their paths. Enter one path per line. E.g. node/1.'),
    '#default_value' => isset($defaults->except) ? $defaults->except : '',
  );
  $form['word_link_add_form']['id'] = array(
    '#type' => 'hidden',
    '#value' => !empty($id) ? $id : NULL,
  );
  $form['word_link_add_form']['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => 15,
    '#submit' => array('word_link_add_form_save_submit'),
  );

  return $form;
}

/**
 * Validate form for add or edit page.
 */
function word_link_add_form_validate($form, &$form_state) {
  if ($form_state['values']['op'] == t('Save')) {
    // @todo: Check if we need this verify.
//    $args = arg();
    $normal_path = drupal_get_normal_path($form_state['values']['url']);
//    $arg = empty($args[5]) ? 0 : $args[5];
//    $exist = word_link_exists(trim($form_state['values']['text']), $arg);
//    if ($exist) {
//      form_set_error('text', t('This word already exists.'));
//    }
    if (!drupal_valid_path($normal_path)) {
      form_set_error('url', t("The path '@url' is either invalid or you do not have access to it.", array('@url' => $form_state['values']['url'])));
    }
    if (!empty($form_state['values']['class']) && !preg_match('/^[_a-zA-Z]+[_a-zA-Z0-9-\s]*$/', trim($form_state['values']['class']))) {
      form_set_error('class', t('Class is not valid.'));
    }
  }
}

/**
 * Submit for delete action.
 */
function word_link_add_form_delete_submit($form, &$form_state) {
  $destination = array();
  if (isset($_GET['destination'])) {
    $destination = drupal_get_destination();
    unset($_GET['destination']);
  }
  cache_clear_all('*', 'cache_filter', TRUE);
  $form_state['redirect'] = array('admin/config/content/word-link/delete/' . $form_state['values']['id'], array('query' => $destination));
}

/**
 * Submit for save action.
 */
function word_link_add_form_save_submit($form, &$form_state) {
  $values = array(
    'text' => trim($form_state['values']['text']),
    'case_sensitive' => $form_state['values']['case_sensitive'],
    'url' => trim($form_state['values']['url']),
    'url_title' => trim($form_state['values']['url_title']),
    'class' => trim($form_state['values']['class']),
    'visibility' => $form_state['values']['visibility'],
    'except' => trim($form_state['values']['except']),
    'weight' => $form_state['values']['weight'],
  );
  if (isset($form_state['values']['id'])) {
    word_link_run_action('update', $values, $form_state['values']['id']);
    drupal_set_message(t('Done'));
  }
  else {
    word_link_run_action('insert', $values);
    drupal_set_message(t('Word successfully added'));
  }

  cache_clear_all('*', 'cache_filter', TRUE);

  $form_state['redirect'] = 'admin/config/content/word-link/list';
}

/**
 * Form builder for delete confirm page.
 */
function word_link_delete_form($form, &$form_state, $id) {
  $form['word_link_delete_form']['message'] = array(
    '#markup' => t('This action cannot be undone.'),
    '#prefix' => '<div class="form-item">',
    '#suffix' => '</div>',
  );
  $form['word_link_delete_form']['delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
  );
  $form['word_link_delete_form']['cancel'] = array(
    '#markup' => l(t('Cancel'), 'admin/config/content/word-link/list'),
  );
  $form['word_link_delete_form']['id'] = array(
    '#type' => 'hidden',
    '#value' => $id,
  );

  return $form;
}

/**
 * Submit form for delete confirm page.
 */
function word_link_delete_form_submit($form, &$form_state) {
  word_link_delete($form_state['values']['id']);
  cache_clear_all('*', 'cache_filter', TRUE);
  $form_state['redirect'] = 'admin/config/content/word-link/list';
}
