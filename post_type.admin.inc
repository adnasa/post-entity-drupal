<?php

/**
 * @file
 * post_type.admin.inc
 */

/**
 * UI controller.
 */
class PostTypeUIController extends EntityDefaultUIController {

  /**
   * Overrides hook_menu() defaults.
   */
  public function hook_menu() {
    $items = parent::hook_menu();
		$items[$this->path]['description'] = 'Manage post entity types, including adding
		and removing fields and the display of fields.';
    return $items;
  }
}

/**
 * Generates the post type editing form.
 */
function post_type_form($form, &$form_state, $post_type, $op = 'edit') {
  if ($op == 'clone') {
    $post_type->label .= ' (cloned)';
    $post_type->type = '';
  }

  $form['label'] = array(
    '#title' => t('Label'),
    '#type' => 'textfield',
    '#default_value' => $post_type->label,
    '#description' => t('The human-readable name of this post type.'),
    '#required' => TRUE,
    '#size' => 30,
  );
  // Machine-readable type name.
  $form['type'] = array(
    '#type' => 'machine_name',
    '#default_value' => isset($post_type->type) ? $post_type->type : '',
    '#maxlength' => 32,
//    '#disabled' => $post_type->isLocked() && $op != 'clone',
    '#machine_name' => array(
      'exists' => 'post_get_types',
      'source' => array('label'),
    ),
    '#description' => t('A unique machine-readable name for this post type. It must only contain lowercase letters, numbers, and underscores.'),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save post type'),
    '#weight' => 40,
  );

  if (!$post_type->isLocked() && $op != 'add') {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete post type'),
      '#weight' => 45,
      '#limit_validation_errors' => array(),
      '#submit' => array('post_type_form_submit_delete')
    );
  }
  return $form;
}

/**
 * Form API submit callback for the type form.
 */
function post_type_form_submit(&$form, &$form_state) {
  $post_type = entity_ui_form_submit_build_entity($form, $form_state);
  $post_type->save();
  $form_state['redirect'] = 'admin/structure/post_types';
}

/**
 * Form API submit callback for the delete button.
 */
function post_type_form_submit_delete(&$form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/post_types/manage/' . $form_state['post_type']->type . '/delete';
}
