<?php
require_once 'core/Module.class.inc';

/**
 * Inline HTML editor (SyndRTE) module
 * @access		protected
 * @package		synd.core.module
 */
class synd_module_rte extends Module {
	var $_api = null;

	function _hook_perm(&$hres) {
		$hres[] = 'edit_htmlarea';
	}

	function _hook_htmlarea(&$result, $id, $content, $style = null, $attrib = null, $default = null) {
		global $synd_user;
		static $hooksAttached = false;

		if ((preg_match('/MSIE (\d+)/', $_SERVER['HTTP_USER_AGENT'], $matches) && $matches[1] >= 6) ||
			strstr($_SERVER['HTTP_USER_AGENT'],'Gecko') || 
			strstr($_SERVER['HTTP_USER_AGENT'],'Opera')) {
			if (!empty($_SESSION['synd']['public']['srte']['disable'])) {
				$result  = $default;
				$result .= '<div style="font-size:7pt; margin-left:3px; margin-top:-2px;"><a title="Enable Rich-Text editor" href="';
				$result .= tpl_view_call('system','setEnvironment',SyndLib::map('senv','srte','disable',0));
				$result .= '">'.SyndLib::translate('use rte').'</a></div>';
			}
			else {
				if (!$hooksAttached) {
					$hooksAttached = true;
					tpl_load_stylesheet(tpl_design_uri('module/rte/style.css'));
					SyndLib::attachHook('html_body_post', array($this,'_callback_html_body_post'));
				}

				if (null == trim(str_replace('&nbsp;', '', $content))) {
					if (!isset($style['height']))
						$style['height'] = '200px';
					if (!isset($style['width']))
						$style['width'] = '500px';
				}
				else if (!isset($style['height']))
					$style['min-height'] = '200px';

				$result  = '<div style="'.tpl_html_implode($style,':','; ').'" class="srte_area" id="'.$id.'" '.tpl_html_implode($attrib,'="','" ').'>';
				$result .= $content;
				$result .= '</div>';

				$result .= '<div style="font-size:7pt; margin-left:3px; margin-top:-2px;"><a title="Disable Rich-Text editor" href="';
				$result .= tpl_link_call('system','setEnvironment',SyndLib::map('senv','srte','disable',1));
				$result .= '">'.SyndLib::translate('No RTE').'</a></div>';

				return true;
			}
		}
	}

	function _callback_html_body_post(&$result) {
		$result .= include(tpl_design_path('module/rte/toolbar/toolbar.inc'));

		// Use JavaScript loader
//		$result .= '<script id="srte_loader" type="text/javascript" src="'.tpl_design_uri('module/rte/loader.js').'"></script>';

		// Use PHP loader
		$result .= '<script type="text/javascript" src="'.tpl_design_uri('module/rte/loader.php').'" defer="defer" ></script>';
	}
}

