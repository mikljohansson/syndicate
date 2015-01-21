<?php
require_once 'core/lib/auth/Role.class.inc';

/**
 * User interface for use with the framework
 */
interface User extends Role {
	/**
	 * Resolves a username into a user object
	 * @param	string	The username to attempt to resolve
	 * @return	User	Returns null if none found
	 */
	static function resolveLogin($username);
	
	/**
	 * Resolves an email address into a user object
	 * @param	string	The email to attempt to resolve
	 * @return	User	Returns null if none found
	 */
	static function resolveEmail($email);

	/**
	 * Check if a password is correct for this user
	 * @param	string	The password to check
	 * @return	bool
	 */
	function validatePassword($password);
	
	/**
	 * Returns the name and details of a user
	 * @return	string
	 */
	function __toString();
	
	/**
	 * Returns the username of this user
	 * @return	string
	 */
	function getLogin();
	
	/**
	 * Returns the phonenumber of this user
	 * @return	string
	 */
	function getPhone();
	
	/**
	 * Returns the email address of this user
	 * @return	string
	 */
	function getEmail();

	/**
	 * Returns contact details as a comma separated string, for example 'mikl@example.com, +4612345678'
	 * @return	string
	 */
	function getContact();
	
	/**
	 * Returns the address of this user
	 *
	 * array(
	 *  'CO'		=> 'No Name'
	 *  'STREET'	=> 'Nonamestreet 12'
	 *  'ZIP'		=> '12345'
	 *  'COUNTRY'	=> 'Sweden'
	 * );
	 *
	 * @return	array
	 */
	function getAddress();

	/**
	 * Returns the social security number of this user
	 * @return	string
	 */
	function getSocialSecurityNumber();
	
	/**
	 * Returns the primary organizational unit
	 * @return	Role
	 */
	function getOrganization();

	/**
	 * Returns roles and groups for this user
	 * @return	array
	 */
	function getRoles();
}
