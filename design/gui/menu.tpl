<?
if (!function_exists('_page_menu_cmp')) {
	function _page_menu_cmp($a, $b) {
		$pri_a = isset($a['prio']) ? $a['prio'] : 0;
		$pri_b = isset($b['prio']) ? $b['prio'] : 0;
		return $pri_b-$pri_a;
	}

	function _page_menu_print($menu, $rec = 0) {
		$output = null;
		$submenu = null;

		//$menu = array_reverse($menu);
		//uasort($menu, '_page_menu_cmp');

		foreach (array_keys($menu) as $key) {
			if (is_array($menu[$key])) 
				$submenu .= _page_menu_print($menu[$key], $rec+1);
		}

		if (isset($menu['content'])) 
			$output .= $menu['content'];
		else if (isset($menu['text'])) {
			$output .= (null != $submenu ? '<li class="bold">' : '<li>');
			if (!empty($menu['uri'])) 
				$output .= '<a'.(isset($menu['title'])?" title=\"{$menu['title']}\"":'')." href=\"{$menu['uri']}\">{$menu['text']}</a></li>\n\t";
			else 
				$output .= $menu['text']."</li>\n\t";
		}
		
		if (null != $submenu)
			return "$output\n\t<ul>$submenu</ul>";
		return $output;
	}
}

if (count($menu)) {
//	$menu = array_reverse($menu);
//	uasort($menu, '_page_menu_cmp');
	?><ul class="Menu"><?
	foreach (array_keys($menu) as $key)
		print _page_menu_print($menu[$key]);
	?></ul><?
}
?>