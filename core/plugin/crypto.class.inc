<?php
require_once 'core/Plugin.class.inc';
require_once 'core/lib/crypto/CryptoProtocol.class.inc';
require_once 'core/plugin/crypto/DatabaseKeyserver.class.inc';
require_once 'core/lib/SyndLib.class.inc';
require_once 'core/lib/Mime.class.inc';

/**
 * Issue module cryptography plugin 
 */
class synd_plugin_crypto extends Plugin implements CryptoProtocolFactory {
	private $_drivers = null;
	private $_keyserver = null;
	private $_project = null;
	
	function __construct($config = null) {
		parent::__construct($config);
		SyndLib::attachHook('project_admin', array($this, '_callback_project_admin'));
		SyndLib::attachHook('issue_verbatim_copy', array($this, '_callback_issue_verbatim_copy'));
		SyndLib::attachHook('issue_message_text', array($this, '_callback_issue_message_text'));
		SyndLib::attachHook('issue_filter_text', array($this, '_callback_issue_filter_text'));
		SyndLib::attachHook('email_render_html', array($this, '_callback_email_render_html'));
	}
	
	function getDriver($protocol) {
		if (!isset($this->_drivers[$protocol])) {
			require_once "core/lib/crypto/driver/$protocol.class.inc";
			$this->_drivers[$protocol] = call_user_func(array("synd_crypto_$protocol", 'factory'));
		}
		return $this->_drivers[$protocol];
	}
	
	function getKeyserver($driver, $project) {
		$keyserver = new DatabaseKeyserver($project);
		if (!empty($this->_config[$driver->getProtocol()])) {
			switch ($driver->getProtocol()) {
				case 'pgp':
					require_once 'core/lib/crypto/CompositeKeyserver.class.inc';
					$secondaries = array();
					foreach ($this->_config[$driver->getProtocol()] as $uri)
						$secondaries[] = $driver->keyserver($uri);
					$keyserver = new CompositeKeyserver($keyserver, $secondaries);
					break;
			}
		}
		return $keyserver;
	}
	
	function _callback_project_admin(&$result, $node, $request, $pane) {
		$selected = isset($request[0]) && 'crypto' == $request[0];
		$tab = null;

		if ($selected) {
			$keyserver = new DatabaseKeyserver($node);

			if (isset($request[1])) {
				if (false == ($key = $keyserver->fetch($this, $request[1])))
					throw new NotFoundException();
				$tab = new DefaultTemplate(tpl_design_path('plugin/crypto/key.tpl'));
				$tab->assign('key', $key);
				$this->_project = $node;
				SyndLib::attachHook('breadcrumbs', array($this, '_callback_breadcrumbs'));
			}
			else {
				$tab = DefaultTemplate::factory('plugin/crypto/pane.tpl', $pane);

				if (isset($request['file'])) {
					require_once 'core/lib/SyndHTML.class.inc';
					SyndHTML::validateFile($request, $errors, 'file');

					if (empty($errors)) {
						$buffer = file_get_contents($request['file']['tmp_name']);
						foreach (array_keys($this->_config) as $proto) {
							$keys = $this->getDriver($proto)->keyring($buffer);

							if (false == $keys) 
								$errors[] = SyndLib::translate('Failed to parse key(s) from uploaded file');
							else {
								foreach ($keys as $key) {
									try {
										$keyserver->store($key);
									}
									catch (RuntimeException $e) {
										$errors[] = SyndLib::translate('Failed to store key (Key ID %s, %s)', $key->getKeyid(), $key->getIdentity()->toString());
									}
								}
							}
						}
					}

					$tab->assign('errors', $errors);
				}

				$offset = isset($request['offset']) ? $request['offset'] : 0;
				$limit = 50;

				list ($matches, $count) = $keyserver->search($this, isset($request['q']) ? $request['q'] : null, $offset, $limit);
				$tab->assign('matches', $matches);
				$tab->assign('offset', $offset);
				$tab->assign('limit', $limit);
				$tab->assign('count', $count);
				$tab->assign('query', isset($request['q']) ? $request['q'] : null);
			}

			$tab->assign('node', $node);
		}

		$pane->append('tabs', array(
			'uri' => tpl_view('issue','project',$node->nodeId,'admin','crypto'),
			'text' => SyndLib::translate('Crypto'),
			'template' => $tab,
			'selected' => $selected));
	}
	
	function _callback_breadcrumbs(&$result, Template $page) {
		$result[] = array(
			'uri' => tpl_link('issue','project',$this->_project->nodeId,'admin','crypto'),
			'text' => SyndLib::translate('Crypto'));
	}

	function _callback_issue_verbatim_copy(&$result, $message) {
		$type = strtolower($message->getHeader('Content-Type'));
		return $result = ('multipart/encrypted' == $type || 'multipart/signed' == $type);
	}

	function _callback_issue_message_text(&$result, $message) {
		return $result = ('application/pgp-signature' == strtolower($message->getHeader('Content-Type')));
	}
	
	protected function _verify($project, $message, $quoted, &$cleartext) {
		$block = new Template();

		try {
			$driver = $this->getDriver('pgp');
			$keyserver = $this->getKeyserver($driver, $project);
			$cleartext = $driver->verify($keyserver, $message, $key, $keyid);

			$block->assign('key', $key);
			$block->assign('keyid', $keyid);
			$block->assign('valid', $cleartext);
			$block->assign('quoted', $quoted);
			$block->assign('project', $project);
			$result = $block->fetch(tpl_design_path('plugin/crypto/signature.tpl'));
		}
		catch (InvalidArgumentException $e) {
			$block->assign('message', SyndLib::translate('Failed to verify message signature <span class="Info">(%s)</span>', $e->getMessage()));
			$result = $block->fetch(tpl_design_path('plugin/crypto/notice.tpl'));
		}
		catch (RuntimeException $e) {
			$block->assign('message', SyndLib::translate('Failed to verify message signature <span class="Info">(%s)</span>', $e->getMessage()));
			$result = $block->fetch(tpl_design_path('plugin/crypto/warning.tpl'));
		}
		
		return $result;
	}
	
	function _callback_issue_filter_text(&$result, $issue, $text, $charset) {
		profiler_enter(__METHOD__);
		
		if (false !== strpos($text, '-----BEGIN PGP SIGNED MESSAGE-----')) {
			$pattern = '/(?<=^|\n)((?:[ \t]*(?:>|&gt;)[ \t]*)*)-----BEGIN PGP SIGNED MESSAGE-----.+?-----END PGP SIGNATURE-----/sS';
			preg_match_all($pattern, $text, $matches);
			preg_match_all($pattern, $result, $offsets, PREG_OFFSET_CAPTURE);
			$offset = 0;
			
			foreach ($matches[0] as $i => $message) {
				if (!empty($matches[1][$i]))
					$message = preg_replace('/(?<=^|\n)'.preg_quote(trim($matches[1][$i]), '/').'\s?/', '', $message);
				
				$project = $issue->getParent();
				if (!$project->isNull()) {
					$block = $this->_verify($project, 
						Mime::createTextpart($message, null, $charset), 
						!empty($matches[1][$i]), $cleartext);
					
					if (isset($offsets[0][$i][1])) {
						if ($cleartext && empty($matches[1][$i])) {
							$text = tpl_html_format(wordwrap($cleartext->getContent(), 150));
							$result = 
								substr($result, 0, $offsets[0][$i][1] + $offset).
								$block.$text.
								substr($result, $offset + $offsets[0][$i][1] + strlen($offsets[0][$i][0]));
							$offset += strlen($block) + strlen($text) - strlen($offsets[0][$i][0]);
						}
						else {
							$result = 
								substr($result, 0, $offsets[0][$i][1] + $offset).
								$block.
								substr($result, $offsets[0][$i][1] + $offset);
							$offset += strlen($block);
						}
					}
				}
			}
		}
		
		profiler_leave(__METHOD__);
	}

	function _callback_email_render_html(&$result, $parent, $message) {
		$type = strtolower($message->getHeader('Content-Type'));
		if ('multipart/signed' == $type) {
			$project = $parent;
			while (!$project->isNull() && !($project instanceof synd_node_project))
				$project = $project->getParent();
			if (!$project->isNull())
				$result .= $this->_verify($project, $message, false, $cleartext);
		}
	}
	
	function _view_setTrust(Request $request, Template $page) {
		if (!isset($request[0]) || null == ($project = SyndNodeLib::getInstance($request[0])))
			throw new InvalidArgumentException();
		if (!$project->isPermitted('admin'))
			throw new ForbiddenException();
		
		$keyserver = new DatabaseKeyserver($project);
		if (false == ($key = $keyserver->fetch($this, $request[1])))
			throw new NotFoundException();
		
		$key->setTrust((int)$request[2]);
		$keyserver->store($key);
	}
	
	function _view_delete(Request $request, Template $page) {
		if (!isset($request[0]) || null == ($project = SyndNodeLib::getInstance($request[0])))
			throw new InvalidArgumentException();
		if (!$project->isPermitted('admin'))
			throw new ForbiddenException();
		
		$keyserver = new DatabaseKeyserver($project);
		if (false == ($key = $keyserver->fetch($this, $request[1])))
			throw new NotFoundException();
		
		if (!empty($request['confirm']))
			$keyserver->delete($key);
		else {
			$page->assign('key', $key);
			$page->assign('node', $project);
			return array('content' => $page->fetch(tpl_design_path('plugin/crypto/delete.tpl')));
		}
	}
}
