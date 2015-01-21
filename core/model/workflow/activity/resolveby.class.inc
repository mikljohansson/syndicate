<?php
require_once 'core/model/workflow/activity/text.class.inc';

/**
 * Modifies the resolve by timestamp on an issue
 */
class synd_workflow_resolveby extends synd_workflow_text {
	protected $_relativetoday = false;
	
	function __toString() {
		return SyndLib::translate('Modify due date');
	}

	function process(Workflow $workflow, $subjects, Response $response) {
		foreach ($subjects as $subject) {
			$ts = $this->getRelativeToday() ? 
				strtotime($this->_option) :
				strtotime($this->_option, $subject->getResolveBy());
			
			$subject->setResolveBy($ts);
		}
	}

	function setSelectedOption($option) {
		if (strtotime($option) !== false) {
			parent::setSelectedOption($option);
		}
	}

	function getRelativeToday() {
		return $this->_relativetoday;
	}

	function _view_setRelativeToday(Request $request, Template $page) {
		$this->_relativetoday = !empty($request['relativetoday']);
	}
}
