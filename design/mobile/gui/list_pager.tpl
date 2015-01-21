<?
if ($offset != 0 || $count >= $limit) {
	if ($offset-$limit >= 0) 
		$prev_offset = $offset-$limit;

	if ($count >= $limit)
		$next_offset = $offset+$limit;
	
	if (isset($prev_offset)) { 
		?><a href="<?= tpl_uri_merge(array('limit'=>$limit,'offset'=>$prev_offset)) ?>"><? } 
		?>&lt;- <?= $limit ?><? if (isset($prev_offset)) { 
		?></a><?
	} 
	?> :: <?
	if (isset($next_offset)) { 
		?><a href="<?= tpl_uri_merge(array('limit'=>$limit,'offset'=>$next_offset)) ?>"><? } 
		?><?= $limit ?> -&gt;<? if (isset($next_offset)) { 
		?></a><?
	}
} 
?>