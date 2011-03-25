<?php
/**
 * @file
 * Various user relationships admin and user pages
 */

/**
 * Main list of relationships for a specified user
 */
function user_relationships_page($account = NULL, $rtid = NULL) {
  if (!$account || !is_object($account) || !$account->uid) {
    global $user;
    $account = $user;
  }

  // Check if this is a valid rtid.
  if (!empty($rtid) && !user_relationships_type_load($rtid)) {
    return MENU_NOT_FOUND;
  }

  return theme('user_relationships', array('account' => $account, 'rtid' =>  $rtid));
}

/**
 * List of pending requests from other users
 */
function user_relationships_pending_requests_page($account = NULL) {
  // Check that the uid is valid, not the anonymous user, and the user exists
  if (!$account) {
    global $user;
    $account = $user;
  }

  if (!is_object($account) || !$account->uid) {
    return MENU_NOT_FOUND;
  }

  return theme('user_relationships_pending_requests', array('account' => $account));
}
