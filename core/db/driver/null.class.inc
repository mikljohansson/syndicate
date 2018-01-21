<?php

/**
 * @package		synd.core.db
 * @filesource
 */
require_once 'core/db/Database.class.inc';

/**
 * Null database, all methods do nothing.
 *
 * Used when the regular database is offline and the site is running
 * off cached content or similar situations.
 *
 * @access		public
 * @package		synd.core.db
 */
class synd_database_null implements Database {
	static function factory($driver, $dsn) {
		return new $driver();
	}
	
	function getDSN() {
		return 'null://null';
	}
	
	function close() {}

	function isNull() {
		return true;
	}

	function getNamespace() {
		return null;
	}
	
	function inTransaction()						{return false;}
	function begin($savepoint = null)				{}
	function commit()								{}
	function rollback($savepoint = null)			{}

	function createQuery() {
		return null;
	}

	function query($sql, $limit = null, $offset = 0, $prefetch = 50) {
		return new synd_result_null();
	}

	function execute($query, $limit = null, $offset = 0, $prefetch = 50) {
		return new synd_result_null();
	}
	
	function getAll($sql, $offset = 0, $limit = null, $prefetch = 50, $flags = 0)			{return array();}
	function getCol($sql, $col = 0, $offset = 0, $limit = null, $prefetch = 50, $flags = 0)	{return array();}
	function getAssoc($sql, $offset = 0, $limit = null, $prefetch = 50)						{return array();}
	function getRow($sql)	{return array();}
	function getOne($sql)	{return null;}
	
	function replace($table, $data)			{}
	function update($table, $data)			{}
	function insert($table, $data)			{}
	function load($table, $rows)			{}

	function delete($table, $data)			{}
	function truncate($table)				{}
	function deleteQuick($table, $where)	{}
	
	function analyze($table) {}
	function optimize($table) {}
	function order($table, $order) {}
	
	function affectedRows()	{return 0;}
	function foundRows() {return null;}
	
	function nextId($name) {
		return 0;
	}
	
	function isAlive()	{
		return true;
	}

	function getTables() {
		return array();
	}

	function getSequences() {
		return array();
	}

	function getTableKeys($table) {
		return array();
	}

	function getTableStructure($table)	{
		require_once 'core/db/TableMetadata.class.inc';
		return new TableMetadata($table);
	}
	
	function quote($value) {
		return $value;
	}

	function wrapBinary($value) {
		return $value;
	}

	function unwrapBinary($value) {
		return $value;
	}

	function ternary($if, $then, $else)	{return $this->quote('');}
	function concat($vals)				{return $this->quote('');}
	function bitand($a, $b)				{return $this->quote('');}
	function unixtodate($expr)			{return $this->quote('');}
	function date($expr)				{return $this->quote('');}

	function limit($sql, $offset, $limit) {
		return $sql;
	}
	
	function rand() {
		return 0;
	}
}

/**
 * Null result
 */
class synd_result_null extends AbstractResultSet {
	function fetchRow()	{
		return null;
	}
	
	function fetchInto(&$row) {
		return $row = null;
	}
}

