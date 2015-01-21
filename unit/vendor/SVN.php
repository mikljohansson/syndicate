<?php
require_once 'PHPUnit2/Framework/TestCase.php';

class _vendor_SVN extends PHPUnit2_Framework_TestCase {
	function setUp() {
		require_once 'core/lib/vendor/SVN.class.inc';
	}
	
	function testFetchRevision() {
		$repository = new SvnWebDavRepository('http://svn.synd.info/synd/');
		$revision = $repository->getRevision('1989');

		$expected = "Refactored synd_module_issue::_view_mail()somewhat";
		$actual = $revision->getComment();
		$this->assertEquals($expected, $actual);
		
		$expected = array(
			new SvnRevisionModify('/branches/PHP4/core/module/issue.module.inc'),
			new SvnRevisionModify('/branches/PHP4/unit/module/Issue.php'),
			new SvnRevisionDelete('/branches/PHP4/unit/module/issue/mail-005.msg'),
			new SvnRevisionAdd('/branches/PHP4/unit/module/issue/mail-005-attachment.msg'),
			);
		$actual = $revision->getModifications();
		$this->assertEquals($expected, $actual);
		
		$expected = 'mdb';
		$actual = $revision->getCreator();
		$this->assertEquals($expected, $actual);
		
		$expected = strtotime('2005-10-04 14:46:34Z');
		$actual = $revision->getCreateTime();
		$this->assertEquals($expected, $actual);
	}
}