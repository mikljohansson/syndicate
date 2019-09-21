<?php
require_once 'core/Module.class.inc';

/**
 * Language module
 *
 * Allows for automatic preferred language detection and GUI 
 * translation similar to gettext().
 *
 * Implements two stage caching where short strings are cached 
 * together and long strings are cached separatly. Enable the cache
 * module for maximum perfomance.
 *
 * @access		protected
 * @package		synd.core.module
 */
class synd_module_language extends Module {
	private $_db = null;
	
	/**
	 * Currently selected locale
	 */
	private $_locale = null;

	/**
	 * Buffered translations
	 */
	private $_translations = null;
	
	/**
	 * Max length of strings to cache inline
	 */
	private $_inlineLength = 25;
	
	function __construct($config = null) {
		global $synd_maindb;
		parent::__construct($config);
		
		$this->_db = $synd_maindb;
		
		SyndLib::attachHook('text', array($this, '_callback_text'));
		SyndLib::attachHook('setlocale', array($this, '_callback_setlocale'));
		SyndLib::attachHook('getlocale', array($this, '_callback_getlocale'));
		SyndLib::attachHook('getlocales', array($this, '_callback_getlocales'));
		SyndLib::attachHook('detect_locale', array($this, '_callback_detect_locale'));
		
		if (count($this->_config) >= 2)
			SyndLib::attachHook('menu_header', array($this, '_callback_menu_header'), -250);
		SyndLib::attachHook('menu_admin', array($this, '_callback_menu_admin'));
	}
	
	/**
	 * Returns the negotiated locale
	 * @return	string	{en, sv, ..}
	 */
	function getLocale() {
		if (null === $this->_locale) {
			if (isset($_SESSION['synd']['locale']))
				$this->_locale = $_SESSION['synd']['locale'];
			else
				$this->_locale = $_SESSION['synd']['locale'] = self::negotiateLanguage(array_map('strtolower', array_keys($this->_config)), 'en');
			if (isset($this->_config[$this->_locale]['code']))
				setlocale(LC_TIME, $this->_config[$this->_locale]['code']);
		}
		return $this->_locale;
	}

	/**
	 * Negotiate Language
	 * 
	 * Negotiate language with the user's browser through the Accept-Language 
	 * HTTP header or the user's host address.  Language codes are generally in 
	 * the form "ll" for a language spoken in only one country, or "ll-CC" for a 
	 * language spoken in a particular country.  For example, U.S. English is 
	 * "en-US", while British English is "en-UK".  Portugese as spoken in
	 * Portugal is "pt-PT", while Brazilian Portugese is "pt-BR".
	 * 
	 * Quality factors in the Accept-Language: header are supported, e.g.:
	 *	  Accept-Language: en-UK;q=0.7, en-US;q=0.6, no, dk;q=0.8
	 * 
	 * <code>
	 *  $supported = array('en','en-us','en-uk','de','de-de','de-at');
	 * </code>
	 * 
	 * @param   array   An array of supported languages
	 * @param   string  The default language to use if none is found.
	 * @return  string  The negotiated language result or the supplied default.
	 */
	static function negotiateLanguage($supported, $default = 'en-US') {
		if (empty($supported))
			return $default;

		$matches = array();
		if (isset($_SERVER['HTTP_ACCEPT_LANGUAGE'])) {
			foreach (explode(',', $_SERVER['HTTP_ACCEPT_LANGUAGE']) as $lang) {
				$lang = array_map('trim', explode(';', $lang));
				
				if (isset($lang[1])) {
					$language = strtolower($lang[0]);
					$quality = (float) str_replace('q=', '', $lang[1]);
				} else {
					$language = strtolower($lang[0]);
					$quality = null;
				}
				
				if (in_array($language, $supported) || 
					false !== ($i = strpos($language, '-')) && in_array($language = substr($language,0,$i), $supported))
					$matches[$language] = null != $quality ? $quality : 1000 - count($matches);
			}
		}

		if (count($matches)) {
			asort($matches, SORT_NUMERIC);
			$langs = array_keys($matches);
			return end($langs);
		}
		
		if (isset($_SERVER['REMOTE_HOST'])) {
			$host = explode('.', $_SERVER['REMOTE_HOST']);
			$tld = strtolower(end($host));
			if (in_array($tld, $supported))
				return $tld;
		}

		return $default;
	}

	/**
	 * @access	protected
	 */
	function _hook_manage_permissions(&$result) {
		$result[] = $this;	
	}
	
	function getDescription() {
		return SyndLib::translate('Manages localization and allows the GUI to be translated into various languages.');
	}

	function _callback_menu_header(&$result, Template $page, $menu) {
		if ($this->isPermitted('read')) {
			$selector = clone $page;
			$selector->assign('languages', $this->_config);
			$selector->assign('selected', $this->getLocale());
			$menu->append(null, $selector->fetch(tpl_design_path('module/language/select_active_language.tpl')));
		}
	}

	function _callback_menu_admin(&$result, $menu) {
		if ($this->isPermitted('admin'))
			$menu->append(tpl_link('system','language'), SyndLib::translate('Language'), '', array($this, '_callback_menu_admin_index'), array('system/language/*'));
		else if ($this->isPermitted('write'))
			$menu->append(tpl_link('system','language'), SyndLib::translate('Language'));
	}

	function _callback_menu_admin_index($menu) {
		$menu->append(tpl_link('system','language','import'), SyndLib::translate('Import'));
		$menu->append(tpl_link('system','language','detection'), SyndLib::translate('Detection'));
	}

	function _callback_setlocale(&$result, $category) {
		if (isset($this->_config[$this->getLocale()]['code']))
			setlocale($category, $this->_config[$this->getLocale()]['code']);
	}
	
	/**
	 * Attempts to detect the language given a text
	 *
	 * Uses n-gram (currently 3-grams) based language detection which
	 * may be trained with text i variuous locales through the 
	 * system/langage/ screen
	 *
	 * @param	string	The text to process
	 * @param	bool	Set the detected locale
	 * @return	string	Return the language code ('en', 'sv', ...)
	 */
	function _callback_detect_locale(&$result, $text, $setlocale = false) {
		if (count($scores = $this->rankLocales($this->_db, $text))) {
			end($scores); $result = key($scores);

                        if ($setlocale && isset($this->_config[$result]['code'])) {
                                $this->_locale = $result;
                                setlocale(LC_TIME, $this->_config[$result]['code']);
                        }

			return true;
		}
	}
	
	function _callback_getlocale(&$result) {
		return $result = $this->getLocale();
	}
	
	function _callback_getlocales(&$result) {
		$result = array_merge((array)$result, $this->_config);
	}
	
	function _view_index(Request $request, Template $page) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();

		$filter = null;
		$offset = isset($_REQUEST['offset']) ? $_REQUEST['offset'] : 0;
		$limit = 25;
		
		if (!empty($request['filter'])) {
			$filter = "
				WHERE 
					LOWER(ls.string) LIKE ".$this->_db->quote('%'.strtolower(trim($request['filter'])).'%')." OR 
					LOWER(ls.location) LIKE ".$this->_db->quote('%'.strtolower(trim($request['filter'])).'%');
		}

		$sql = "
			SELECT ls.* FROM synd_locale_string ls $filter
			ORDER BY ls.string";
		$strings = $this->_db->getAll($sql, $offset, $limit);

		$page->assign('module', $this);
		$page->assign('request', $request);
		$page->assign('limit', $limit);
		$page->assign('offset', $offset);
		
		$sql = "
			SELECT COUNT(*) FROM synd_locale_string ls 
			$filter";
		$page->assign('count', $this->_db->getOne($sql));
			
		$page->assign('strings', $strings);
		$page->assign('locales', $this->_config);

		$translations = array('short' => array(), 'long' => array());
		foreach (array_keys($this->_config) as $locale) {
			$sql = "
				SELECT ls.string, lt.translation
				FROM synd_locale_string ls
					LEFT JOIN synd_locale_translation lt
					ON (ls.lid = lt.lid AND lt.locale = ".
					$this->_db->quote($locale).") $filter
				ORDER BY ls.string";
			$translations[$locale] = $this->_db->getAssoc($sql, $offset, $limit);
		}
		
		$page->assign('translations', $translations);

		return array('content' => $page->fetch(tpl_design_path('module/language/index.tpl')));
	}
	
	function _view_select(Request $request, Template $page) {
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();
		if (!isset($request[0], $this->_config[$request[0]]))
			throw new InvalidArgumentException();
		$_SESSION['synd']['locale'] = $request[0];
	}
	
	function _view_saveStrings(Request $request, Template $page) {
		require_once 'core/db/SyndDBLib.class.inc';
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();

		foreach ((array)$request['strings'] as $locale => $strings) {
			$sql = "
				SELECT ls.lid, lt.translation 
				FROM synd_locale_string ls, synd_locale_translation lt
				WHERE ls.lid = lt.lid AND lt.locale = ".$this->_db->quote($locale);
			$translations = $this->_db->getAssoc($sql);

			foreach ($strings as $lid => $translation) {
				if (array_key_exists($lid, $translations)) {
					if (null == $translation) {
						$sql = "
							DELETE FROM synd_locale_translation
							WHERE 
								lid = ".$this->_db->quote($lid)." AND
								locale = ".$this->_db->quote($locale);
						$this->_db->query($sql);
					}
					else if ($translations[$lid] != $translation) {
						$sql = "
							UPDATE synd_locale_translation
							SET translation = ".$this->_db->quote($translation)."
							WHERE 
								lid = ".$this->_db->quote($lid)." AND
								locale = ".$this->_db->quote($locale);
						$this->_db->query($sql);
					}

					SyndLib::runHook('cache_del', "locale:$locale:$lid");
				}
				else if (null != $translation) {
					$sql = "
						INSERT INTO synd_locale_translation
						(lid, locale, translation)
						VALUES
						(".$this->_db->quote($lid).",
						 ".$this->_db->quote($locale).",
						 ".$this->_db->quote($translation).")";
					$this->_db->query($sql);
					
					SyndLib::runHook('cache_del', "locale:$locale:$lid");
				}
			}

			SyndLib::runHook('cache_del', "locale:$locale");
		}
	}

	function _view_deleteString(Request $request, Template $page) {
		if (!$this->isPermitted('write'))
			throw new ForbiddenException();

		if (!empty($request['lid'])) {
			$sql = "
				DELETE FROM synd_locale_string
				WHERE lid = ".$this->_db->quote($request['lid']);
			$this->_db->query($sql);
			$sql = "
				DELETE FROM synd_locale_translation
				WHERE lid = ".$this->_db->quote($request['lid']);
			$this->_db->query($sql);
		
			$this->invalidate($request['lid']);
		}
	}
	
	function _view_import(Request $request, Template $page) {
		if (!$this->isPermitted('admin'))
			throw new ForbiddenException();

		$page->assign('request', $request);
		
		$errors = array();
		$status = array();
		
		if (isset($request['post'])) {
			require_once 'core/lib/rpc/RpcTransport.class.inc';
			$transport = Activator::getTransport('xmlrpc;'.$request['uri']);
			
			foreach (array_keys($this->_config) as $locale) {
				$translations = $transport->call('getTranslations', array($locale));
				if (!is_array($translations)) {
					$errors[] = SyndLib::translate('Invalid response from %s', $request['uri']);
					break;
				}

				$sql = "
					SELECT ls.lid, lt.lid
					FROM synd_locale_string ls
						LEFT JOIN synd_locale_translation lt
						ON (ls.lid = lt.lid AND lt.locale = ".$this->_db->quote($locale).")";
				
				$lids = $this->_db->getAssoc($sql);
				$newStrings = array();
				$newTranslations = array();
				$delete = array();
				
				foreach ($translations as $translation) {
					if (!empty($request['inclusive']) && !array_key_exists($translation['LID'], $lids)) {
						$newStrings[] = array(
							'lid' => $translation['LID'],
							'string' => $translation['STRING']);
					}
					if (!empty($request['overwrite']) || !isset($lids[$translation['LID']]) && 
						(!empty($request['inclusive']) || array_key_exists($translation['LID'], $lids))) {
						$newTranslations[] = array(
							'lid' => $translation['LID'],
							'locale' => $locale,
							'translation' => $translation['TRANSLATION']);
						$delete[] = $translation['LID'];
					}

					SyndLib::runHook('cache_del', "locale:$locale:{$translation['LID']}");
				}
				
				for ($i=0; $i<count($delete); $i+=1000) {
					$sql = "
						DELETE FROM synd_locale_translation
						WHERE 
							locale = ".$this->_db->quote($locale)." AND
							lid IN (".implode(',',$this->_db->quote(array_slice($delete,$i,1000))).")";
					$this->_db->query($sql);
				}
				
				$this->_db->load('synd_locale_string', $newStrings);
				$this->_db->load('synd_locale_translation', $newTranslations);
				SyndLib::runHook('cache_del', "locale:$locale");
				
				$status[] = SyndLib::translate('Imported %d %s (%s) translations from %s', count($newTranslations), 
					$this->_config[$locale]['name'], $locale, $request['uri']);
			}
		}
		
		$page->assign('errors', $errors);
		$page->assign('status', $status);
		
		return array('content' => $page->fetch(tpl_design_path('module/language/import.tpl')));
	}

	function _view_detection(Request $request, Template $page) {
		if (!$this->isPermitted('admin'))
			throw new ForbiddenException();

		$page->assign('request', $request);
		$page->assign('locales', $this->_config);

		if (isset($request['train'])) {
			$grams = $this->trainDetection($request['locale'], $request['text']);
			$page->assign('status', SyndLib::translate("Trained the language detection routine using %d %d-grams from language '%s'", $grams, $this->getDetectionLength(), $request['locale']));
		}
		else if (isset($request['detect']))
			$page->assign('scores', $this->rankLocales($this->_db, $request['text']));

		return array('content' => $page->fetch(tpl_design_path('module/language/detection.tpl')));
	}

	/**
	 * Extracts n-grams from a text
	 * @param	integer	Gram length
	 * @param	string	Locale of text (en, sv, ..)
	 * @param	string	Text to process
	 * @return	array	Returns an associative array of grams and their frequencies
	 */
	function getGrams($length, $locale, $text) {
		$prev = setlocale(LC_CTYPE, $locale);
		$grams = array();
		
		for ($i=0, $l=strlen($text); $i<$l; $i++) {
			$gram = '';
			
			for ($j=$i; $j<$l && strlen($gram)<$length; $j++) {
				if (ctype_alpha($text[$j]))
					$gram .= $text[$j];
				else if (0 == strlen($gram) || ctype_alpha($gram[strlen($gram)-1]))
					$gram .= ' ';
			}
			
			if (strlen($gram) >= $length-1) {
				$gram = sprintf("%-{$length}s", strtolower($gram));
				if (isset($grams[$gram]))
					$grams[$gram]++;
				else
					$grams[$gram] = 1;
			}
		}
	
		setlocale(LC_CTYPE, $prev);
		return $grams;
	}

	/**
	 * Trains the languange detection routines
	 * @param	string	The locale to train (en, sv, ..)
	 * @param	string	The text to train on
	 * @return	integer	Returns the number of extracted n-grams
	 */
	function trainDetection($locale, $text) {
		$grams = $this->getGrams($this->getDetectionLength(), $locale, $text);

		foreach ($grams as $gram => $freq) {
			$sql = "
				UPDATE synd_locale_gram
				SET frequency = frequency + ".$this->_db->quote($freq)."
				WHERE 
					locale = ".$this->_db->quote($locale)." AND
					gram = ".$this->_db->quote($gram);
			$this->_db->query($sql);
			
			if (!$this->_db->affectedRows()) {
				$sql = "
					INSERT INTO synd_locale_gram
					(locale, gram, frequency)
					VALUES
					(".$this->_db->quote($locale).",
					 ".$this->_db->quote($gram).",
					 ".$this->_db->quote($freq).")";
				$this->_db->query($sql);
			}
		}
		
		return count($grams);
	}
	
	/**
	 * @access	private
	 */
	function rankLocales($database, $text) {
		$scores = array();
		
		foreach (array_keys((array)$this->_config) as $locale) {
			if (count($grams = $this->getGrams($this->getDetectionLength(), $locale, $text))) {
				$keys = array_slice(array_keys($grams),0,1000);
				$sql = "
					SELECT g.gram, g.frequency FROM synd_locale_gram g
					WHERE 
						g.locale = ".$database->quote($locale)." AND
						g.gram IN (".implode(',',$database->quote($keys)).")";
				
				$stats = $database->getAssoc($sql);
				$scores[$locale] = 0;
				$mstats = empty($stats) ? 1 : max($stats);
				$mgrams = empty($grams) ? 1 : max($grams);
				$i = 0;
				
				foreach ($keys as $gram) {
					if (isset($stats[$gram]))
						$scores[$locale] += ($stats[$gram] / $mstats) * ($grams[$gram] / $mgrams) * pow(0.9975, $i);
					$i++;
				}
			}
		}
		
		asort($scores);
		return $scores;
	}

	/**
	 * @access	private
	 */
	function getDetectionLength() {
		return 3;
	}

	/**
	 * Returns the translations for a locale
	 * @param	string	Locale to export {en, se, ...}
	 * @return	array
	 */
	function _public_getTranslations($locale) {
		if (!$this->isPermitted('read'))
			throw new ForbiddenException();
		$sql = "
			SELECT ls.LID, ls.STRING, lt.TRANSLATION 
			FROM synd_locale_string ls, synd_locale_translation lt
			WHERE 
				ls.lid = lt.lid AND
				lt.locale = ".$this->_db->quote($locale);
		return $this->_db->getAll($sql);
	}
	
	protected function invalidate($lid) {
		foreach (array_keys($this->_config) as $locale) {
			SyndLib::runHook('cache_del', "locale:$locale");
			SyndLib::runHook('cache_del', "locale:$locale:$lid");
		}
	}

	function _callback_text(&$result, $string) {
		if (null == $string)
			return false;
		
		if (null === $this->_translations && null === ($this->_translations = SyndLib::runHook('cache_get', 'locale:'.$this->getLocale()))) {
			try {
				$sql = "
					SELECT ls.string, lt.translation
					FROM synd_locale_string ls
						LEFT JOIN synd_locale_translation lt
						ON (ls.lid = lt.lid AND lt.locale = ".$this->_db->quote($this->getLocale()).")
					WHERE LENGTH(ls.string) < {$this->_inlineLength}";

				$sql2 = "
					SELECT ls.lid, 1 FROM synd_locale_string ls
					WHERE LENGTH(ls.string) >= {$this->_inlineLength}";

				$this->_translations = array(
					'inline'	=> $this->_db->getAssoc($sql),
					'separate'	=> $this->_db->getAssoc($sql2));
				SyndLib::runHook('cache_set', 'locale:'.$this->getLocale(), $this->_translations);
			}
			catch (DatabaseException $e) {
				return false;
			}
		}

		if (!array_key_exists($string, $this->_translations['inline'])) {
			$lid = md5($string);

			try {
				if (!isset($this->_translations['separate'][$lid])) {
					$sql = "
						INSERT INTO synd_locale_string
						(lid, location, string)
						VALUES
						(".$this->_db->quote($lid).", 
						 ".$this->_db->quote(isset($_SERVER['REDIRECT_URL']) ? $_SERVER['REDIRECT_URL'] : $_SERVER['SCRIPT_NAME']).", 
						 ".$this->_db->quote($string).")";
					$this->_db->query($sql);
					$this->invalidate($lid);
					$this->_translations['inline'][$string] = null;
				}
				else if (null === ($this->_translations['inline'][$string] = SyndLib::runHook('cache_get', $cid = 'locale:'.$this->getLocale().':'.$lid))) {
					$sql = "
						SELECT lt.translation FROM synd_locale_translation lt
						WHERE 
							lt.lid = ".$this->_db->quote($lid)." AND
							lt.locale = ".$this->_db->quote($this->getLocale());
					$this->_translations['inline'][$string] = $this->_db->getOne($sql);
					SyndLib::runHook('cache_set', $cid, $this->_translations['inline'][$string]);
				}
			}
			catch (DatabaseException $e) {
				return false;
			}
		}

		if (null != $this->_translations['inline'][$string]) {
			$result = $this->_translations['inline'][$string];
			return true;
		}
	}
}
