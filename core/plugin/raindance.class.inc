<?php
require_once 'core/Plugin.class.inc';
require_once 'core/db/Database.class.inc';

/**
 * Raindance inventory plugin
 *
 */
class synd_plugin_raindance extends Plugin {
	/**
	 * @access	private
	 */
	var $_db = null;

	function __construct($config = null) {
		parent::__construct($config);
		SyndLib::attachHook('search_costcenter', array($this, '_callback_search_costcenter'));
		SyndLib::attachHook('search_project', array($this, '_callback_search_project'));
		SyndLib::attachHook('find_costcenter', array($this, '_callback_find_costcenter'));
		SyndLib::attachHook('find_project', array($this, '_callback_find_project'));
		SyndLib::attachHook('instance', array($this, '_callback_instance'), -500);
		SyndLib::attachHook('inventory_installation_summary', array($this, '_callback_inventory_installation_summary'));
	}
	
	function getDatabase() {
		if (null === $this->_db)
			$this->_db = DatabaseManager::getConnection($this->_config);
		return $this->_db;
	}
	
	function _callback_search_costcenter(&$result, $query) {
		$result[] = new RaindanceCostcenterCollection($this, $query);
	}

	function _callback_search_project(&$result, $query) {
		$result[] = new RaindanceProjectCollection($this, $query);
	}

	function _callback_find_costcenter(&$result, $id) {
		if (null !== ($result = SyndLib::getInstance("raindance.costcenter.$id")))
			return true;
	}

	function _callback_find_project(&$result, $id) {
		if (null !== ($result = SyndLib::getInstance("raindance.project.$id")))
			return true;
	}
	
	function _callback_instance(&$result, $id) {
		if (0 === strpos($id, 'raindance.costcenter.') && null !== ($result = $this->getCostcenter($id)))
			return true;
		else if (0 === strpos($id, 'raindance.project.') && null !== ($result = $this->getProject($id)))
			return true;
	}
	
	function getCostcenter($id) {
		if (null == ($result = SyndLib::runHook('cache_get', $id))) {
			$database = $this->getDatabase();
			$sql = "
				SELECT k.ID, k.TEXT FROM obj_kst k
				WHERE k.id = ".$database->quote(substr($id, 21));
			if (null == ($row = $database->getRow($sql))) 
				$result = null;
			else {
				$result = new RaindanceCostcenter($row['ID'], $row['TEXT']);
				SyndLib::runHook('cache_set', $id, $result);
			}
		}
		return $result;
	}
	
	function getProject($id) {
		if (null == ($result = SyndLib::runHook('cache_get', $id))) {
			$database = $this->getDatabase();
			$sql = "
				SELECT p.ID, p.TEXT FROM obj_proj p
				WHERE p.id = ".$database->quote(substr($id, 18));
			if (null == ($row = $database->getRow($sql))) 
				$result = null;
			else {
				$result = new RaindanceProject($row['ID'], $row['TEXT']);
				SyndLib::runHook('cache_set', $id, $result);
			}
		}
		return $result;
	}
	
	/**
	 * Displays installation info from Raindance
	 */
	function _callback_inventory_installation_summary(&$result, Template $page, $installation) {
		$template = clone $page;
		$database = $this->getDatabase();

		$sql = "
			SELECT a.TEXT, TO_CHAR(a.giltig_fom, 'YYYY-MM-DD') GILTIG_FOM, TO_CHAR(a.giltig_tom, 'YYYY-MM-DD') GILTIG_TOM
			FROM obj_anlid a
			WHERE
				a.id = ".$database->quote($installation->getInstallationNumber())." OR
				a.id LIKE ".$database->quote($installation->getInstallationNumber().' %');

		if (null == ($row = $database->getRow($sql))) 
			$template->assign('invalid', true);
		else {
			$template->assign('title', $row['TEXT']);
			$template->assign('validfrom', strtotime($row['GILTIG_FOM']));
			$template->assign('validto', strtotime($row['GILTIG_TOM']));
		}
		
		$result .= $template->fetch(tpl_design_path('plugin/raindance/inventory_installation_summary.tpl'));
	}
}

/**
 * @access	private
 */
class RaindanceCostcenterCollection {
	var $_plugin = null;
	var $_query = null;
	var $_count = null;
	
	function RaindanceCostcenterCollection($plugin, $query) {
		$this->_plugin = $plugin;
		$this->_query = $query;
	}
	
	function getContents($offset = 0, $limit = null) {
		$database = $this->_plugin->getDatabase();
		$sql = "
			SELECT k.id, k.text FROM obj_kst k
			WHERE 
				-- (k.giltig_fom IS NULL OR k.giltig_fom <= SYSDATE) AND 
				-- (k.giltig_tom IS NULL OR k.giltig_tom >= SYSDATE) AND
				(k.id LIKE ".$database->quote($this->_query.'%')." OR
				 LOWER(k.text) LIKE ".$database->quote(strtolower($this->_query).'%').")";
		$rows = $database->getAssoc($sql, $offset, $limit);
		
		if (SyndLib::isError($rows)) {
			trigger_error($rows->getMessage(), E_USER_WARNING);
			return array();
		}
		
		$result = array();
		foreach ($rows as $id => $name) {
			$costcenter = new RaindanceCostcenter($id, $name);
			$result[$costcenter->id()] = $costcenter;
		}
		
		return $result;
	}
	
	function getCount() {
		if (null === $this->_count) {
			$database = $this->_plugin->getDatabase();
			$sql = "
				SELECT COUNT(1) FROM obj_kst k
				WHERE 
					-- (k.giltig_fom IS NULL OR k.giltig_fom <= SYSDATE) AND 
					-- (k.giltig_tom IS NULL OR k.giltig_tom >= SYSDATE) AND
					(k.id LIKE ".$database->quote($this->_query.'%')." OR
					 LOWER(k.text) LIKE ".$database->quote(strtolower($this->_query).'%').")";
			$this->_count = (int)$database->getOne($sql);
		}
		return $this->_count;
	}
}

/**
 * @access	private
 */
class RaindanceCostcenter {
	var $_id = null;
	var $_name = null;
	
	function RaindanceCostcenter($id, $name) {
		$this->_id = $id;
		$this->_name = $name;
	}
	
	function id() {
		return 'raindance.costcenter.'.$this->_id;
	}
	
	function toString() {
		return $this->_name;
	}
	
	function getLogin() {
		return $this->_id;
	}
	
	function getContact() {
		return $this->_id;
	}
	
	function getAddress() {
		return array();
	}
}

/**
 * @access	private
 */
class RaindanceProjectCollection {
	var $_plugin = null;
	var $_query = null;
	var $_count = null;
	
	function RaindanceProjectCollection($plugin, $query) {
		$this->_plugin = $plugin;
		$this->_query = $query;
	}
	
	function getContents($offset = 0, $limit = null) {
		$database = $this->_plugin->getDatabase();
		$sql = "
			SELECT p.id, p.text FROM obj_proj p
			WHERE 
				-- (p.giltig_fom IS NULL OR p.giltig_fom <= SYSDATE) AND 
				-- (p.giltig_tom IS NULL OR p.giltig_tom >= SYSDATE) AND
				(p.id LIKE ".$database->quote($this->_query.'%')." OR
				 LOWER(p.text) LIKE ".$database->quote(strtolower($this->_query).'%').")";
		$rows = $database->getAssoc($sql, $offset, $limit);
		
		if (SyndLib::isError($rows)) {
			trigger_error($rows->getMessage(), E_USER_WARNING);
			return array();
		}
		
		$result = array();
		foreach ($rows as $id => $name) {
			$project = new RaindanceProject($id, $name);
			$result[$project->id()] = $project;
		}
		
		return $result;
	}
	
	function getCount() {
		if (null === $this->_count) {
			$database = $this->_plugin->getDatabase();
			$sql = "
				SELECT COUNT(1) FROM obj_proj p
				WHERE 
					-- (p.giltig_fom IS NULL OR p.giltig_fom <= SYSDATE) AND 
					-- (p.giltig_tom IS NULL OR p.giltig_tom >= SYSDATE) AND
					(p.id LIKE ".$database->quote($this->_query.'%')." OR
					 LOWER(p.text) LIKE ".$database->quote(strtolower($this->_query).'%').")";
			$this->_count = (int)$database->getOne($sql);
		}
		return $this->_count;
	}
}

/**
 * @access	private
 */
class RaindanceProject {
	var $_id = null;
	var $_name = null;
	
	function RaindanceProject($id, $name) {
		$this->_id = $id;
		$this->_name = $name;
	}
	
	function id() {
		return 'raindance.project.'.$this->_id;
	}
	
	function toString() {
		return $this->_name;
	}
	
	function getLogin() {
		return $this->_id;
	}
	
	function getContact() {
		return $this->_id;
	}
}