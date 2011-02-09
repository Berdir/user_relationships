<?php
// $Id$

/**
 * @file
 * This file documents all hooks for user_relationships_api.module
 */

/**
 * Act when relationship types are loaded.
 *
 * @param $relationship_types_list
 *   A list of relationship types, keyed by rtid.
 *
 * @see user_relationhips_type_load_multiple()
 */
function hook_user_relationships_type_load($relationship_types_list) {

}

/**
 * This hook is executed before an existing or new relationship type is saved.
 *
 * @param $relationship_type
 *   Relationship type object.
 *
 * @see user_relationships_type_save()
 */
function hook_user_relationships_type_presave($relationship_type) {

}

/**
 * This hook is executed after an relationship type has been updated.
 *
 * @param $relationship_type
 *   Relationship type object.
 *
 * @see user_relationships_type_save()
 */
function hook_user_relationships_type_update($relationship_type) {

}

/**
 * This hook is executed after a new relationship type has been inserted.
 *
 * @param $relationship_type
 *   Relationship type object.
 *
 * @see user_relationships_type_save()
 */
function hook_user_relationships_type_insert($relationship_type) {

}

/**
 * This hook is executed after a relationship type has been deleted.
 *
 * @param $relationship_type
 *   Relationship type object.
 *
 * @see user_relationships_type_delete()
 */
function hook_user_relationships_type_delete($relationship_type) {

}

/**
 * Act when relationships are loaded.
 *
 * @param $relationship_list
 *   Array of relations, keyed by rid.
 *
 * @see user_relationships_load()
 */
function hook_user_relationships_load($relationship_list) {

}

/**
 * This hook is executed before a relationship will be saved.
 *
 * @param $relationship
 *   Relationship object.
 *
 * @see user_relationships_save_relationship()
 */
function hook_user_relationships_presave($relationship) {

}

/**
 * This hook is executed after a relationship has been updated.
 *
 * @param $relationship
 *   Relationship object.
 * @param $action
 *   The reason for the update (request, approve, update).
 *
 * @see user_relationships_save_relationship()
 */
function hook_user_relationships_save($relationship, $action) {

}

/**
 * This hook is executed after a new relationship has been saved.
 *
 * @param $relationship
 *   Relationship object.
 *
 * @see user_relationships_save_relationship()
 */
function hook_user_relationships_insert($relationship) {

}

/**
 * This hook is executed before a relationship will be saved.
 *
 * @param $relationship
 *   Relationship object.
 * @param $action
 *   String reason for removal ('cancel','disapprove','remove').
 *
 * @see user_relationships_delete_relationship()
 * 
 */
function hook_user_relationships_delete($relationship, $action) {

}