<form action="<?= tpl_link('search') ?>" method="get">
	<input type="text" name="q" value="<?= tpl_attribute($request['q']) ?>" size="60" />
	<select name="o" onchange="this.form.submit();">
		<?= tpl_form_options(array('date'=>tpl_text('Order by date'),'relevance'=>tpl_text('Order by relevance')), $request['o']) ?>
	</select>
	<input type="submit" value="<?= tpl_text('Search') ?>" />
	<? if (!empty($request['rset']) && is_array($request['rset'])) { ?>
	<input type="submit" name="new" value="<?= tpl_text('New search') ?>" />
		<? foreach ((array)$request['rset'] as $i => $uri) { ?>
		<input type="hidden" name="rset[<?= $i ?>]" value="<?= tpl_attribute($uri) ?>" />
		<? } ?>
		<? if (!empty($request['rsetpageid'])) { ?>
		<input type="hidden" name="rsetpageid" value="1" />
		<? } ?>
	<? } ?>
</form>

<? if (!empty($request['rset']) || !empty($request['nids'])) { ?>
	<h3><?= tpl_text('Pages similar to') ?></h3>
	<ul class="ExternalLinks">
	<? foreach ((array)$request['rset'] as $docid => $pageid) { ?>
		<? if (null != ($node = SyndLib::getInstance($pageid))) { ?>
		<li><a href="<?= tpl_link($node->getHandler(),'view',$node->nodeId) ?>"><?= $node->getTitle() ?></a></li>
		<? } else { ?>
		<li><a href="<?= tpl_attribute($pageid) ?>"><?= $pageid ?></a></li>
		<? } ?>
	<? } ?>
	</ul>
<? } ?>

<div style="margin-top:1em;">
<? if (isset($result)) { ?>
	<? if ($count) { ?>
		<div class="Result">
			<? if ('' != trim($request['q'])) { ?>
			<?= tpl_text("Results %d-%d of about %d matching <b>'%s'</b> <span class=\"Info\">(%.02f seconds)</span>", 
				$offset+1, $offset+count($result), $count, tpl_chop($request['q'],60), $duration) ?>
			<? } else { ?>
			<?= tpl_text("Results %d-%d of about %d <span class=\"Info\">(%.02f seconds)</span>", 
				$offset+1, $offset+count($result), $count, $duration) ?>
			<? } ?>
			<? if (min($count - $offset, $limit) > count($result)) { 
				?><span class="Info"> <?= tpl_text('(Some matches are excluded due to access restrictions)') ?></span><? 
			} ?><br />
			<? $this->display('gui/pager.tpl') ?>
		</div>
		<?= SyndLib::runHook('search_result',$request,$this,$request['q'],$offset,$limit,$count) ?>
		<? $this->display('module/search/default_query_suggestions.tpl'); ?>
		<? foreach (array_keys($result) as $docid) { 
			$rset = (array)$request['rset'];
			$rset[$docid] = $result[$docid]->id(); 
			$docids = empty($request['rset']) || !empty($request['docids']);
		?>
		<? $this->render($result[$docid],'index_search_result.tpl',array(
			'request'=>$request,'highlight'=>$highlight,'rset'=>$rset,'docids'=>$docids)) ?>
		<? } ?>
	<? } else { ?>
		<? $this->display('module/search/default_query_suggestions.tpl') ?>
		<?= SyndLib::runHook('search_result',$request,$this,$request['q'],$offset,$limit,$count) ?>
		<em><?= tpl_text("No results matching <b>'%s'</b> were found", $request['q']) ?></em>
	<? } ?>
<? } else { ?>
	<div class="Info">
		<?= tpl_text('Use +/- to force include and exclude of words and &quot; to specify phrases.') ?>
		<? if (null != ($link = Module::runHook('search_help_uri'))) { ?>
		<?= tpl_translate('Consult this <a href="%s">page</a> for more information.', $link) ?>
		<? } ?>
	</div>
<? } ?>
</div>
