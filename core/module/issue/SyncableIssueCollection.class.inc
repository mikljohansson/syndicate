<?php
require_once 'core/model/ISyncable.class.inc';

/**
 * Wraps a collection of issues
 *
 * Takes a normal Collection and allows the contained issues
 * to be synced against.
 *
 * @package		synd.core.module
 */
class SyncableIssueCollection extends ISyncableCollection {
	/**
	 * @access	protected
	 */
	var $_collection = null;

	/**
	 * @access	protected
	 */
	var $_project = null;

	/**
	 * @access	protected
	 */
	var $_user = null;
	
	function SyncableIssueCollection($collection, $project, $user) {
		$this->_collection = $collection->id();
		$this->_project = $project->id();
		$this->_user = $user->id();
	}
	
	/**
	 * @abstract
	 */
	function getName() {assert(false);}
	
	/**
	 * @abstract
	 * @access	protected
	 */
	function _newInstance($issue) {assert(false);}
	
	function getInstance($id) {
		if ((($issue = SyndLib::getInstance($id)) instanceof synd_node_issue))
			return $this->_newInstance($issue);
		return null;
	}
	
	function isNull() {
		return false;
	}
	
	function getCollection() {
		if (null != ($collection = SyndLib::getInstance($this->_collection)))
			return $collection;
		return SyndType::factory('null_collection');
	}
	
	function getProject() {
		if (null != ($project = SyndLib::getInstance($this->_project)))
			return $project;
		return SyndNodeLib::getInstance('null.null');
	}
	
	function getUser() {
		if (null != ($user = SyndLib::getInstance($this->_user)))
			return $user;
		return SyndNodeLib::getInstance('user_null.null');
	}

	function getContents($last = null, $next = null) {
		$collection = $this->getCollection();
		$contents = $collection->getFilteredContents(array('synd_node_issue'));
		$result = array();

		foreach (array_keys($contents) as $key) {
			if ((null == $last || $contents[$key]->data['TS_UPDATE'] >= $last-10) &&
				(null == $next || $contents[$key]->data['TS_UPDATE'] <= $next+10))
				$result[$contents[$key]->id()] = $this->_newInstance($contents[$key]);
		}
		
		return $result;
	}

	function append($memento) {
		if (!($memento instanceof VCalendar)) {
			$result = false;
			return $result;
		}
		
		$project = $this->getProject();
		$issue = $project->_storage->factory('issue');
		
		if ($project->hasPermission($this->getUser(), 'append', $issue))
			$issue->setParent($project);
		else {
			$result = false;
			return $result;
		}
			
		$issue->setCreator($this->getUser());
		$issue->setCustomer($this->getUser());
		$issue->setAssigned($this->getUser());
		
		$instance = $this->_newInstance($issue);
		if (false === $instance->setMemento($memento)) {
			$result = false;
			return $result;
		}

		$issue->save();
		$issue->flush();
		
		return $instance;
	}
	
	function remove($instance) {
		if (!$instance->hasPermission($this->getUser(), 'write'))
			return false;
		$instance->cancel();
		return $instance;
	}
}

/**
 * Communicates using VTODO entries
 */
class SyncableIssueTodoCollection extends SyncableIssueCollection {
	function SyncableIssueTodoCollection($collection, $project, $user) {
		parent::SyncableIssueCollection($collection, $project, $user);
	}
	
	function getName() {
		return 'tasks';
	}

	function _newInstance($issue) {
		return new SyncableTodoIssue($this, $issue);
	}

	function getCalendarCapabilities() {
		return array(
			'BEGIN'			=> array('VTODO'),
			'UID'			=> true,
			'URL'			=> true,
			'DTSTAMP'		=> true,
			'CREATED'		=> true,
			'LAST-MODIFIED'	=> true,
			'CLASS'			=> true,
			'STATUS'		=> true,
			'PRIORITY'		=> true,
			'DTSTART'		=> true,
			'DUE'			=> true,
			'SUMMARY'		=> true,
			'DESCRIPTION'	=> true,
			'CATEGORIES'	=> true,
			'AALARM'		=> true,
			'DALARM'		=> true,
			'END'			=> array('VTODO'),
			);
	}
}

/**
 * Communicates using VJOURNAL entries
 */
class SyncableIssueJournalCollection extends SyncableIssueCollection {
	function SyncableIssueJournalCollection($collection, $project, $user) {
		parent::SyncableIssueCollection($collection, $project, $user);
	}

	function getName() {
		return 'notes';
	}

	function _newInstance($issue) {
		return new SyncableJournalIssue($this, $issue);
	}

	function getCalendarCapabilities() {
		return array(
			'BEGIN'			=> array('VJOURNAL'),
			'UID'			=> true,
			'URL'			=> true,
			'DTSTAMP'		=> true,
			'CREATED'		=> true,
			'LAST-MODIFIED'	=> true,
			'CLASS'			=> true,
			'STATUS'		=> true,
			'DTSTART'		=> true,
			'SUMMARY'		=> true,
			'DESCRIPTION'	=> true,
			'CATEGORIES'	=> true,
			'END'			=> array('VJOURNAL'),
			);
	}
}

/**
 * Syncable wrapper for issues
 *
 * Hold no other state information other than the wrapped issue
 * itself and the creating collection.
 *
 * @package		synd.core.module
 */
class SyncableIssue extends ISyncable {
	var $_collection = null;
	var $_issue = null;
	
	function SyncableIssue($collection, $issue) {
		$this->_collection = $collection;
		$this->_issue = $issue;
	}
	
	function id() {
		return $this->_issue->id();
	}
	
	function hasPermission($user, $perm) {
		return $this->_issue->hasPermission($user, $perm);
	}
	
	function cancel() {
		$this->_issue->cancel();
	}

	function getMemento() {
		require_once 'core/lib/calendar/VCalendar.class.inc';
		$container = new VCalendarObject();
		$container->setProperty('VERSION', '1.0');
		$container->appendChild($this->_getMemento());
		return $container;
	}
	
	/**
	 * @abstract
	 * @access	protected
	 */
	function _newMemento() {assert(false);}
	
	/**
	 * @access	protected
	 */
	function _getMemento() {
		$memento = $this->_newMemento();
		$memento->setProperty('UID', $this->_issue->id());
		$memento->setProperty('URL', tpl_request_host().tpl_link($this->_issue->getHandler(),'view',$this->_issue->nodeId));
		$memento->setProperty('DTSTAMP', gmdate('Ymd\THis\Z'));
		$memento->setProperty('CREATED', gmdate('Ymd\THis\Z', $this->_issue->data['TS_CREATE']));
		$memento->setProperty('LAST-MODIFIED', gmdate('Ymd\THis\Z', $this->_issue->data['TS_UPDATE']));
		$memento->setProperty('CLASS', 'PRIVATE');

		if ($this->_issue->getStatus() >= synd_node_issue::CLOSED)
			$memento->setProperty('STATUS', 'COMPLETED');
		else if ($this->_issue->getStatus() >= synd_node_issue::ACTIVE)
			$memento->setProperty('STATUS', 'IN PROCESS');
		else
			$memento->setProperty('STATUS', 'NEEDS ACTION');

		if (null != $this->_issue->getResolveBy())
			$memento->setStart($this->_issue->getResolveBy()));
		if ($this->_issue->isClosed())
			$memento->setCompleted($this->_issue->data['TS_RESOLVE']);
		if (count($keywords = $this->_issue->getCategories())) 
			$memento->setProperty('CATEGORIES', implode(', ', SyndLib::invoke($keywords,'toString')));
		
		$memento->setSummary($this->_issue->getTitle());
		$description = $this->_issue->getDescription();
		
		// Append notes to description
		foreach ($this->_issue->getNotes() as $note) {
			$str = "\r\n\r\n<synd:note minutes=\"".round($note->getDuration()/60)."\" id=\"".$note->nodeId."\">\r\n";
			$str .= $note->getDescription();
			$str .= "\r\n</synd:note>";
			
			if (strlen($description) + strlen($str) < 7500)
				$description .= $str;
		}

		$description .= "
		
<synd:note minutes=\"20\">
## ".SyndLib::translate('Remove the # signs to create a new note')."
</synd:note>";
		
		$memento->setDescription($description);
		return $memento;
	}
	
	function setMemento($container) {
		if (!($container instanceof VCalendar) || null == ($memento = $this->_findMemento($container)))
			return false;
		
		if ($this->_issue->isNew() && 3 == $memento->getProperty('PRIORITY'))
			$this->_issue->setStatus(synd_node_issue::ACTIVE);
		else if (null != $memento->getStart()) {
			if ($memento->getStart() < time())
				$this->_issue->setStatus(synd_node_issue::ACTIVE, $memento->getStart());
			else
				$this->_issue->setStatus(synd_node_issue::PENDING);
		}
		
		if ($memento->getProperty('PRIORITY') >= 1 && $memento->getProperty('PRIORITY') <= 3)
			$this->_issue->setPriority(3 - $memento->getProperty('PRIORITY'));
		if (null != $memento->getDue())
			$this->_issue->setResolveBy($memento->getDue());
		if (null != $memento->getSummary())
			$this->_issue->setTitle($memento->getSummary());

		// Extract notes from VTODO description
		if (null != ($description = $memento->getDescription())) {
			if (!preg_match_all('/(.*)(<synd:note[^>]*>)(.*)(?=<\/synd:note>|$)/sU', $description, $matches))
				$this->_setDescription($this->_issue, $description);
			else {
				// Issue description comes first in the VTODO description
				if (null != trim(reset($matches[1])))
					$this->_setDescription($this->_issue, trim(reset($matches[1])));
				
				// Foreach <synd:note ...> entries
				foreach ($matches[2] as $i => $header) {
					if (null != trim($matches[3][$i])) {
						if (preg_match('/ id="([^"]+)"/', $header, $id)) {
							if (null === ($task = $this->_issue->_storage->getInstance($id[1])) ||
								!$task->hasPermission($this->_collection->getUser(), 'write'))
								continue;
						}
						else if ('##' == substr(trim($matches[3][$i]),0,2)) 
							continue;
						else {
							if (false === ($task = $this->_issue->appendChild($this->_issue->_storage->factory('task'))))
								continue;
							$this->_issue->setStatus(synd_node_issue::ACTIVE);
							$task->setParent($this->_issue);
							$task->setCreator($this->_collection->getUser());
							$task->save();
						}
					
						$this->_setDescription($task, trim($matches[3][$i]));
						$task->setDuration(preg_match('/ minutes="?(\d+)"?/', $header, $duration) ? $duration[1]*60 : 20*60);
					}
				}
			}
		}

		// Find and assign categories
		if (null != ($names = $memento->getProperty('CATEGORIES'))) {
			$project = $this->_issue->getParent();
			$available = $project->getCategories();
			$seen = array();

			preg_match_all('/[^,]+/', $names, $matches);
			foreach ($matches[0] as $name) {
				foreach (array_keys($available) as $key) {
					if (strtolower(trim($available[$key]->toString())) == strtolower(trim($name))) {
						if ($this->_issue->addCategory($available[$key]))
							$this->_issue->_save();
						$seen[] = $available[$key]->nodeId;
					}
				}
			}
			
			foreach (array_keys($current = $this->_issue->getCategories()) as $key) {
				if (!in_array($current[$key]->nodeId, $seen)) {
					$this->_issue->removeCategory($current[$key]);
					$this->_issue->_save();
				}
			}
		}

		if (null != $memento->getCompleted())
			$this->_issue->setStatus(synd_node_issue::CLOSED, $memento->getCompleted());
		
		// Flush the issue so TS_UPDATE is written
		$this->_issue->flush();
		
		return true;
	}
	
	function _setDescription($issue, $description) {
		if ($this->_stripWhitespace($issue->getDescription()) !== $this->_stripWhitespace($description))
			$issue->setDescription($description);
	}
	
	function _stripWhitespace($value) {
		return preg_replace('/\s+/', '', $value);
	}
	
	function getRevisions() {
		return array();
	}
}

/**
 * VTODO wrapper
 */
class SyncableTodoIssue extends SyncableIssue {
	function _newMemento() {
		return new VCalendarTodo();
	}
	
	function _getMemento() {
		$memento = parent::_getMemento();
		if (null != $this->_issue->getResolveBy())
			$memento->setDue(strtotime('17:00:00', $this->_issue->getResolveBy()));
		if (null != $this->_issue->data['INFO_PRIO'])
			$memento->setProperty('PRIORITY', 3 - $this->_issue->data['INFO_PRIO']);
		if (null != $this->_issue->getResolveBy() && !$this->_issue->isClosed())
			$memento->setAlarm(strtotime('09:00:00', $this->_issue->getResolveBy()), $this->_issue->getTitle());
		return $memento;
	}

	function _findMemento($memento) {
		if (($memento instanceof VCalendarTodo))
			return $memento;
		foreach (array_keys($children = $memento->getChildren()) as $key) {
			if (null != ($child = $this->_findMemento($children[$key])))
				return $child;
		}
		return null;
	}
}

/**
 * VJOURNAL wrapper
 */
class SyncableJournalIssue extends SyncableIssue {
	function _newMemento() {
		return new VCalendarJournal();
	}
	
	function _findMemento($memento) {
		if (($memento instanceof VCalendarJournal))
			return $memento;
		foreach (array_keys($children = $memento->getChildren()) as $key) {
			if (null != ($child = $this->_findMemento($children[$key])))
				return $child;
		}
		return null;
	}
}
