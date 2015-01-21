	<? 
	if (isset($selected['partial'])) { 
		if (isset($request['partial_uri']))
			$_SERVER['REQUEST_URI'] = $request['partial_uri']; 
		?>
	<div partial="<?= $selected['partial'][0] ?><?= false===strpos($selected['partial'][0],'?')?'?':'&amp;' ?><?= http_build_query(array(
		'partial'		=> $selected['partial'][1],
		'partial_uri'	=> $_SERVER['REQUEST_URI'],
		'stack'			=> $request['stack']),null,'&amp;') ?>" class="PaneBody">
	<? } else { ?>
	<div class="PaneBody">
	<? } ?>
		<? 
		if (isset($request['partial'], $selected['partial']) && $request['partial'] == $selected['partial'][1])
			ob_start();
		
		if (!isset($selected)) 
			$selected = reset($tabs);
		if (is_array($selected['template']))
			$this->render($selected['template'][0], $selected['template'][1], $_data);
		else if (is_object($selected['template']))
			print $selected['template'];
		else
			$this->display($selected['template']);

		if (isset($request['partial'], $selected['partial']) && $request['partial'] == $selected['partial'][1])
			throw new PartialContentException(ob_get_clean());

		?>
	</div>
