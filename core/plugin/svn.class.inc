<?php
require_once 'core/Plugin.class.inc';
require_once 'core/lib/SyndLib.class.inc';

/**
 * SVN issue module plugin 
 *
 * Allows for integration of issues and SVN revisions by displaying
 * log comments and changed files
 *
 * @link		http://subversion.tigris.org/
 */
class synd_plugin_svn extends Plugin {
	function __construct($config = null) {
		parent::__construct($config);
		SyndLib::attachHook('project_edit', array($this, '_callback_project_edit'));
		SyndLib::attachHook('issue_edit_details', array($this, '_callback_issue_edit_details'));
		SyndLib::attachHook('issue_data_validate', array($this, '_callback_issue_data_validate'));
		SyndLib::attachHook('issue_data_merge', array($this, '_callback_issue_data_merge'));
		SyndLib::attachHook('issue_onpreflush', array($this, '_callback_issue_onpreflush'));
	}
	
	/**
	 * Adds the SVN repository uri textbox
	 * @access	protected
	 */
	function _callback_project_edit(&$result, $project) {
		require_once 'core/lib/Template.class.inc';
		$template = new Template();
		$template->assign('repository', $project->getAttribute('svn_repository_location'));
		$template->assign('viewvc', $project->getAttribute('svn_viewvc_location'));
		$result .= $template->fetch(tpl_design_path('plugin/svn/repository_location.tpl'));
	}

	function _callback_issue_edit_details(&$result, $issue) {
		$project = $issue->getParent();
		if (!$project->isNull() && null != $project->getAttribute('svn_repository_location')) {
			$template = new Template();
			if (isset($issue->_revision))
				$template->assign('revision', $issue->_revision);
			$result .= $template->fetch(tpl_design_path('plugin/svn/new_revision_link.tpl'));
		}
	}

	function _callback_issue_data_validate(&$result, $issue, $data) {
		if (!empty($data['svn']['revision']) && !is_numeric($data['svn']['revision'])) {
			$result['svn_revision'] = SyndLib::translate("Invalid SVN revision ('%s')", tpl_chop($data['svn']['revision'],5));
		}
	}
	
	function _callback_issue_data_merge(&$result, $issue, $data) {
		if (!empty($data['svn']['revision'])) {
			if (isset($issue->_revision))
				$issue->_revision->setContent(SyndType::factory('svn_revision', $data['svn']['revision']));
			else {
				$issue->_revision = $issue->appendChild($issue->_storage->factory('task'));
				$issue->_revision->setContent(SyndType::factory('svn_revision', $data['svn']['revision']));
			}
		}
	}

	function _callback_issue_onpreflush(&$result, $issue) {
		if (isset($issue->_revision)) {
			$issue->_revision->save();
			unset($issue->_revision);
			$issue->_node_onchange_internal();
		}
	}	
}
