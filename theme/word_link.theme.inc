<?php

/**
 * Render a word_link link.
 */
function theme_word_link($vars) {
  $tag = 'a';
  if ($vars['highlight']) {
    $tag = 'span';
    unset($vars['attributes']['href']);
  }
  $attributes = drupal_attributes($vars['attributes']);

  return '<' . $tag . $attributes . '>' . check_plain($vars['text']) . '</' . $tag . '>';
}
