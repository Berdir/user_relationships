<?php
// $Id$
/**
 * @file
 * Page to manage sent and received relationship requests
 */

  $output = '';
  $pager_id = 0;
  $section_headings = array(
    'sent_requests'     => t('Sent Requests'),
    'received_requests' => t('Received Requests')
  );

  $edit_access = ($user->uid == $account->uid && user_access('maintain own relationships')) || user_access('administer user relationships');

  $i = 0;
  foreach ($sections as $column => $section) {
    if (empty($$section)) { continue; }
    $rows = array();

    $header = array(array('data' => $section_headings[$section], 'colspan' => 2));

    foreach ($$section as $relationship) {
      $links = array();
      if ($edit_access) {
        if ($section == 'sent_requests') {
          $links[] = theme('user_relationships_pending_request_cancel_link', array('uid' => $account->uid, 'rid' => $relationship->rid));
        }
        else {
          $links[] = theme('user_relationships_pending_request_approve_link', array('uid' => $account->uid, 'rid' => $relationship->rid));
          $links[] = theme('user_relationships_pending_request_disapprove_link', array('uid' => $account->uid, 'rid' => $relationship->rid));
        }
      }
      $links = implode(' | ', $links);

      if ($relationship->requester_id == $account->uid) {
        $rows[]   = array(t('@rel_name to !requestee', array('@rel_name' => ur_tt("user_relationships:rtid:$relationship->rtid:name", $relationship->name), '!requestee' => theme('username', array('account' => $relationship->requestee)))), $links);
      }
      else {
        $rows[]   = array(t('@rel_name from !requester', array('@rel_name' => ur_tt("user_relationships:rtid:$relationship->rtid:name", $relationship->name), '!requester' => theme('username', array('account' => $relationship->requester)))), $links);
      }
    }

    $output .= theme('table', array('rows' => $rows, 'header' => $header, 'attributes' => array('class' => array('user-relationships-pending-listing-table'))));
    $output .= theme('pager', array('element' => $i++));
  }

  if (empty($output)) {
    $output = t('You do not have any relationship requests currently pending.');
  }

  print $output;
?>
