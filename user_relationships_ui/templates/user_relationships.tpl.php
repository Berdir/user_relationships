<?php
// $Id$

//$relationships array is loaded in template_preprocess_user_relationships()
if ($relationships) {
  foreach ($relationships as $relationship) {
    $edit_access = ($user->uid == $account->uid && user_access('maintain own relationships')) || user_access('administer user relationships');

    $this_user_str  = $account->uid == $relationship->requestee_id ? 'requester' : 'requestee';
    $this_user      = $relationship->{$this_user_str};

    $row = array(
      theme('username', array('account' => $this_user)),
      ur_tt("user_relationships:rtid:$relationship->rtid:name", $relationship->name) . ($relationship->is_oneway ? ($this_user_str == 'requestee' ? t(' (You to Them)') : t(' (Them to You)')) : NULL),
      !empty($relationship->extra_for_display) ? $relationship->extra_for_display : '',
      $edit_access ? theme('user_relationships_remove_link', array('uid' => $account->uid, 'rid' => $relationship->rid)) : '&nbsp;',
    );
    if (variable_get('user_relationships_show_user_pictures', 0)) {
      array_unshift($row, theme('user_picture', array('account' => $this_user)));
    }
    $rows[] = $row;
  }

  print theme('table', array('rows' =>  $rows, 'attributes' => array('class' => array('user-relationships-listing-table'))));
  print theme('pager');
}
else {
  if (!empty($rtid)) {
    print t('You do not have any %plural_name.', array('%plural_name' => ur_tt("user_relationships:rtid:$relationship_type->rtid:plural_name", $relationship_type->plural_name)));
  }
  else {
    print t('You do not have any relationships with other users.');
  }
}
?>
