<?php
require_once 'core/model/DatabaseEntity.class.inc';

class synd_node_poll extends AbstractDatabaseEntity {
	function __factory() {
		global $synd_user;
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_poll');
		$this->data['CREATE_NODE_ID'] = $synd_user->nodeId;
		$this->data['TS_CREATE'] = time();
		parent::__factory();
	}

	function __clone() {
		global $synd_user;
		$this->data['NODE_ID'] = $this->classId().'.'.$this->_db->nextId('synd_poll');
		$this->data['CREATE_NODE_ID'] = $synd_user->nodeId;
		$this->data['TS_CREATE'] = time();
		parent::__clone();
	}

	static function getEntityQuery(DomainStorage $storage) {
		return self::createQuery($storage, 'synd_poll');
	}

	function nodeId() {
		return $this->data['NODE_ID'];
	}

	function getHandler() {
		return 'poll';
	}
	
	function getChildren() {
		$page = $this->getPage();
		return $page->getChildren();
	}

	function getQuestions() {
		$page = $this->getPage();
		return $page->getQuestions();
	}
	
	function hasPermission($user, $perm, $arg = null) {
		if ('read' == $perm && in_array($arg, array('full_view_confirm.tpl')))
			return $this->hasPermission($user, 'read');
		if ('read' == $perm && in_array($arg, 
			array('full_view_attempts.tpl','full_view_replies.tpl','full_view_emails.tpl')))
			return $this->hasPermission($user, 'statistics');
		
		if ('read' == $perm && null !== $arg)
			return false;
		
		if (parent::hasPermission($user, $perm, $arg))
			return true;

		$creator = $this->getCreator();
		if ($creator->hasAuthorization($user, $this))
			return true;
		if ('statistics' == $perm && $this->hasPermission($user, 'write'))
			return true;
		if ('read' == $perm && $this->hasPermission($user, 'statistics'))
			return true;
			
		if ('merge' == $perm && in_array($arg, array(
			'PARENT_NODE_ID','TS_START','TS_STOP','FLAG_PROMOTE','FLAG_ANONYMOUS','INFO_REDIRECT')))
			return $this->hasPermission($user, 'write');

		return false;
	}
	
	function getDefinedPermissions() {
		return array(
			'read' => SyndLib::translate('Allows users to view and reply to this poll'),
			'statistics' => SyndLib::translate('Read access to poll and collected answers and statistics'),
			'write' => SyndLib::translate('Read and write access for changing questions and text'),
			'admin' => SyndLib::translate('Poll administration allows for chaning permission and other options'),
			);
	}
	
	function getCreator() {
		if (null === ($creator = SyndNodeLib::getInstance($this->data['CREATE_NODE_ID'])))
			$creator = SyndNodeLib::getInstance('user_null.null');
		return $creator;
	}
	
	function toString() {
		$page = $this->getPage();
		return $page->toString();
	}

	function validate($data) {
		require_once 'core/lib/SyndDate.class.inc';
		$errors = array();

		if (!empty($data['TS_START'])) {
			if (!SyndDate::checkdate($data['TS_START']))
				$errors['TS_START'] = SyndLib::translate("Invalid date given ('%s')", tpl_chop($data['TS_START'],12));
			unset($data['TS_START']);
		}

		if (!empty($data['TS_STOP'])) {
			if (!SyndDate::checkdate($data['TS_STOP']))
				$errors['TS_STOP'] = SyndLib::translate("Invalid date given ('%s')", tpl_chop($data['TS_STOP'],12));
			unset($data['TS_STOP']);
		}

		return array_merge(parent::validate($data), $errors);
	}
	
	function merge($data) {
		$page = $this->getPage();
		$page->merge($data);
		
		if (isset($data['TS_START']))
			$data['FLAG_PROMOTE'] = empty($data['FLAG_PROMOTE']) ? 0 : 1;
		if (!empty($data['TS_START']))
			$data['TS_START'] = strtotime($data['TS_START']);
		if (!empty($data['TS_STOP']))
			$data['TS_STOP'] = strtotime($data['TS_STOP']);

		if (isset($data['confirm'])) {
			$confirm = $this->getConfirmation();
			$confirm->merge($data['confirm']);
			unset($data['confirm']);
		}

		if (isset($data['error'])) {
			$error = $this->getErrorDocument();
			$error->merge($data['error']);
			unset($data['error']);
		}
		
		parent::merge($data);
	}
	
	function delete() {
		SyndLib::invoke($this->getChildren(), 'delete');
		parent::delete();
	}

	/**
	 * Attempts to resolve an object id into a node id
	 * @param	integer	Poll object id
	 * @return	string
	 */
	function resolve($id) {
		$database = SyndNodeLib::getDefaultStorage('poll')->getDatabase();
		$sql = "
			SELECT p.node_id FROM synd_poll p
			WHERE p.node_id LIKE ".$database->quote("%.$id");
		return $database->getOne($sql);
	}

	function getPage() {
		if (null == ($page = SyndNodeLib::getInstance($this->data['PAGE_NODE_ID']))) {
			$page = SyndNodeLib::factory('questionnaire');
			$page->save();
			$this->appendChild($page);
			$this->data['PAGE_NODE_ID'] = $page->nodeId;
		}
		return $page;
	}

	function getBranch() {
		return array();
	}

	function getChapters() {
		$page = $this->getPage();
		return array($page->nodeId => $page);
	}

	function getNext() {
		$result = null;
		return $result;
	}
	
	function getConfirmation() {
		if (null == ($confirm = SyndNodeLib::getInstance($this->data['CONFIRM_NODE_ID']))) {
			$confirm = SyndNodeLib::factory('page');
			$confirm->save();

			$this->data['CONFIRM_NODE_ID'] = $confirm->nodeId;
			$this->appendChild($confirm);
			$this->_save();
		}
		return $confirm;
	}
	
	function getErrorDocument() {
		if (null == ($error = SyndNodeLib::getInstance($this->data['ERROR_NODE_ID']))) {
			$error = SyndNodeLib::factory('page');
			$error->save();

			$this->data['ERROR_NODE_ID'] = $error->nodeId;
			$this->appendChild($error);
			$this->_save();
		}
		return $error;
	}

	function getCompositeData() {
		$page = $this->getPage();
		$data = $page->getCompositeData();

		$confirm = $this->getConfirmation();
		$data['confirm'] = $confirm->getCompositeData();
		
		$error = $this->getErrorDocument();
		$data['error'] = $error->getCompositeData();

		return array_merge($data, parent::getCompositeData());
	}

	function getTitle() {
		$page = $this->getPage();
		return $page->toString();
	}
	
	function getDescription() {
		$page = $this->getPage();
		return $page->getDescription();
	}

	/**
	 * @deprecated	Use getContent()
	 */
	function getBody() {
		return $content = $this->getContent();
	}
	
	function getContent() {
		$page = $this->getPage();
		return $content = $page->getContent();
	}
	
	function save() {
		parent::save();
		
		$page = $this->getPage();
		$page->save();

		$confirm = $this->getConfirmation();
		$confirm->save();

		$error = $this->getErrorDocument();
		$error->_save();
	}
	
	function _save() {
		parent::_save();

		$page = $this->getPage();
		$page->_save();

		$confirm = $this->getConfirmation();
		$confirm->_save();

		$error = $this->getErrorDocument();
		$error->_save();
	}

	function createAttempt() {
		$page = $this->getPage();
		$attempt = $page->createAttempt();
		if ($this->isAnonymous())
			$attempt->setAnonymous();
		return $attempt;
	}
	
	function createTrainingAttempt() {
		SyndNodeLib::loadClass('attempt');
		return new SyndTrainingAttempt();
	}
	
	function isAnonymous() {
		return !empty($this->data['FLAG_ANONYMOUS']);
	}
	
	function getAttempts() {
		$page = $this->getPage();
		return $page->getAttempts();
	}

	function getAttemptCount() {
		$page = $this->getPage();
		return $page->getAttemptCount();
	}

	function _callback_breadcrumbs(&$result, Template $page) {
		$result[] = array(
			'uri' => tpl_link($this->getHandler()),
			'text' => SyndLib::translate('Home'));
		$result[] = array(
			'uri' => tpl_link($this->getHandler(),'view',$this->nodeId),
			'text' => $this->toString());
	}
	
	function getFilteredCount() {
		$filters = $this->getStatisticsFilters();
		if (empty($filters))
			return $this->getAttemptCount();
			
		$sql = "
			SELECT COUNT(*) FROM synd_attempt a
			WHERE 1=1";
		foreach ($filters as $filter)
			$sql .= " AND a.node_id IN ($filter)";
		
		return $this->_db->getOne($sql);
	}
	
	function addStatisticsFilter($question, $option) {
		$_SESSION['synd']['filter'][$this->nodeId][$question->nodeId][$option] = $option;
		ksort($_SESSION['synd']['filter'][$this->nodeId][$question->nodeId]);
		ksort($_SESSION['synd']['filter'][$this->nodeId]);
	}
	
	function delStatisticsFilter($question, $option) {
		unset($_SESSION['synd']['filter'][$this->nodeId][$question->nodeId][$option]);
		if (empty($_SESSION['synd']['filter'][$this->nodeId][$question->nodeId]))
			unset($_SESSION['synd']['filter'][$this->nodeId][$question->nodeId]);
	}
	
	function getStatisticsFilters() {
		if (!isset($_SESSION['synd']['filter'][$this->nodeId]))
			return array();
		
		$filters = array();
		foreach ($_SESSION['synd']['filter'][$this->nodeId] as $id => $options) {
			if (null != ($question = SyndNodeLib::getInstance($id)))
				$filters[] = $question->getAttemptFilter($options);
		}
		
		return array_filter($filters);
	}
	
	function getFilterOptions() {
		if (isset($_SESSION['synd']['filter'][$this->nodeId]))
			return $_SESSION['synd']['filter'][$this->nodeId];
		return array();
	}
	
	function _view_submit(Request $request, Template $page) {
		return array('redirect' => tpl_uri_merge(null,tpl_view($this->getHandler(),'view',$this->nodeId,'confirm')));
	}
	
	function _callback_html_head_redirect(&$result) {
		if (null != $this->data['INFO_REDIRECT'])
			$result .= "\t<meta http-equiv=\"refresh\" content=\"3;url=".tpl_attribute($this->data['INFO_REDIRECT'])."\" />";
	}
	
	function _view_clearAttempts(Request $request, Template $page) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();
		
		if (isset($request['confirm']))
			SyndLib::invoke($this->getAttempts(),'delete');
		else {
			return array('content' => $page->fetchnode($this, 'confirm_clear_attempts.tpl'));
		}
	}
	
	function _entity_onpreflush() {
		global $synd_user;
		parent::_entity_onpreflush();
		$this->data['UPDATE_NODE_ID'] = $synd_user->nodeId;
		$this->data['TS_UPDATE'] = time();
	}
}
