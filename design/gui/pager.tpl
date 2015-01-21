<? 
assert('is_numeric($count) && is_numeric($offset) && is_numeric($limit)');
if ($count && ($offset != 0 || $count > $limit)) { ?>
<div class="Pager">
	<?
	if (!isset($offset_variable_name))
		$offset_variable_name = 'offset';
	if ($offset-$limit >= 0) 
		$prev = $offset-$limit;
	if ($count-$offset > $limit)
		$next = $offset+$limit;

	if (isset($prev))
		print '<a href="'.$this->merge(array($offset_variable_name=>$prev)).'">&laquo;</a> ';
	else print '&laquo; ';

	list ($start, $stop) = SyndLib::fitInterval(floor($offset/$limit), ceil($count/$limit), 20);
	if ($start > 0)
		print ' ... ';

	for ($i=$start; $i<$stop; $i++) {
		if (floor($offset/$limit) == $i) 
			print '<b>';
		?><a href="<?= $this->merge(array($offset_variable_name=>$i*$limit)) 
			?>"><? printf("%02d", $i+1); ?></a><?
		if (floor($offset/$limit) == $i) 
			print '</b>';
		print '&nbsp;';
	}

	if ($stop < ceil($count/$limit))
		print ' ... ';
	if (isset($next))
		print '<a href="'.$this->merge(array($offset_variable_name=>$next)).'">&raquo;</a>';
	else print '&raquo;';
	
	?>
</div>
<? } ?>