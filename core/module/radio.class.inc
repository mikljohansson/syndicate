<?php
require_once 'core/Module.class.inc';

/**
 * Web radio module
 *
 * Uses a separate frame with a Shockware Flash based MP3 player to
 * allow clients to continue browsing the site while listening to 
 * the streaming  audio. Has support for playlists.
 *
 * @access		protected
 * @package		synd.core.module
 */
class synd_module_radio extends Module {
	var $_loader = false;

	function _view_index(Request $request, Template $page) {
		return array('content' => $page->fetch(tpl_design_path('module/radio/frameset.tpl')),
			'page' => 'frame.tpl');
	}

	/**
	 * Appends a song to the playlist
	 * @param	string	Uri to play
	 * @param	string	Title of song
	 */
	function appendPlaylist($uri, $title) {
		if (null == $uri)
			return;
		if (null == $title)
			$title = basename($uri);
		
		// Remove song if exists
		foreach (array_keys($_SESSION['radio']['playlist']) as $key) {
			if (isset($_SESSION['radio']['playlist']['uri']) &&
				$_SESSION['radio']['playlist']['uri'] == $uri)
				unset($_SESSION['radio']['playlist'][$key]);
		}

		$_SESSION['radio']['playlist'][] = array('uri' => $uri, 'title' => $title);
		$_SESSION['radio']['playlist'] = array_values($_SESSION['radio']['playlist']);
	}

	function _view_append(Request $request, Template $page) {
		$this->appendPlaylist($request['uri'], $request['title']);
	}

	function _view_player(Request $request, Template $page) {
		if (!isset($_SESSION['radio']))
			$_SESSION['radio'] = array('playlist' => array(), 'current' => null);
		
		$playlist = $_SESSION['radio']['playlist'];
		$current = $_SESSION['radio']['current'];
		
		if (!isset($request[0]) || !$request[0]) {
			// Append to playlist
			if (isset($_REQUEST['synd']['radio']['uri'])) {
				$this->appendPlaylist(
					$_REQUEST['synd']['radio']['uri'], 
					$_REQUEST['synd']['radio']['title']);
				$current = end(array_keys($playlist));
			}

			// Clear cookies
			if (isset($_COOKIE['synd']['radio']['uri'])) {
				setcookie('synd[radio][uri]', null, 0, '/');
				setcookie('synd[radio][title]', null, 0, '/');
			}
			
			// Display the Flash player
			return array(
				'content' => $page->fetch(tpl_design_path('module/radio/player.tpl')),
				'page' => 'empty.tpl');
		}
		else {
			// Handle player command
			switch ($request[0]) {
				case 'play':
					break;
				
				case 'prev':
					if (!isset($playlist[--$current]))
						$current = end(array_keys($playlist));
					break;
					
				case 'next':
					if (!isset($playlist[++$current]))
						$current = reset(array_keys($playlist));
					break;

				case 'auto': 
					$current++;
					break;
				
				default:
					return array('content'=>'','page'=>'');
			}
			
			// Stop player
			if (!isset($playlist[$current]))
				return array('content' => 'uri=&title=','page' => '');

			// Play song
			return array(
				'content' => 
					'uri='.urlencode($playlist[$current]['uri']).
					'&title='.urlencode($playlist[$current]['title']),
				'page' => '');
		}
	}
}
