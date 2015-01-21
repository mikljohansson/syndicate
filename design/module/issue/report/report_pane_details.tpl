<?
$assigned = SyndLib::invoke($handlers,'toString');
uasort($assigned, 'strcasecmp'); 

?>
<table>
	<tr>
		<th rowspan="5"><?= tpl_text('Report') ?></th>
		<td rowspan="5">
			<dl>
				<dt>
					<?= tpl_form_radiobutton('report',$request['report'],'list') ?>
					<label for="report[list]"><?= tpl_text('Listing of issues') ?></label>
				</dt>
				<dd><?= tpl_text('Displays issues with title, duration and other info. The Excel version is geared to be used as an invoice') ?></dd>
				<dt>
					<?= tpl_form_radiobutton('report',$request['report'],'workload') ?>
					<label for="report[workload]"><?= tpl_text('Scheduled workload') ?></label>
				</dt>
				<dd><?= tpl_text('Displays issues with summaries per week and month for easy workload scheduling') ?></dd>
				<dt>
					<?= tpl_form_radiobutton('report',$request['report'],'trends') ?>
					<label for="report[trends]"><?= tpl_text('Statistics trends') ?></label>
				</dt>
				<dd><?= tpl_text('Report showing service level performance figures summarized per week/month/year') ?></dd>
				<dt>
					<?= tpl_form_radiobutton('report',$request['report'],'projects') ?>
					<label for="report[projects]"><?= tpl_text('Statistics per project') ?></label>
				</dt>
				<dd><?= tpl_text('Report showing service level performance figures grouped by project') ?></dd>
				<dt>
					<?= tpl_form_radiobutton('report',$request['report'],'categories') ?>
					<label for="report[categories]"><?= tpl_text('Statistics per category') ?></label>
				</dt>
				<dd><?= tpl_text('Report showing service level performance figures grouped by category') ?></dd>
				<dt>
					<?= tpl_form_radiobutton('report',$request['report'],'assigned') ?>
					<label for="report[assigned]"><?= tpl_text('Statistics per assigned') ?></label>
				</dt>
				<dd><?= tpl_text('Report showing service level performance figures grouped by assigned') ?></dd>
				<dt>
					<?= tpl_form_radiobutton('report',$request['report'],'customers') ?>
					<label for="report[customers]"><?= tpl_text('Statistics per customer') ?></label>
				</dt>
				<dd><?= tpl_text('Report showing service level performance figures grouped by customer') ?></dd>
				<dt>
					<?= tpl_form_radiobutton('report',$request['report'],'departments') ?>
					<label for="report[departments]"><?= tpl_text('Statistics per customer department') ?></label>
				</dt>
				<dd><?= tpl_text('Report showing service level performance figures grouped by department') ?></dd>
			</dl>
		</td>
		<th><?= tpl_text('Options') ?></th>
		<td style="height:1%;">
			<?= tpl_form_checkbox('subissues',$request['subissues']) ?>
			<label for="subissues"><?= tpl_text('Include matching subissues') ?></label><br />
			<?= tpl_form_checkbox('recurse',$request['recurse']) ?>
			<label for="recurse"><?= tpl_text('Include all child projects') ?></label><br />
			<?= tpl_form_checkbox('exclude_keywords',$request['exclude_keywords']) ?>
			<label for="exclude_keywords"><?= tpl_text('Exclude selected categories') ?></label>
		</td>
	</tr>
	<tr>
		<th style="height:1%;"><?= tpl_text('Period') ?></th>
		<td>
			<select name="format">
				<?= tpl_form_options(array(
					'year'	=> tpl_text('Year'),
					''		=> tpl_text('Month'),
					'week'	=> tpl_text('Week'),
					'day'	=> tpl_text('Day')),
					(string)$request['format']) ?>
			</select>
		</td>
	</tr>
	<tr>
		<th style="height:1%;"><?= tpl_text('Created by') ?></th>
		<td><input type="text" name="creator" id="creator" value="<?= $this->quote($request['creator']) ?>" /></td>
	</tr>
	<tr>
		<th style="height:1%;"><?= tpl_text('Changed by') ?></th>
		<td><input type="text" name="updater" id="updater" value="<?= $this->quote($request['updater']) ?>" /></td>
	</tr>
	<tr>
		<th><?= tpl_text('Comment by') ?></th>
		<td><input type="text" name="commenter" id="commenter" value="<?= $this->quote($request['commenter']) ?>" /></td>
	</tr>
</table>