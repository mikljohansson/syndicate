			<tr>
				<th><?= tpl_text('Link SVN revision') ?></th>
				<td<? if(isset($errors['svn_revision'])) print ' class="InvalidField"'; ?>>
					<input type="text" tabindex="<?= $tabindex++ ?>" name="data[svn][revision]" value="<?= tpl_value($revision) ?>" style="width:200px;" />
				</td>
			</tr>
