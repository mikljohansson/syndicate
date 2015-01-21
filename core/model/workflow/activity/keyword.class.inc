<?php
require_once 'core/model/workflow/activity/instance.class.inc';

/**
 * Adds a keyword to an issue
 */
class synd_workflow_keyword extends synd_workflow_instance {
	function __toString() {
		return SyndLib::translate('Add category');
	}

	function process(Workflow $workflow, $subjects, Response $response) {
		foreach ($subjects as $subject)
			$subject->addCategory($this->getSelectedOption());
	}
	
	function getOptions() {
		global $synd_user;
		$categories = array();
		foreach (Module::getInstance('issue')->getProjectTree(new MethodDecider('hasPermission', array($synd_user, 'monitor')), $synd_user) as $project)
			$categories = array_merge($categories, $project->getLocalCategories());
		return SyndLib::sort($categories);
	}
}
