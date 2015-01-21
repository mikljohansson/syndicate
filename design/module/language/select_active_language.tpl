{<? foreach ($languages as $code => $info) {
	if ($i++) 
		print ' ';
	print '<a href="'.tpl_link_call('language','select',$code).'" title="'.$info['name'].'">';
	if ($code == $selected) 
		print '<em>'; 
	print false === ($j = strpos($info['code'],'_')) ? $info['code'] : substr($info['code'],0,$j);
	if ($code == $selected) 
		print '</em>';
	print '</a>';
} ?>}