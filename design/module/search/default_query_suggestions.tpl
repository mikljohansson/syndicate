		<? if (!empty($suggestions)) { 
			$query = $request['query'];
			$query2 = tpl_chop($request['query'],60);
			foreach ($suggestions as $term => $alternate) {
				$query = preg_replace('/\b'.preg_quote($term, '/').'\b/i', $alternate, $query);
				$query2 = preg_replace('/\b'.preg_quote($term, '/').'\b/i', "<em>$alternate</em>", $query2);
			}
			?>
			<div class="SearchSuggestion">
				<?= tpl_translate('Did you mean: <a href="%s">%s</a>', tpl_link('search',null,array('query'=>$query)), $query2) ?>
			</div>
		<? } ?>
