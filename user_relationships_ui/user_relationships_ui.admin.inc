<?php

/**
 * @file
 * User Relationships admin settings and config forms
 */
module_load_include('inc', 'user_relationships_ui', 'user_relationships_ui.admin_actions');

/**
 * Main settings
 */
function user_relationships_ui_settings() {

  $form['settings'] = array(
    '#type' => 'vertical_tabs',
  );

  $form['general'] = array(
    '#type'   => 'fieldset',
    '#title'  => t('General'),
    '#weight' => -10,
    '#group' => 'settings',
  );
  $form['general']['user_relationships_allow_multiple'] = array(
    '#type'           => 'checkbox',
    '#title'          => t('Allow multiple relationships'),
    '#description'    => t('If checked, a user may create multiple relationships (each relationship of a different type) with another user.'),
    '#default_value'  => variable_get('user_relationships_allow_multiple', 1),
  );
  $form['general']['user_relationships_show_direct_links'] = array(
    '#type'           => 'checkbox',
    '#title'          => t('Show a separate link per relationship type'),
    '#description'    => t("On a user's page, show a separate link for each available relationship type (instead of the generic 'Create a relationship' link)."),
    '#default_value'  => variable_get('user_relationships_show_direct_links', 1),
  );
  $form['general']['user_relationships_show_user_pictures'] = array(
    '#type'           => 'checkbox',
    '#title'          => t("Show user pictures on relationship pages"),
    '#description'    => t("Show a picture next to each user's name on 'My relationships' pages."),
    '#default_value'  => variable_get('user_relationships_show_user_pictures', 0),
  );
  $form['general']['user_relationships_allow_auto_approve'] = array(
    '#type'           => 'checkbox',
    '#title'          => t('Allow users to auto approve'),
    '#description'    => t('Provide users with an option to automatically approve all requested relationships.'),
    '#default_value'  => variable_get('user_relationships_allow_auto_approve', 0),
  );
  $form['general']['user_relationships_relationships_per_page'] = array(
    '#type'           => 'textfield',
    '#title'          => t('Relationships per page'),
    '#size'           => 4,
    '#description'    => t('Number of relationships to show per page. If set to 0 all will be shown.'),
    '#default_value'  => variable_get('user_relationships_relationships_per_page', 16),
    '#validate'       => array('user_relationships_setting_validation' => array(array(
      'is_numeric' => array('msg' => t('The relationships per page setting is not an integer'))
    )))
  );
  $form['positioning'] = array(
    '#type'   => 'fieldset',
    '#title'  => t('AJAX Popup Positioning'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#description' => t('Some themes may require repositioning of AJAX confirmation dialogs. You may use these controls to set where the popup appears on the page or in relation to the mouse cursor.'),
    '#group' => 'settings',
  );
  $form['positioning']['user_relationships_enable_ajax_popups'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show AJAX confirmation popups'),
    '#default_value' => variable_get('user_relationships_enable_ajax_popups', 0),
    '#description' => t('Other popup settings below will only take effect if popups are enabled.')
  );
  $form['positioning']['user_relationships_position'] = array(
    '#type' => 'select',
    '#title' => t("Elaboration form's css position"),
    '#default_value' => variable_get('user_relationships_position', 'absolute'),
    '#options' => array(
      'absolute' => t('Mouse cursor'),
      'fixed' => t('Fixed'),
    ),
    '#description' => t('Sets the css <em>position</em> property of AJAX confirmation popups.'),
  );
  $form['positioning']['user_relationships_left'] = array(
    '#type' => 'textfield',
    '#title' => t("Elaboration form's css left value"),
    '#default_value' => variable_get('user_relationships_left', '0'),
    '#size' => 4,
    '#description' => t("Sets the css <em>left</em> property of AJAX confirmation popups. Try the value of 0 for 'Mouse cursor', or 0.5 for 'Fixed'. You may enter a distance in pixels, or as a % using a value 1 or less. Relative positioning requires a fixed position."),
  );
  $form['positioning']['user_relationships_top'] = array(
    '#type' => 'textfield',
    '#title' => t("Elaboration form's css top value"),
    '#default_value'  => variable_get('user_relationships_top', '0'),
    '#size' => 4,
    '#description' => t("Sets the css <em>top</em> property of AJAX confirmation popups. Try the value of 0 for 'Mouse cursor', or 0.4 for 'Fixed'. You may enter a distance in pixels, or as a % using a value 1 or less. Relative positioning requires a fixed position."),
  );

  $form['messages'] = array(
    '#type'         => 'fieldset',
    '#title'        => t('Custom Screen Messages'),
    '#weight'       => 0,
    '#description'  => t('Customize the confirmation messages displayed to users when specific relationship events occur.'),
    '#group' => 'settings',
  );

  $form['messages']['user_relationships_requests_link'] = array(
    '#type' => 'textfield',
    '#title' => t('Path to relationship requests'),
    '#default_value'  => variable_get('user_relationships_requests_link', 'relationships/requests'),
    '#description' => t("Only change this setting if a user's pending relationship requests have a different location than the default path (relationships/requests)"),
  );

  $form['messages']['messages_settings'] = array(
    '#type' => 'vertical_tabs',
  );

  $default_messages = _user_relationships_ui_default_messages(array());
  _user_relationships_ui_message_settings_form($form['messages'], $default_messages);

  //options for author pane integration
  if (function_exists('author_pane_api') && author_pane_api() == '2') {
    $rtypes = user_relationships_types_load();
    $form['author_pane'] = array(
      '#type'   => 'fieldset',
      '#title'  => t('Author Pane'),
      '#collapsible' => TRUE,
      '#group' => 'settings',
    );
    $form['author_pane']['user_relationships_enable_author_pane'] = array(
      '#type'           => 'checkbox',
      '#title'          => t('Show links in Author Pane'),
      '#description'    => t('Check if you wish to show add/remove relationship links in Author Pane.'),
      '#default_value'  => variable_get('user_relationships_enable_author_pane', 0),
    );
    // Since AP supports only one link per module, need to pick one relationship type
    if (count($rtypes)) {
      $options = array();
      foreach ($rtypes as $rtype) {
        $options[$rtype->rtid] = $rtype->name;
      }
      $form['author_pane']['user_relationships_api_author_pane_rtids'] = array(
        '#type' => 'select',
        '#multiple' => TRUE,
        '#title' => t('Relationship types to use'),
        '#default_value' => variable_get('user_relationships_api_author_pane_rtids', 'absolute'),
        '#options' => $options,
        '#description' => t('The chosen relationship types will be used for add/remove links.'),
      );
    }
  }
  return system_settings_form($form);
}


/**
 * Relationship type edit page.
 */
function user_relationships_ui_type_edit($form, &$form_state, $relationship_type = NULL) {
  if (!$relationship_type) {
    drupal_set_title(t('Add relationship type'));
    $relationship_type = (object) array(
      'name' => '',
      'plural_name' => '',
      'is_oneway' => FALSE,
      'is_reciprocal' => FALSE,
    );
  }
  else {
    drupal_set_title(t('Edit @name relationship type', array('@name' => $relationship_type->name)));
  }

  $form['tabs'] = array(
    '#type' => 'vertical_tabs',
  );

  $form['general'] = array(
    '#type' => 'fieldset',
    '#title' => t('General'),
    '#group' => 'tabs',
  );

  $form['general']['name'] = array(
    '#type'           => 'textfield',
    '#title'          => t('Name'),
    '#maxlength'      => 255,
    '#default_value'  => isset($relationship_type) ? $relationship_type->name : NULL,
    '#description'    => t("Example: buddy, friend, coworker, spouse."),
    '#required'       => TRUE,
    '#weight'         => -10,
  );
  $form['general']['plural_name'] = array(
    '#type'           => 'textfield',
    '#title'          => t('Plural name'),
    '#maxlength'      => 255,
    '#default_value'  => isset($relationship_type) ? $relationship_type->plural_name : NULL,
    '#description'    => t("Example: buddies, friends, coworkers, spouses."),
    '#required'       => TRUE,
    '#weight'         => -9,
  );
  $form['general']['requires_approval'] = array(
    '#type'           => 'checkbox',
    '#title'          => t('Requires Approval'),
    '#default_value'  => (isset($relationship_type->requires_approval) ? $relationship_type->requires_approval : 1),
    '#description'    => t('Check this if the requestee must approve the relationship'),
    '#weight'         => -8,
  );
  $form['general']['expires_val'] = array(
    '#title'          => t('Request expires in'),
    '#field_suffix'   => t('days'),
    '#type'           => 'textfield',
    '#size'           => 4,
    '#default_value'  => isset($relationship_type->expires_val) ? $relationship_type->expires_val : 0,
    '#description'    => t('After how many days should a request of this type be removed? (0 for never)'),
    '#weight'         => -7,
  );
  $form['general']['is_oneway'] = array(
    '#type'           => 'checkbox',
    '#title'          => t('This is a one-way relationship'),
    '#default_value'  => isset($relationship_type) ? $relationship_type->is_oneway : NULL,
    '#description'    => t('Check this if this relationship should only go one way (ex Manager, Subscriber)'),
    '#weight'         => -6,
  );
  $form['general']['is_reciprocal'] = array(
    '#type'           => 'checkbox',
    '#title'          => t('This one-way relationship can be reciprocated'),
    '#default_value'  => isset($relationship_type) ? $relationship_type->is_reciprocal : NULL,
    '#description'    => t('Check if this one-way relationship can go either way'),
    '#weight'         => -5,
  );

  $form['roles'] = array(
    '#type' => 'fieldset',
    '#title' => t('Allowed Roles'),
    '#group' => 'tabs',
    '#weight' => 10,
  );

  $form['roles']['roles'] = array(
    '#type' => 'checkboxes',
    '#options' => user_roles(TRUE),
    '#default_value' => isset($relationship_type->roles) ? $relationship_type->roles : array(),
    '#title' => t('Role request access'),
    '#description' => t('You may choose which roles are allowed to request this relationship. If none are selected, all roles are allowed.'),
  );
  $form['roles']['roles_receive'] = array(
    '#type' => 'checkboxes',
    '#options' => user_roles(TRUE),
    '#default_value' => isset($relationship_type->roles_receive)  && is_array($relationship_type->roles_receive) ? $relationship_type->roles_receive : array(),
    '#title' => t('Role receive access'),
    '#description' => t('You may choose which roles are allowed to receive this relationship. If none are selected, all roles are allowed to receive the relationship.'),
  );
  $form['rtid'] = array(
    '#type'   => 'value',
    '#value'  => isset($relationship_type->rtid) ? (int)$relationship_type->rtid : NULL,
  );
  $form['action'] = array(
    '#type'   => 'value',
    '#value'  => (isset($relationship_type->rtid) ? 'edit' : 'add'),
  );

  $form['submit'] = array(
    '#type'   => 'submit',
    '#value'  => t('Submit'),
    '#weight' => 10,
  );

  return $form;
}


/**
 * Relationship type delete page.
 */
function user_relationships_ui_type_delete($form, &$form_state, $relationship_type = NULL) {
  if ($relationship_type) {
    $form['rtid'] = array(
      '#type'   => 'value',
      '#value'  => (int)$relationship_type->rtid,
    );

    return confirm_form(
      $form,
      t('Are you sure you want to delete %name?', array('%name' => ur_tt("user_relationships:rtid:$relationship_type->rtid:name", $relationship_type->name))),
      'admin/config/people/relationships',
      t('This action cannot be undone.'),
      t('Delete'), t('Cancel')
    );
  }
  else {
    drupal_set_message(user_relationships_ui_get_message('non_existent_type'));
    drupal_goto('admin/config/people/relationships');
  }
}
