<?php
set_include_path(get_include_path().':'.dirname(dirname(dirname(dirname(__FILE__)))).'/local:'.dirname(dirname(dirname(__FILE__))));
require_once 'synd.inc';

/**
 * Database dump script.
 *
 * Handy when moving data between databases of different make.
 *
 * @access		public
 * @package		synd.core
 */

if (count($argv) < 2) {
	print "Usage: php dump.inc [OPTIONS] SOURCE_DSN [TARGET_DSN]\n";
	print "Dumps database to INSERT statements\n";
	print "\n";
	print "Arguments\n";
	print "  --match         case insensitive tables pattern to dump, see fnmatch()\n";
	print "  --lower         lowercase table names\n";
	print "  --single-row    dump one row from each table (useful for testing)\n";
	print "  --no-data       no row information\n";
	print "  --mysql         write output formatted for MySQL\n";
	print "  --replace-into  use MySQL REPLACE INTO statements\n";
	print "  --sequences     dump sequences\n";
	print "\n";
	print "If TARGET_DSN is specified that database driver will be used when quoting values\n";
	print "\n";
	print "Example.\n";
	print "  php dump.inc --mysql --match synd_\* mysql://user:password/localhost/database\n";
	exit;
}

/**
 * @access	private
 */
function _score_topo(&$topo, &$relations, $table, $visited = array()) {
	if (!isset($topo[$table]))
		$topo[$table] = 0;
	if (isset($relations[$table])) {
		foreach (array_unique($relations[$table]) as $target) {
			if (!isset($visited["$table.$target"])) {
				if (!isset($topo[$target]))
					$topo[$target] = 0;
				
				$topo[$target] += $topo[$table] + 1;
				$visited["$table.$target"] = true;
				
				_score_topo($topo, $relations, $target, $visited);
			}
		}
	}
}

/**
 * @access	private
 */
function _parse_args($spec, $args) {
	$result = array();
	
	foreach ($spec as $name => $info) {
		if (false === ($i = array_search($name, $args))) 
			$result[$name] = $info[1];
		else if ($info[0]) {
			if (isset($args[$i+1])) {
				$result[$name] = $args[$i+1];
				unset($args[$i], $args[$i+1]);
			}
			else {
				$result[$name] = $info[1];
				unset($args[$i]);
			}
		}
		else {
			$result[$name] = true;
			unset($args[$i]);
		}
	}
	
	return array_merge($result, $args);
}

set_time_limit(0);

ini_set('display_startup_errors', true);
ini_set('display_errors', true);
error_reporting(E_ALL);
ob_implicit_flush(1);

$args = _parse_args(array(
	'--match'			=> array(true,null),
	'--lower'			=> array(false,false),
	'--single-row'		=> array(false,false),
	'--no-data'			=> array(false,false),
	'--sequences'		=> array(false,false),
	'--mysql'			=> array(false,false),
	'--replace-into'	=> array(false,false),
	), $argv);

require_once 'core/db/Database.class.inc';
$source = DatabaseManager::getConnection($args[1]);
$tables = $source->getTables();

if (!empty($args['--mysql'])) {
	print "/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;\n";
	print "/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;\n";
	print "/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;\n";
	print "/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;\n";
	print "\n";
}

if (empty($args['--no-data'])) {
	$relations = array();
	if (!empty($args[2]))
		$target = DatabaseManager::getConnection($args[2]);
	else
		$target = $source;

	// Sort the tables topologically
	foreach ($tables as $table) {
		if (empty($args['--match']) || fnmatch($args['--match'], $table, FNM_CASEFOLD)) {
			$structure = $source->getTableStructure($table);
			$relations[$table] = array();
			foreach ($structure->getRelations() as $relation)
				$relations[$table][] = $relation[1]->getTable();
		}
	}

	$topo = array();
	foreach (array_keys($relations) as $table)
		_score_topo($topo, $relations, $table);
	arsort($topo);

	// Dump data to INSERT statements
	foreach (array_keys($topo) as $table) {
		$sql = "SELECT * FROM $table";
		$query = $source->query($sql);

		if (!empty($args['--lower']))
			$table = strtolower($table);

		while (null != ($row = $query->fetchRow())) {
			print empty($args['--replace-into']) ? 'INSERT' : 'REPLACE';
			print " INTO $table (".implode(',',array_keys($row)).") VALUES (";
			print implode(',', $target->quote($row));
			print ");\n";

			if (!empty($args['--single-row']))
				break;
		}
	}

	print "\n";
}

if (!empty($args['--sequences']) && !empty($args['--mysql'])) {
	foreach ($source->getSequences() as $sequence => $lastid) {
		if (!isset($args['--match']) || fnmatch($args['--match'], $sequence, FNM_CASEFOLD)) {
			print "CREATE TABLE IF NOT EXISTS ".strtolower($sequence)." (\n";
			print "	id bigint(20) unsigned NOT NULL auto_increment,\n";
			print "	PRIMARY KEY (id)\n";
			print ") ENGINE=InnoDB DEFAULT CHARSET=latin1;\n";
			print "INSERT INTO ".strtolower($sequence)." VALUES ($lastid);\n";
			print "DELETE FROM ".strtolower($sequence)." WHERE id = $lastid;\n";
			print "\n";
		}
	}
}

if (!empty($args['--mysql'])) {
	print "/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;\n";
	print "/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;\n";
	print "/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;\n";
	print "/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;\n";
}
