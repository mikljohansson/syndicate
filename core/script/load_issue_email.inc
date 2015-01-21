<?php
require_once 'core/lib/Mime.class.inc';
require_once 'core/lib/HttpConnection.class.inc';
require_once 'core/model/storage/DatabaseStorage.class.inc';

if (!isset($args['--trec9']) || !is_file($args['--trec9']))
	return trigger_error('Please specify the location of the TREC9 ohsumed database using --trec9', E_USER_ERROR);

/**
 * @access	private
 */
class IssueTransaction {
	var $_storage = null;
	var $_fp = null;
	
	var $_pNewIssue = 0.1;
	var $_pNewNote = 0.15;
	var $_pLoadIssue = 0.3;
	
	function IssueTransaction($storage, $trec9) {
		$this->_storage = $storage;
		$this->_project = $storage->factory('project');
		$this->_project->save();
		$storage->flush();
		
		$this->_fp = fopen($trec9, 'r');
		fgets($this->_fp, 8192);
		
		$pList = 1 - $this->_pNewIssue - $this->_pNewNote - $this->_pLoadIssue;
		print "newIssue($this->_pNewIssue), newNote($this->_pNewNote), view($this->_pLoadIssue), list($pList)\n";
	}
	
	function run() {
		if (feof($this->_fp))
			return;
		
		$m = 1000;
		$r = mt_rand(1,$m);
		
		if ($r <= $this->_pNewIssue * $m)
			return $this->_newIssue();
		if ($r <= $this->_pNewNote * $m)
			return $this->_newNote();
		if ($r <= $this->_pLoadIssue * $m)
			return $this->_loadIssue();
		return $this->_loadListing();
	}
	
	function _newIssue() {
		list($title, $description, $author) = $this->_trec9();
		
		$email = '
From mikael@example.com  Tue Nov  2 15:16:26 2004
Return-Path: <mikael@example.com>
X-Original-To: issues+_unit_test+unassigned@example.com
Delivered-To: issues+_unit_test+unassigned@example.com
Received: from localhost (localhost.localdomain [127.0.0.1])
        by mail.example.com (Postfix) with ESMTP id 771F31D316C
        for <issues+_unit_test+unassigned@example.com>; Tue,  2 Nov 2004 15:16:26  0100 (CET)
Received: from mail.example.com ([127.0.0.1])
 by localhost (morbo [127.0.0.1]) (amavisd-new, port 10024) with LMTP
 id 31212-01-82 for <issues+_unit_test+unassigned@example.com>;
 Tue,  2 Nov 2004 15:16:26  0100 (CET)
Received: from shell.example.com (shell.example.com [129.16.214.9])
        by mail.example.com (Postfix) with ESMTP id 4C5701D3164
        for <issues+_unit_test+unassigned@example.com>; Tue,  2 Nov 2004 15:16:26  0100 (CET)
Received: by shell.example.com (Postfix, from userid 27846)
        id 413B156803; Tue,  2 Nov 2004 15:15:33  0100 (CET)
Received: from localhost (localhost [127.0.0.1])
        by shell.example.com (Postfix) with ESMTP id 3D8A357001
        for <issues+_unit_test+unassigned@example.com>; Tue,  2 Nov 2004 15:15:33  0100 (CET)
Date: Tue, 2 Nov 2004 15:15:33  0100 (CET)
From: Mikael Johansson <mikael@example.com>
X-X-Sender: mikl@shell.example.com
To: issues+_unit_test+unassigned@example.com
Subject: ';
		$email .= MimeHeader::encodeHeader('Subject', substr($title,0,35));
		$email .= '
Message-ID: <Pine.LNX.4.60.0411021515220.1874@shell.example.com>
MIME-Version: 1.0
Content-Type: TEXT/PLAIN; charset=US-ASCII; format=flowed
X-Virus-Scanned: by amavisd-new at example.com

';
		$email .= $description;
		
		$connection = new HttpConnection('http://devel.synd.info');
		$response = $connection->postRequest('/issue/mail/'.$this->_project->nodeId.'/', $email, 'text/plain');
		print $response;
	}
	
	function _newNote() {
		list($title, $description, $author) = $this->_trec9();
		
		$sql = "
			SELECT i.node_id FROM synd_issue i
			ORDER BY i.ts_create DESC";
		$id = $this->_storage->_db->getOne($sql);
		$oid = substr($id, strpos($id, '.')+1);
		
		$email = '
From mikael@example.com  Tue Nov  2 15:16:26 2004
Return-Path: <mikael@example.com>
X-Original-To: issues+_unit_test+unassigned@example.com
Delivered-To: issues+_unit_test+unassigned@example.com
Received: from localhost (localhost.localdomain [127.0.0.1])
        by mail.example.com (Postfix) with ESMTP id 771F31D316C
        for <issues+_unit_test+unassigned@example.com>; Tue,  2 Nov 2004 15:16:26  0100 (CET)
Received: from mail.example.com ([127.0.0.1])
 by localhost (morbo [127.0.0.1]) (amavisd-new, port 10024) with LMTP
 id 31212-01-82 for <issues+_unit_test+unassigned@example.com>;
 Tue,  2 Nov 2004 15:16:26  0100 (CET)
Received: from shell.example.com (shell.example.com [129.16.214.9])
        by mail.example.com (Postfix) with ESMTP id 4C5701D3164
        for <issues+_unit_test+unassigned@example.com>; Tue,  2 Nov 2004 15:16:26  0100 (CET)
Received: by shell.example.com (Postfix, from userid 27846)
        id 413B156803; Tue,  2 Nov 2004 15:15:33  0100 (CET)
Received: from localhost (localhost [127.0.0.1])
        by shell.example.com (Postfix) with ESMTP id 3D8A357001
        for <issues+_unit_test+unassigned@example.com>; Tue,  2 Nov 2004 15:15:33  0100 (CET)
Date: Tue, 2 Nov 2004 15:15:33  0100 (CET)
From: Mikael Johansson <mikael@example.com>
X-X-Sender: mikl@shell.example.com
To: issues+_unit_test+unassigned@example.com
Subject: ';
		$email .= MimeHeader::encodeHeader('Subject', substr($title,0,20).' #'.$oid);
		$email .= '
Message-ID: <Pine.LNX.4.60.0411021515220.1874@shell.example.com>
MIME-Version: 1.0
Content-Type: TEXT/PLAIN; charset=US-ASCII; format=flowed
X-Virus-Scanned: by amavisd-new at example.com

';
		$email .= $description;
		
		$connection = new HttpConnection('http://devel.synd.info');
		$response = $connection->postRequest('/issue/mail/'.$this->_project->nodeId.'/', $email, 'text/plain');
		print $response;
	}

	function _loadIssue() {
		$sql = "
			SELECT i.NODE_ID, i.INFO_PRIVATE_KEY FROM synd_issue i
			WHERE i.assigned_node_id = 'user.1'
			ORDER BY RAND()";
		$row = $this->_storage->_db->getRow($sql);
			
		$ts = round(time()/3600/24);
		$token = substr(md5($ts.$row['INFO_PRIVATE_KEY']),0,16).dechex($ts);

		$response = file_get_contents('http://devel.synd.info/issue/view/'.$row['NODE_ID'].'/'.$token.'/');
	}

	function _loadListing() {
		$response = file_get_contents('http://devel.synd.info/issue/default/issues/');
	}

	function _trec9() {
		$title = null;
		$description = null;
		$author = null;
		
		while (!feof($this->_fp)) {
			$line = fgets($this->_fp, 8192);
			if ('.I' == substr($line,0,2))
				break;
			else {
				$text = trim(fgets($this->_fp, 8192));
				switch (substr($line,0,2)) {
					case '.T':
						$title = $text;
						break;
					case '.W':
						$description = $text;
						break;
					case '.A':
						$author = $text;
						break;
				}
			}
		}
		
		return array($title, $description, $author);
	}
}

$transaction = new IssueTransaction(new DatabaseStorage($synd_maindb), $args['--trec9']);
