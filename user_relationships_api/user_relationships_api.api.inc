<?php
// $Id$

/**
 * @file
 * User Relationships API. Data abstraction layer
 * @author Jeff Smick (creator)
 * @author Alex Karshakevich (maintainer) http://drupal.org/user/183217
 */

/**
 * Public API for retrieving a specific relationship
 *
 * @param $param
 *    The rtid or an associative array of attributes to search for in selecting the
 *    relationship, such as rtid or name. Attributes must match column names
 *    in the user_relationship_types table.
 *
 * @param $reset
 *    a boolean that forces a reset of the internal static types list
 *
 * @return
 *    object of the requested relationship type
 *
 */
function user_relationships_type_load($param = array(), $reset = NULL) {
  $types = user_relationships_types_load($reset);

  if (!is_array($param)) {
    if (isset($types[$param])) {
      return $types[$param];
    }
    return NULL;
  }

  foreach ($types as $type) {
    $found = TRUE;

    foreach ($param as $column => $value) {
      $column = strtolower($column);

      if ($column == 'name' || $column == 'plural_name') {
        $value = strtolower($value);
        $col_val = strtolower($type->$column);
      }
      else {
        $col_val = $type->$column;
      }

      // mismatch, move to the next type
      if ($col_val != $value) {
        $found = FALSE;
        break;
      }
    }

    if ($found) {
      return $type;
    }
  }
}


/**
 * Public API for loading the full list of relationship types.
 *
 * @param $reset
 *    a boolean that forces a reset of the internal static types list
 *
 * @return
 *    array of relationship_type objects
 */
function user_relationships_types_load($reset = NULL) {
  static $relationship_types_list = array();

  if ($reset || empty($relationship_types_list)) {
    //clear the cached list, since some relationships may have disappeared
    $relationship_types_list = db_query("SELECT * FROM {user_relationship_types}")->fetchAllAssoc('rtid');
    //load role permissions for all types
    $result = db_query('SELECT rtid, rid FROM {user_relationship_type_roles} ORDER BY rtid, rid');
    foreach ($result as $mapping) {
      $rtype = $relationship_types_list[$mapping->rtid];
      // //roles is an associative array {role id} => {role id}
      $rtype->roles[$mapping->rid] = $mapping->rid;//] = $mapping->name;
    }

    //load role receive permissions for all types
    $results = db_query('SELECT rtid, rid FROM {user_relationship_type_roles_receive} ORDER BY rtid, rid');
    foreach ($results as $mapping) {
      $rtype = $relationship_types_list[$mapping->rtid];
      // //roles is an associative array {role id} => {role id}
      $rtype->roles_receive[$mapping->rid] = $mapping->rid;//] = $mapping->name;
    }
  }

  module_invoke_all('user_relationships_type_load', $relationship_types_list);

  return $relationship_types_list;
}


/**
 * Create or Update a User Relationship Type
 *
 * @param $rtype
 *   A User Relationship type object
 */
function user_relationships_type_save($rtype) {
  module_invoke_all('user_relationships_type_presave', $rtype);

  $op = (isset($rtype->rtid) && $rtype->rtid) ? 'update' : 'insert';

  // find a relationship type with the name we're trying to save
  // if it's an update action check to see that the rtypes match
  // otherwise it's just invalid
  if (
    ($found_rt = user_relationships_type_load(array('name' => $rtype->name))) &&
    ($op == 'update' ? $found_rt->rtid != $rtype->rtid : TRUE)
  ) {
    return FALSE;
  }

  // ensure "expires_val" is numeric and not negative
  if (!is_numeric($rtype->expires_val) || $rtype->expires_val < 0) {
    $rtype->expires_val = 0;
  }

  drupal_write_record('user_relationship_types', $rtype, ($op == 'update' ? 'rtid' : array()));

  module_invoke_all('user_relationships_type_' . $op, $rtype);
}


/**
 * Delete a User Relationship Type
 *
 * @param $rtid
 *   A User Relationship type ID
 */
function user_relationships_type_delete($rtid) {
  $rtype = user_relationships_type_load($rtid);

  db_delete('user_relationship_types')
    ->condition('rtid', $rtid)
    ->execute();
  db_delete('user_relationships')
    ->condition('rtid', $rtid)
    ->execute();
  db_delete('user_relationship_type_roles')
    ->condition('rtid', $rtid)
    ->execute();
  db_delete('user_relationship_type_roles_receive')
    ->condition('rtid', $rtid)
    ->execute();

  module_invoke_all('user_relationships_type_delete', $rtype);
}


/**
 * Request a new user relationship
 *
 * @param $requester
 *   object or UID of the requester
 * @param $requestee
 *   object  or UID of the requestee
 * @param $type
 *   object or RTID of the relationship type
 * @param $approved
 *    boolean status of the relationship
 *
 * @return
 *    object of the newly created relationship
 */
function user_relationships_request_relationship($requester, $requestee = NULL, $type = NULL, $approved = FALSE) {
  //#578372 check if this is a "new" type of invocation, where first argument is a relationship object
  if (is_object($requester) && isset($requester->requester) && isset($requester->requestee) && isset($requester->type)) {
    $relationship = $requester;
    $requester = $relationship->requester;
    $requestee = $relationship->requestee;
    $type = $relationship->type;
    if (isset($relationship->approved)) {
      $approved = $relationship->approved;
    }
  }
  else {
    $relationship = new stdClass;
  }
  // translate an ID into an object
  foreach (array('requester' => $requester, 'requestee' => $requestee, 'type' => $type) as $key => $value) {
    if (!is_object($value)) {
      $$key = $key == 'type' ? user_relationships_type_load($value) : user_load($value);
    }
  }

  //check requester is allowed to create this rtype
  if (!user_relationships_api_can_request($requester, $type)) {
    watchdog('user_relationships_api', 'User %user has no suitable roles to request a %rtype relationship', array('%user' => format_username($requester), '%rtype' => $type->name), WATCHDOG_WARNING);
    //it would be good to return a reason why save is denied, but it's the API portion, it does not expose anything user-visible
    return FALSE;
  }

  //check requestee is allowed to receive this rtype
  if (!user_relationships_api_can_receive($requestee, $type)) {
    watchdog('user_relationships_api', 'User %user has no suitable roles to receive a %rtype relationship', array('%user' => format_username($requestee), '%rtype' => $type->name), WATCHDOG_WARNING);
    //it would be good to return a reason why save is denied, but it's the API portion, it does not expose anything user-visible
    return FALSE;
  }

  // check whether this relationship aleady exists
  if (!$type->is_oneway || !$type->is_reciprocal) {
    $existing = user_relationships_load(array('between' => array($requester->uid, $requestee->uid), 'rtid' => $type->rtid), array('count' => TRUE));
  }
  else {
    $existing = user_relationships_load(array('requester_id' => $requester->uid, 'requestee_id' => $requestee->uid, 'rtid' => $type->rtid), array('count' => TRUE));
  }

  if (!$existing) {
    $relationship->requester_id = $requester->uid;
    $relationship->requestee_id = $requestee->uid;
    $relationship->approved = $approved || !$type->requires_approval || (isset($requestee->data['user_relationships_ui_auto_approve']) && isset($requestee->data['user_relationships_ui_auto_approve'][$type->rtid]));
    $relationship->rtid = $type->rtid;

    return user_relationships_save_relationship($relationship);
  }
}


/**
 * Create or update a user relationship.
 *
 * @param $relationship
 *   Object of the current relationship.
 * @param $action
 *   The reason for the update (request, approve, update).
 *
 * @return
 *   Object of the updated relationship or FALSE if the relationship wasn't
 *   able to save
 */
function user_relationships_save_relationship($relationship, $action = 'request') {
  //set basic info if it doesn't already exist
  !isset($relationship->flags) ? $relationship->flags = UR_OK : NULL;
  !isset($relationship->created_at) ? $relationship->created_at = REQUEST_TIME : NULL;
  $relationship->updated_at = REQUEST_TIME;

  if ($action == 'approve') {
    $relationship->approved = 1;
  }

  // Start a transaction so that we can roll back in case of an error.
  $transaction = db_transaction();
  try {
    module_invoke_all('user_relationships_presave', $relationship);

    if (isset($relationship->rid)) {
      // Delete possible requests coming the other direction
      $relationship_type = user_relationships_type_load($relationship->rtid);
      if (!$relationship_type->is_oneway) {
        db_delete('user_relationships')
          ->condition('rtid', $relationship->rtid)
          ->condition('requester_id', $relationship->requestee_id)
          ->condition('requestee_id', $relationship->requester_id)
          ->execute();
      }
    }

    //#454680 make sure that an update is performed if this is an existing relationship
    if (drupal_write_record('user_relationships', $relationship, (isset($relationship->rid) ? array('rid') : array()))) {
      // if the relationship has been approved and is two-way we need
      // to do a double entry for DB efficiency
      $relationship_type = user_relationships_type_load($relationship->rtid);
      if ($relationship->approved && !$relationship_type->is_oneway) {
        //#365623 drupal_write_record will not work because rid is a serial field, and we need to use the same rid
        // @todo: Verify that this doesn't cause problems when an existing
        // relationship is updated again after being approved.
        $select = db_select('user_relationships', 'ur')
          ->fields('ur', array('rid', 'requester_id', 'requestee_id', 'rtid', 'approved', 'created_at', 'updated_at', 'flags'))
          ->condition('rid', $relationship->rid);
        db_insert('user_relationships')
          ->fields(array('rid', 'requestee_id', 'requester_id', 'rtid', 'approved', 'created_at', 'updated_at', 'flags'))
          ->from($select)
          ->execute();
      }
    }

    // Don't execute a different hook for update/insert since that is not
    // relevant. The $action is important if someone needs to differentiate.
    module_invoke_all('user_relationships_save', $relationship, $action);
    return $relationship;
  }
  catch (Exception $e) {
    $transaction->rollback();
    watchdog_exception('user_relationships', $e);
    return FALSE;
  }
}


/**
 * Public API for deleting a relationship.
 *
 * @param $relationship
 *    object of the relationship
 * @param $deleted_by
 *    object of the user that initiated the delete command
 * @param $action
 *    string reason for removal ('cancel','disapprove','remove')
 */
function user_relationships_delete_relationship($relationship, $deleted_by, $action = 'remove') {
  $relationship->deleted_by = $deleted_by;
  db_delete('user_relationships')
    ->condition('rid', $relationship->rid)
    ->execute();

  module_invoke_all('user_relationships_delete', $relationship, $action);
}


/**
  * Load relationship objects from the database.
  *
  * @param $param
  *   an array of parameters with the key being the column. columns from both the user_relationships and user_relationship_types tables will work
  *     columns from user_relationships: rid, requester_id, requestee_id, rtid, approved, created_at, updated_at, flags
  *     columns from user_relationship_types: name, plural_name, is_oneway, requires_approval, expires_val
  *   There are two special keys:
  *     1) array("between" => array($uid1, $uid2)) will return all relationships between the two user ids.
  *     2) array("user" => $uid) will return all relationships for the specified uid
  *
  *   arguments will process operators as well using the syntax: array(col => '> {value}').
  *     example: show all relationships created in 2007
  *       $start_time = mktime(0,0,0,0,0,2007);
  *       $end_time = mktime(0,0,0,0,0,2008);
  *       user_relationships_load(array('created_at' => ">= {$start_time}", 'created_at' => '< {$end_time'}));
  *
  *   each parameter may be an array, if you wish to pass several values
  *     example: user_relationships_load(array('rtid' => array(2, 3), 'between' => array($uid1, $uid2)))
  *       will load all relationships of types 2 or 3 between uid1 and uid2 (query will have an IN clause)
  *
  * @param @options
  *   An array keyed by the option
  *   count
  *     a boolean stating whether or not the return value should be the number of relationships found
  *
  *   sort
  *     a string containing a valid column name which will become the key for the returned array of relationships
  *     default is 'rid'
  *
  *   order
  *     a string containing SQL stating the column and direction of the sort (ex. "requester_id ASC, rtid DESC")
  *
  *   limit
  *     a string containing SQL stating the limit (ex "10" or "10, 5")
  *
  *   include_user_info
  *     a boolean that will load basic user info without having to call user_load
  *     columns: uid, name, mail, data, picture
  *
  *   include_twoway_reverse (not used unless there is a specific need)
  *     a boolean that, if present, and if sort is set to other than 'rid', will include records for both directions for
  *     two-way relationships. Normally for a two-way relationship only one entry is returned, although in the database there
  *     are two records. This flag has no effect if sort is 'rid'
  *
  * @param $reset
  *   a boolean that will reset the internal static $relationships variable to ensure programatic relationship insertion works
  *
  * @return
  *   an array of relationships
  *   if the key is "rid" the array will be a single dimention: array($rid => $relationship, $rid => $relationship)
  *   otherwise it'll be multidimentional: array($rtid => array($relationship, $relationship))
  *
  *   each relationship will have the user's name, mail, and data attached as requester_name, requester_mail, requester_data
  *   or requestee_name, requestee_mail, requestee_data
 */
function user_relationships_load($param = array(), $options = array(), $reset = FALSE) {
  static $relationships = array();

  $default_options = array(
    'sort' => 'rid',
  );
  $options = array_merge($default_options, $options);
  extract($options, EXTR_SKIP);

  $arguments = array();
  if (is_numeric($param)) {
    if (!$reset && isset($relationships[$param])) {
      return is_object($relationships[$param]) ? clone $relationships[$param] : $relationships[$param];
    }
    $rid = $param;
    $param = array('rid' => $param);
  }

  $options['only_count'] = !empty($count);

  $query = _user_relationships_generate_query($param, $options);

  if (!empty($count)) {
    return (int)$query->execute()->fetchField();
  }

  $relationships = array();
  foreach ($query->execute() as $relationship) {
    if (isset($include_user_info)) {
      user_relationships_api_translate_user_info($relationship);
    }

    if ($sort == 'rid') {
      $relationships[$relationship->$sort] = $relationship;
    }
    else {
      $relationships[$relationship->$sort][] = $relationship;
    }
  }

  if (isset($rid) && !isset($relationships[$rid])) {
    return FALSE;
  }

  // Always call the hook with an array, even though there is only single entry.
  if (isset($rid)) {
    $return = $relationships[$rid];
    module_invoke_all('user_relationships_load', array($rid => $return));
  }
  else {
    $return = $relationships;
    // Only execute the hook if the array is not empty.
    if (!empty($return)) {
      module_invoke_all('user_relationships_load', $return);
    }
  }
  return $return;
}


/**
 * Used when the "include_user_info" option is set on user_relationships_load
 * to translate the retrieved user fields into actual user objects. This allows us
 * to pull the basic user data without having to run user_load
 *
 * @param $relationship
 *    The relationship object with pulled user info
 */
function user_relationships_api_translate_user_info(&$relationship) {
  if ($relationship->requester_name) {
    foreach (array('requester', 'requestee') as $user_type) {
      $relationship->{$user_type} = new stdClass();
      foreach (array('name', 'mail', 'data', 'picture') as $field) {
        $db_field = "{$user_type}_{$field}";
        $relationship->{$user_type}->{$field} = $relationship->{$db_field};

        unset($relationship->{$db_field});
      }
      $user_type_id = "{$user_type}_id";
      $relationship->{$user_type}->uid = $relationship->{$user_type_id};
      $relationship->{$user_type}->data = unserialize($relationship->{$user_type}->data);
    }
  }
}

/**
 * Check whether a user is allowed to request a certain relationship type
 *
 * @param $requester requesting user object or uid
 * @param $relationship_type loaded relationship type object or rtid
 * @return TRUE iff requester is allowed to request this type of relationship
 */
function user_relationships_api_can_request($requester, $relationship_type) {
  //validate arguments
  if (is_numeric($requester)) {
    $requester = user_load($requester);
  }
  if (is_numeric($relationship_type)) {
    $relationship_type = user_relationships_type_load($relationship_type);
  }
  if (!is_object($requester) || !is_object($relationship_type)) {
    return FALSE;
  }
  if ($requester->uid == 1) {
    return TRUE;
  }
  //if no roles have been set, any user can request
  if (!isset($relationship_type->roles) || !count($relationship_type->roles)) {
    return TRUE;
  }
  //check user has at least one required role
  foreach ($relationship_type->roles as $role_id) {
    if (isset($requester->roles[$role_id])) {
      return TRUE;
    }
  }
  return FALSE;
}

/**
 * Check whether a user is allowed to receive a certain relationship type
 *
 * @param $requestee requesting user object or uid
 * @param $relationship_type loaded relationship type object or rtid
 * @return TRUE iff receiver is allowed to request this type of relationship
 */
function user_relationships_api_can_receive($requestee, $relationship_type) {
  //validate arguments
  if (is_numeric($relationship_type)) {
    $relationship_type = user_relationships_type_load($relationship_type);
  }
  if (is_numeric($requestee)) {
    $requestee = user_load($requestee);
  }
  if (!is_object($requestee) || !is_object($relationship_type)) {
    return FALSE;
  }
  //if no roles have been set to receive the relationship, any user can receive it
  if (!isset($relationship_type->roles_receive) || !count($relationship_type->roles_receive)) {
    return TRUE;
  }
  //check user has at least one required role
  foreach ($relationship_type->roles_receive as $role_id) {
    if (isset($requestee->roles[$role_id])) {
      return TRUE;
    }
  }
  return FALSE;
}