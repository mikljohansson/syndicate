<h2><?= tpl_text('Searching in document fields') ?></h2>
<p><?= tpl_text("Search specific fields such as title or description by entering the field name followed by a colon the search query. For example; searching for 'title:browser' would return documents with a title containing the word 'browser'. Documents where the word 'browser' appears in other fields would not be included. Also; please note that not all document types support the entire range of fields.") ?></p>

<? 
$search = Module::getInstance('search'); 
$fields = $extension->getFields();
ksort($fields);
?>
<table>
	<caption><?= tpl_text('Available fields') ?></caption>
	<? foreach ($fields as $field => $hash) { ?>
		<tr>
			<th><?= $field ?></th>
			<td><?= is_string($hash) ? tpl_text("Alias of field <b>'%s'</b>", $hash) : tpl_def(Module::runHook('field_extension_description', $field)) ?></td>
		</tr>
	<? } ?>
</table>

<h3><?= tpl_text('Some examples') ?></h3>
<code>
	title:"users guide"<br />
	creator:"Mikael Johansson"<br />
	created:2004-01-01<br />
	subject:(searchengine OR indexing)
</code>
