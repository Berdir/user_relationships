
User Relationships API
------------------
This the API only portion of User Relationships. This is required by all UR plugins and addon
modules. It provides no frontend interface.

Developers
------------
There are a number of API functions and corresponding hooks. The hooks are documented in user_relationships_api.api.php.
The API functions are all defined in user_relationships_api.api.inc. I've provded a list below for quick lookup, but you'll
need to see the documentation in that file for a deeper explanaition.

  Functions
  =========
  user_relationships_type_load($param = array())
  user_relationships_types_load($reset = NULL)
  user_relationships_type_save($rtype)
  user_relationships_type_delete($rtid)

  user_relationships_load($param = array(), $options = array(), $reset = FALSE)
  user_relationships_api_translate_user_info($relationship)
  user_relationships_request_relationship($requester, $requestee, $type, $approved = FALSE)
  user_relationships_save_relationship($relationship, $op = 'request')
  user_relationships_delete_relationship($relationship, &$deleted_by, $op = 'remove')

