<?php
// $Id$
/**
 * @file
 * User Relationships admin submit handlers
 */

/**
 * Validate the submitted relationship type
 */
function user_relationships_ui_type_edit_validate($form, &$form_state) {
  $select = db_select('user_relationship_types', 'urt')
    ->fields('urt', array('name'))
    ->where("LOWER(name) = LOWER(:name)", array(':name' => $form_state['values']['name']));
  if (isset($form_state['values']['rtid'])) {
    $select->condition('rtid', $form_state['values']['rtid'], '<>');
  }
  if ($select->execute()->fetchField()) {
    form_set_error('name', t('%name has already been used.', array('%name' => $form_state['values']['name'])));
  }

  $expire = $form_state['values']['expires_val'];
  if (!is_numeric($expire) || (int)$expire < 0) {
    form_set_error('expires_val', t('Expiration period must be an integer greater than or equal to 0.'));
  }
}

/***
 * Generic settings validation.
 */
function user_relationships_ui_setting_validation($element, $validations) {
  foreach ($validations as $validation => $info) {
    $arguments = array($element['#value']);
    if (isset($info['valid_options'])) {
      $arguments[] = $info['valid_options'];
    }

    $valid = function_exists($validation) && call_user_func_array($validation, $arguments);
    $valid = !$valid ? ($validation == 'is_positive' && is_numeric($arguments) && (int)$arguments > 0) : FALSE;
    $valid = !$valid ? ($validation == 'is_non_negative' && is_numeric($arguments) && (int)$arguments > -1) : FALSE;

    if (!$valid) {
      form_set_error($element['#name'], $info['msg']);
    }
  }
}

/**
 * Process the relationship edit page form submission.
 */
function user_relationships_ui_type_edit_submit($form, &$form_state) {
  $relationship_type = (object)$form_state['values'];
  if ($form_state['values']['action'] == 'add') {
    //#348025 when editing a type, make sure that two-way relationships are not left as reciprocal, just in case, as the UI allows it.
    if (!$relationship_type->is_oneway) {
      $relationship_type->is_reciprocal = 0;
    }
    drupal_write_record('user_relationship_types', $relationship_type);

    // Update strings for translation
    _user_relationships_ui_update_translations($relationship_type);

    $form_state['values']['rtid'] = $relationship_type->rtid;
    module_invoke_all('user_relationships_type_insert', $relationship_type);

    drupal_set_message(t('Relationship %name has been added.', array('%name' => $relationship_type->name)));
    watchdog(
      'u_relationship',
      'User Relationships form: relationship %name added.', array('%name' => $relationship_type->name),
      WATCHDOG_NOTICE,
      l(t('view'), 'admin/user/user_relationship/types/'. $relationship_type->rtid .'/edit')
    );
  }
  else {
    //#348025 when editing a type, make sure that two-way relationships are not left as reciprocal, just in case, as the UI allows it.
    if (!$relationship_type->is_oneway) {
      $relationship_type->is_reciprocal = 0;
    }
    drupal_write_record('user_relationship_types', $relationship_type, 'rtid');

    // Update strings for translation
    _user_relationships_ui_update_translations($relationship_type);
    module_invoke_all('user_relationships_type_update', $relationship_type);

    drupal_set_message(t('Relationship %name has been updated.', array('%name' => $relationship_type->name)));
    watchdog(
      'u_relationship',
      'User Relationships form: relationship %name updated.', array('%name' => $relationship_type->name),
      WATCHDOG_NOTICE,
      l(t('view'), 'admin/config/people/relationships/'. $relationship_type->rtid .'/edit')
    );
  }
  
  //save role permissions for this rtype
  db_delete('user_relationship_type_roles')
    ->condition('rtid', $relationship_type->rtid)
    ->execute();
  if ($form_state['values']['roles']) {
    $role_record = new StdClass;
    $role_record->rtid = $relationship_type->rtid;
    foreach ($form_state['values']['roles'] as $rid => $selected) {
      if ($rid == $selected) {
        $role_record->rid = $selected;
        drupal_write_record('user_relationship_type_roles', $role_record);
      }
    }
  }

  //save role receive permissions for this rtype
  db_delete('user_relationship_type_roles_receive')
    ->condition('rtid', $relationship_type->rtid)
    ->execute();
  if ($form_state['values']['roles_receive']) {
    $role_receive_record = new StdClass;
    $role_receive_record->rtid = $relationship_type->rtid;
    foreach ($form_state['values']['roles_receive'] as $rid => $selected) {
      if ($rid == $selected) {
        $role_receive_record->rid = $selected;
        drupal_write_record('user_relationship_type_roles_receive', $role_receive_record);
      }
    }
  }

  user_relationships_types_load(TRUE);
  menu_rebuild();

  $form_state['redirect'] = 'admin/config/people/relationships';
}


/**
 * Process relationship delete form submission.
 */
function user_relationships_ui_type_delete_submit($form, &$form_state) {
  $relationship_type = user_relationships_type_load($form_state['values']['rtid']);

  db_delete('user_relationship_type_roles')
    ->condition('rtid', $relationship_type->rtid)
    ->execute();
  db_delete('user_relationships')
    ->condition('rtid', $relationship_type->rtid)
    ->execute();
  db_delete('user_relationship_types')
    ->condition('rtid', $relationship_type->rtid)
    ->execute();

  module_invoke_all('user_relationships_type_delete', $relationship_type);

  // #371311 force menu rebuild
  user_relationships_types_load(TRUE);
  menu_rebuild();

  drupal_set_message(t('Relationship %name has been deleted.', array('%name' => $relationship_type->name)));
  watchdog(
    'u_relationship',
    'User Relationships form: relationship %name deleted.', array('%name' => $relationship_type->name),
    WATCHDOG_NOTICE
  );

  $form_state['redirect'] = 'admin/config/people/relationships';
}

/**
 * Make relationship type available to translation by i18nstrings.module
 */
function _user_relationships_ui_update_translations(&$relationship_type) {
  if (function_exists('i18nstrings_update') && $relationship_type->rtid) {
    i18nstrings_update("user_relationships:rtid:$relationship_type->rtid:name", $relationship_type->name);
    i18nstrings_update("user_relationships:rtid:$relationship_type->rtid:plural_name", $relationship_type->plural_name);
  }
}