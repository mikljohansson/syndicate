<? 
$path = 'access/savePermissions/'.$subject->id();
?>
<form method="post">
	<? foreach ($subject->getDefinedPermissions() as $perm => $description) { ?>
	<fieldset>
		<legend><?= tpl_text(ucfirst($perm)." permission") ?></legend>
		<div class="Info"><?= $description ?></div>
		<? foreach (array_keys($roles = $subject->getDefinedRoles()) as $key) { ?>
			<? $id = "mplex[$path][$perm][".$roles[$key]->id()."]"; ?>
			<? if ($module->isInherited($subject,$perm) && $module->checkPermission($subject->getParent(),$roles[$key],$perm)) { ?>
				<input id="<?= $id ?>" type="checkbox" checked="checked" disabled="disabled" />
			<? } else { ?>
				<?= tpl_form_checkbox("mplex[$path][perms][$perm][]",
					$module->checkPermission($subject,$roles[$key],$perm),
					$roles[$key]->id(), $id) ?>
			<? } ?>
			<label for="<?= $id ?>"><?= $roles[$key]->isNull() ? tpl_text('Anonymous') : $roles[$key]->toString() ?></label>
		<? } ?>

		<? if (null != ($parent = $subject->getParent()) && !$parent->isNull()) { ?>
		<? $id = "mplex[$path][inherit][$perm]"; ?>
		<?= tpl_form_checkbox("mplex[$path][inherit][]",$module->isInherited($subject,$perm),$perm,$id) ?>
			<label for="<?= $id ?>"><?= tpl_text('Inherit permissions') ?></label>
		<? } ?>
		
		<? if (count($roles = SyndLib::array_kdiff($module->getPermissionAssignment($subject, $perm), $subject->getDefinedRoles()))) { ?>
			<? $roles = SyndLib::sort($roles); ?>
			<table>
				<? foreach (array_keys($roles) as $key) { ?>
				<tr>
					<td class="nowrap" style="padding-right:1em;">
						<? $this->render($roles[$key],'contact.tpl') ?>
					</td>
					<td>
						<? if (!$module->isInheritedAssignment($subject, $perm, $roles[$key])) { ?>
						<input type="hidden" name="mplex[<?= $path ?>][perms][<?= $perm ?>][]" value="<?= $roles[$key]->id() ?>" />
						<a href="<?= tpl_link_call('access','removePermission',$subject->id(),array('role'=>$roles[$key]->id(),'perm'=>$perm)) 
							?>"><img src="<?= tpl_design_uri('image/icon/trash.gif') ?>" /></a>
						<? } else print '&nbsp;' ?>
					</td>
				</tr>
				<? } ?>
			</table>
		<? } ?>
	</fieldset>
	<? } ?>

	<h3><?= tpl_text('Assign permission') ?></h3>
	<div class="indent">
		<? 
		$options = array();
		foreach (array_keys($subject->getDefinedPermissions()) as $perm)
			$options[$perm] = tpl_text(ucfirst($perm)." permission");
		?>
		<select name="mplex[<?= $path ?>][assign][perm]">
			<?= tpl_form_options($options,$_REQUEST['mplex'][$path]['assign']['perm']) ?>
		</select>
		<input type="text" name="assign[query]" value="<?= tpl_attribute($_REQUEST['assign']['query']) ?>" />
		<input type="submit" value="<?= tpl_text('Search') ?>" />

		<div style="margin-top:0.5em; margin-bottom:1em;">
		<? if (!empty($_REQUEST['assign']['query']) && empty($_REQUEST['mplex'][$path]['post'])) { ?>
			<? 
			$collection = $module->findRoles($_REQUEST['assign']['query']); 
			if (count($roles = $collection->getContents(0,20))) { ?>
				<? foreach ($roles as $role) break; { ?>
					<?= tpl_form_checkbox("mplex[$path][assign][roles][]",isset($_REQUEST['mplex'][$path]['assign']['roles']) && 
						in_array($role->id(),$_REQUEST['mplex'][$path]['assign']['roles']),$role->id()) ?>
					<? $this->render($role,'contact.tpl') ?>
					<br />
				<? } ?>
			<? } else { ?>
				<em><?= tpl_text("No results matching <b>'%s'</b> were found", 
					$_REQUEST['assign']['query']) ?></em>
			<? } ?>
		<? } ?>
		</div>
	</div>

	<input type="submit" name="mplex[<?= $path ?>][post]" value="<?= tpl_text('Save') ?>" />
</form>
