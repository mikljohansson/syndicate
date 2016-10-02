<? if (!empty($matches)) { ?>
	<div class="Result">
		<? if (null != $query) { ?>
		<?= tpl_translate("Results %d-%d of %d matching <b>'%s'</b> (<a href=\"%s\">reset search</a>)", 
			tpl_quote($offset+1), tpl_quote($offset+count($matches)), 
			tpl_quote($count), tpl_quote($query),
			tpl_link('issue','project',$node->getProjectId(),'admin','crypto')); ?>
		<? } else { ?>
		<?= tpl_text("Results %d-%d of %d keys in keyring", 
			$offset+1, $offset+count($matches), $count); ?>
		<? } ?>
		<? $this->display(tpl_design_path('gui/pager.tpl'),
			array('offset'=>$offset,'limit'=>$limit,'count'=>$count)) ?>
	</div>
	<table>
		<thead>
			<tr>
				<th><?= tpl_text('Account') ?></th>
				<th><?= tpl_text('Key ID') ?></th>
				<th><?= tpl_text('Type') ?></th>
				<th><?= tpl_text('Expiry') ?></th>
				<th><?= tpl_text('Trusted') ?></th>
				<th>&nbsp;</th>
			</tr>
		</thead>
		<tbody>
			<? foreach ($matches as $key) { ?>
			<tr>
				<td><?= synd_htmlspecialchars($key->getIdentity()->toString()) ?></td>
				<td><a href="<?= tpl_link('issue','project',$node->getProjectId(),'admin','crypto',$key->getKeyid()) ?>"><?= $key->getKeyid() ?></a></td>
				<td><?
					if ($key->isEncryptionKey() && $key->isDecryptionKey())
						print tpl_text('pub/sec');
					else if ($key->isEncryptionKey())
						print tpl_text('pub');
					else if ($key->isDecryptionKey())
						print tpl_text('sec');
					?></td>
				<td><?= tpl_strftime('%Y-%d-%m', $key->getExpiry()) ?></td>
				<td><?= $key->getTrust() ? tpl_text('Yes') : tpl_text('No') ?></td>
				<td><a href="<?= tpl_link_call('issue','crypto','delete',$node->nodeId,$key->getKeyid()) 
					?>"><img src="<?= tpl_design_uri('image/icon/trash.gif') ?>" width="16" height="16" alt="<?= tpl_text('Delete') ?>" /></a></td>
			</tr>
			<? } ?>
		</tbody>
	</table>
<? } else if (null != $query) { ?>
	<div class="Result">
		<?= tpl_translate("No keys matching <em>'%s'</em> were found (<a href=\"%s\">reset search</a>)", 
			tpl_quote($query), 
			tpl_link('issue','project',$node->getProjectId(),'admin','crypto')) ?>
	</div>
<? } else { ?>
	<div class="Result">
		<?= tpl_text("No keys were found") ?>
	</div>
<? } ?>

<form action="<?= tpl_link('issue','project',$node->getProjectId(),'admin','crypto') ?>" method="get">
	<p>
		<input type="text" name="q" value="<?= tpl_attribute($query) ?>" size="30" />
		<input type="submit" value="<?= tpl_text('Search') ?>" />
	</p>
</form>

<h3><?= tpl_text('Upload key') ?></h3>
<? include tpl_design_path('gui/errors.tpl'); ?>
<form action="<?= tpl_link('issue','project',$node->getProjectId(),'admin','crypto') ?>" method="post" enctype="multipart/form-data">
	<input type="hidden" name="MAX_FILE_SIZE" value="2000000" />
	<input type="file" name="file" size="60" />
	<input type="submit" value="<?= tpl_text('Upload') ?>" />
</form>
